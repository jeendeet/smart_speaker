/*! \file main.cpp
 *  \brief Send/Receive data through sound
 *  \author Georgi Gerganov
 */

#include "reed-solomon/rs.hpp"

#include <cmath>
#include <cstdio>
#include <array>
#include <string>
#include <chrono>
#include <ctime>
#include <algorithm>
#include <map>
#include <complex>
#include "encode_wav.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

#ifdef __EMSCRIPTEN__
#include "build_timestamp.h"
#include "emscripten/emscripten.h"
#else
#include <iostream>
#endif

#ifdef main
#undef main
#endif

static char *g_captureDeviceName = nullptr;
static int g_captureId = -1;
static int g_playbackId = -1;

static bool g_isInitialized = false;
static int g_totalBytesCaptured = 0;

struct DataRxTx;
static DataRxTx *g_data = nullptr;
const char* wav_file;
namespace {

constexpr double kBaseSampleRate = 44100.0;
constexpr auto kMaxSamplesPerFrame = 1024;
constexpr auto kMaxDataBits = 256;
constexpr auto kMaxDataSize = 256;
constexpr auto kMaxLength = 140;
constexpr auto kMaxSpectrumHistory = 4;
constexpr auto kMaxRecordedFrames = 64*10;
constexpr auto kDefaultFixedLength = 82;

enum TxMode {
    FixedLength = 0,
    VariableLength,
};

using AmplitudeData   = std::array<float, kMaxSamplesPerFrame>;
using AmplitudeData16 = std::array<int16_t, kMaxRecordedFrames*kMaxSamplesPerFrame>;
using SpectrumData    = std::array<float, kMaxSamplesPerFrame>;
using RecordedData    = std::array<float, kMaxRecordedFrames*kMaxSamplesPerFrame>;

inline void addAmplitudeSmooth(const AmplitudeData & src, AmplitudeData & dst, float scalar, int startId, int finalId, int cycleMod, int nPerCycle) {
    int nTotal = nPerCycle*finalId;
    float frac = 0.15f;
    float ds = frac*nTotal;
    float ids = 1.0f/ds;
    int nBegin = frac*nTotal;
    int nEnd = (1.0f - frac)*nTotal;
    for (int i = startId; i < finalId; i++) {
        float k = cycleMod*finalId + i;
        if (k < nBegin) {
            dst[i] += scalar*src[i]*(k*ids);
        } else if (k > nEnd) {
            dst[i] += scalar*src[i]*(((float)(nTotal) - k)*ids);
        } else {
            dst[i] += scalar*src[i];
        }
    }
}

int getECCBytesForLength(int len) {
    return std::max(4, 2*(len/5));
}
}

void get_wav_header(int raw_sz, wav_header_t *wh)
{
	// RIFF chunk
	strcpy((char*)wh->chunk_id, "RIFF");
	wh->chunk_size = 36 + raw_sz;
	
	// fmt sub-chunk (to be optimized)
	strncpy((char*)wh->sub_chunk1_id, "WAVEfmt ", strlen("WAVEfmt "));
	wh->sub_chunk1_size = 16;
	wh->audio_format = 1;
	wh->num_channels = 1;
	wh->sample_rate = kBaseSampleRate;
	wh->bits_per_sample = 16;
	wh->block_align = wh->num_channels * wh->bits_per_sample / 8;
	wh->byte_rate = wh->sample_rate * wh->num_channels * wh->bits_per_sample / 8;
	
	// data sub-chunk
	strncpy((char*)wh->sub_chunk2_id, "data", strlen("data"));
	wh->sub_chunk2_size = raw_sz;
}

void dump_wav_header (wav_header_t *wh)
{
	printf ("=========================================\n");
	printf ("chunk_id:\t\t\t%s\n", wh->chunk_id);
	printf ("chunk_size:\t\t\t%d\n", wh->chunk_size);
	printf ("sub_chunk1_id:\t\t\t%s\n", wh->sub_chunk1_id);
	printf ("sub_chunk1_size:\t\t%d\n", wh->sub_chunk1_size);
	printf ("audio_format:\t\t\t%d\n", wh->audio_format);
	printf ("num_channels:\t\t\t%d\n", wh->num_channels);
	printf ("sample_rate:\t\t\t%d\n", wh->sample_rate);
	printf ("bits_per_sample:\t\t%d\n", wh->bits_per_sample);
	printf ("block_align:\t\t\t%d\n", wh->block_align);
	printf ("byte_rate:\t\t\t%d\n", wh->byte_rate);
	printf ("sub_chunk2_id:\t\t\t%s\n", wh->sub_chunk2_id);
	printf ("sub_chunk2_size:\t\t%d\n", wh->sub_chunk2_size);
	printf ("=========================================\n");
}

struct DataRxTx {
    DataRxTx(int aSampleRateOut, int aSampleRate, int aSamplesPerFrame, int aSampleSizeB, const char * text) {
        sampleSizeBytes = aSampleSizeB;
        sampleRate = aSampleRate;
        sampleRateOut = aSampleRateOut;
        samplesPerFrame = aSamplesPerFrame;

        init(strlen(text), text);
    }

    void init(int textLength, const char * stext) {
        if (textLength > ::kMaxLength) {
            printf("Truncating data from %d to 140 bytes\n", textLength);
            textLength = ::kMaxLength;
        }

        const uint8_t * text = reinterpret_cast<const uint8_t *>(stext);
        frameId = 0;
        nIterations = 0;
        hasData = false;

        isamplesPerFrame = 1.0f/samplesPerFrame;
        sendVolume = ((double)(paramVolume))/100.0f;
        hzPerFrame = sampleRate/samplesPerFrame;
        ihzPerFrame = 1.0/hzPerFrame;
        framesPerTx = paramFramesPerTx;

        nDataBitsPerTx = paramBytesPerTx*8;
        nECCBytesPerTx = (txMode == ::TxMode::FixedLength) ? paramECCBytesPerTx : getECCBytesForLength(textLength);

        framesToAnalyze = 0;
        framesLeftToAnalyze = 0;
        framesToRecord = 0;
        framesLeftToRecord = 0;
        nBitsInMarker = 16;
        nMarkerFrames = 16;
        nPostMarkerFrames = 0;
        sendDataLength = (txMode == ::TxMode::FixedLength) ? ::kDefaultFixedLength : textLength + 3;

        d0 = paramFreqDelta/2;
        freqDelta_hz = hzPerFrame*paramFreqDelta;
        freqStart_hz = hzPerFrame*paramFreqStart;
        if (paramFreqDelta == 1) {
            d0 = 1;
            freqDelta_hz *= 2;
        }

        outputBlock.fill(0);
        encodedData.fill(0);

        for (int k = 0; k < (int) phaseOffsets.size(); ++k) {
            phaseOffsets[k] = (M_PI*k)/(nDataBitsPerTx);
        }
#ifdef __EMSCRIPTEN__
        std::random_shuffle(phaseOffsets.begin(), phaseOffsets.end());
#endif

        for (int k = 0; k < (int) dataBits.size(); ++k) {
            double freq = freqStart_hz + freqDelta_hz*k;
            dataFreqs_hz[k] = freq;

            double phaseOffset = phaseOffsets[k];
            double curHzPerFrame = sampleRateOut/samplesPerFrame;
            double curIHzPerFrame = 1.0/curHzPerFrame;
            for (int i = 0; i < samplesPerFrame; i++) {
                double curi = i;
                bit1Amplitude[k][i] = std::sin((2.0*M_PI)*(curi*isamplesPerFrame)*(freq*curIHzPerFrame) + phaseOffset);
            }
            for (int i = 0; i < samplesPerFrame; i++) {
                double curi = i;
                bit0Amplitude[k][i] = std::sin((2.0*M_PI)*(curi*isamplesPerFrame)*((freq + hzPerFrame*d0)*curIHzPerFrame) + phaseOffset);
            }
        }

        if (rsData) delete rsData;
        if (rsLength) delete rsLength;

        if (txMode == ::TxMode::FixedLength) {
            rsData = new RS::ReedSolomon(kDefaultFixedLength, nECCBytesPerTx);
        } else {
            rsData = new RS::ReedSolomon(textLength, nECCBytesPerTx);
            rsLength = new RS::ReedSolomon(1, 2);
        }

        if (textLength > 0) {
            static std::array<char, ::kMaxDataSize> theData;
            theData.fill(0);

            if (txMode == ::TxMode::FixedLength) {
                for (int i = 0; i < textLength; ++i) theData[i] = text[i];
                rsData->Encode(theData.data(), encodedData.data());
            } else {
                theData[0] = textLength;
                for (int i = 0; i < textLength; ++i) theData[i + 1] = text[i];
                rsData->Encode(theData.data() + 1, encodedData.data() + 3);
                rsLength->Encode(theData.data(), encodedData.data());
            }

            hasData = true;
        }

        // Rx
        receivingData = false;
        analyzingData = false;

        sampleAmplitude.fill(0);

        sampleSpectrum.fill(0);
        for (auto & s : sampleAmplitudeHistory) {
            s.fill(0);
        }

        rxData.fill(0);

        for (int i = 0; i < samplesPerFrame; ++i) {
            fftOut[i].real(0.0f);
            fftOut[i].imag(0.0f);
        }
    }

    void send() {
        int samplesPerFrameOut = (sampleRateOut/sampleRate)*samplesPerFrame;
        if (sampleRateOut != sampleRate) {
            printf("Resampling from %d Hz to %d Hz\n", (int) sampleRate, (int) sampleRateOut);
        }

        while(hasData) {
            int nBytesPerTx = nDataBitsPerTx/8;
            std::fill(outputBlock.begin(), outputBlock.end(), 0.0f);
            std::uint16_t nFreq = 0;

            if (sampleRateOut != sampleRate) {
                for (int k = 0; k < nDataBitsPerTx; ++k) {
                    double freq = freqStart_hz + freqDelta_hz*k;

                    double phaseOffset = phaseOffsets[k];
                    double curHzPerFrame = sampleRateOut/samplesPerFrame;
                    double curIHzPerFrame = 1.0/curHzPerFrame;
                    for (int i = 0; i < samplesPerFrameOut; i++) {
                        double curi = (i + frameId*samplesPerFrameOut);
                        bit1Amplitude[k][i] = std::sin((2.0*M_PI)*(curi*isamplesPerFrame)*(freq*curIHzPerFrame) + phaseOffset);
                    }
                    for (int i = 0; i < samplesPerFrameOut; i++) {
                        double curi = (i + frameId*samplesPerFrameOut);
                        bit0Amplitude[k][i] = std::sin((2.0*M_PI)*(curi*isamplesPerFrame)*((freq + hzPerFrame*d0)*curIHzPerFrame) + phaseOffset);
                    }
                }
            }

            if (frameId < nMarkerFrames) {
                nFreq = nBitsInMarker;

                for (int i = 0; i < nBitsInMarker; ++i) {
                    if (i%2 == 0) {
                        ::addAmplitudeSmooth(bit1Amplitude[i], outputBlock, sendVolume, 0, samplesPerFrameOut, frameId, nMarkerFrames);
                    } else {
                        ::addAmplitudeSmooth(bit0Amplitude[i], outputBlock, sendVolume, 0, samplesPerFrameOut, frameId, nMarkerFrames);
                    }
                }
            } else if (frameId < nMarkerFrames + nPostMarkerFrames) {
                nFreq = nBitsInMarker;

                for (int i = 0; i < nBitsInMarker; ++i) {
                    if (i%2 == 0) {
                        ::addAmplitudeSmooth(bit0Amplitude[i], outputBlock, sendVolume, 0, samplesPerFrameOut, frameId - nMarkerFrames, nPostMarkerFrames);
                    } else {
                        ::addAmplitudeSmooth(bit1Amplitude[i], outputBlock, sendVolume, 0, samplesPerFrameOut, frameId - nMarkerFrames, nPostMarkerFrames);
                    }
                }
            } else if (frameId <
                       (nMarkerFrames + nPostMarkerFrames) +
                       ((sendDataLength + nECCBytesPerTx)/nBytesPerTx + 2)*framesPerTx) {
                int dataOffset = frameId - nMarkerFrames - nPostMarkerFrames;
                int cycleModMain = dataOffset%framesPerTx;
                dataOffset /= framesPerTx;
                dataOffset *= nBytesPerTx;

                dataBits.fill(0);

                if (paramFreqDelta > 1) {
                    for (int j = 0; j < nBytesPerTx; ++j) {
                        for (int i = 0; i < 8; ++i) {
                            dataBits[j*8 + i] = encodedData[dataOffset + j] & (1 << i);
                        }
                    }

                    for (int k = 0; k < nDataBitsPerTx; ++k) {
                        ++nFreq;
                        if (dataBits[k] == false) {
                            ::addAmplitudeSmooth(bit0Amplitude[k], outputBlock, sendVolume, 0, samplesPerFrameOut, cycleModMain, framesPerTx);
                            continue;
                        }
                        ::addAmplitudeSmooth(bit1Amplitude[k], outputBlock, sendVolume, 0, samplesPerFrameOut, cycleModMain, framesPerTx);
                    }
                } else {
                    for (int j = 0; j < nBytesPerTx; ++j) {
                        {
                            uint8_t d = encodedData[dataOffset + j] & 15;
                            dataBits[(2*j + 0)*16 + d] = 1;
                        }
                        {
                            uint8_t d = encodedData[dataOffset + j] & 240;
                            dataBits[(2*j + 1)*16 + (d >> 4)] = 1;
                        }
                    }

                    for (int k = 0; k < 2*nBytesPerTx*16; ++k) {
                        if (dataBits[k] == 0) continue;

                        ++nFreq;
                        if (k%2) {
                            ::addAmplitudeSmooth(bit0Amplitude[k/2], outputBlock, sendVolume, 0, samplesPerFrameOut, cycleModMain, framesPerTx);
                        } else {
                            ::addAmplitudeSmooth(bit1Amplitude[k/2], outputBlock, sendVolume, 0, samplesPerFrameOut, cycleModMain, framesPerTx);
                        }
                    }
                }
            } else if (txMode == ::TxMode::VariableLength && frameId <
                       (nMarkerFrames + nPostMarkerFrames) +
                       ((sendDataLength + nECCBytesPerTx)/nBytesPerTx + 2)*framesPerTx +
                       (nMarkerFrames)) {
                nFreq = nBitsInMarker;

                int fId = frameId - ((nMarkerFrames + nPostMarkerFrames) + ((sendDataLength + nECCBytesPerTx)/nBytesPerTx + 2)*framesPerTx);
                for (int i = 0; i < nBitsInMarker; ++i) {
                    if (i%2 == 0) {
                        ::addAmplitudeSmooth(bit0Amplitude[i], outputBlock, sendVolume, 0, samplesPerFrameOut, fId, nMarkerFrames);
                    } else {
                        ::addAmplitudeSmooth(bit1Amplitude[i], outputBlock, sendVolume, 0, samplesPerFrameOut, fId, nMarkerFrames);
                    }
                }
            } else {
                textToSend = "";
                hasData = false;
            }

            if (nFreq == 0) nFreq = 1;
            float scale = 1.0f/nFreq;
            for (int i = 0; i < samplesPerFrameOut; ++i) {
                outputBlock[i] *= scale;
            }

            for (int i = 0; i < samplesPerFrameOut; ++i) {
                outputBlock16[frameId*samplesPerFrameOut + i] = std::round(32000.0*outputBlock[i]);
            }
            ++frameId;
        }

        FILE *fwav;
        wav_header_t wheader;
        // construct wav header
        get_wav_header (outputBlock16.size()/2, &wheader);
        dump_wav_header (&wheader);

        // write out the .wav file
        fwav = fopen(wav_file, "wb");
        fwrite(&wheader, 1, sizeof(wheader), fwav);
        fwrite(outputBlock16.data(), 1, outputBlock16.size()/2, fwav);
        fclose(fwav);
    }

    int nIterations;
    bool needUpdate = false;

    int paramFreqDelta = 6;
    int paramFreqStart = 40;
    int paramFramesPerTx = 6;
    int paramBytesPerTx = 2;
    int paramECCBytesPerTx = 32;
    int paramVolume = 10;

    // Rx
    bool receivingData;
    bool analyzingData;

    std::array<float, kMaxSamplesPerFrame> fftIn;
    std::array<std::complex<float>, kMaxSamplesPerFrame> fftOut;

    ::AmplitudeData sampleAmplitude;
    ::SpectrumData sampleSpectrum;

    std::array<std::uint8_t, ::kMaxDataSize> rxData;
    std::array<std::uint8_t, ::kMaxDataSize> encodedData;

    int historyId = 0;
    ::AmplitudeData sampleAmplitudeAverage;
    std::array<::AmplitudeData, ::kMaxSpectrumHistory> sampleAmplitudeHistory;

    ::RecordedData recordedAmplitude;

    // Tx
    bool hasData;
    int sampleSizeBytes;
    float sampleRate;
    float sampleRateOut;
    int samplesPerFrame;
    float isamplesPerFrame;

    ::AmplitudeData outputBlock;
    ::AmplitudeData16 outputBlock16;

    std::array<::AmplitudeData, ::kMaxDataBits> bit1Amplitude;
    std::array<::AmplitudeData, ::kMaxDataBits> bit0Amplitude;

    float sendVolume;
    float hzPerFrame;
    float ihzPerFrame;

    int d0 = 1;
    float freqStart_hz;
    float freqDelta_hz;

    int frameId;
    int nRampFrames;
    int nRampFramesBegin;
    int nRampFramesEnd;
    int nRampFramesBlend;
    int dataId;
    int framesPerTx;
    int framesToAnalyze;
    int framesLeftToAnalyze;
    int framesToRecord;
    int framesLeftToRecord;
    int nBitsInMarker;
    int nMarkerFrames;
    int nPostMarkerFrames;
    int recvDuration_frames;

    ::TxMode txMode = ::TxMode::FixedLength;

    std::array<bool, ::kMaxDataBits> dataBits;
    std::array<double, ::kMaxDataBits> phaseOffsets;
    std::array<double, ::kMaxDataBits> dataFreqs_hz;

    int nDataBitsPerTx;
    int nECCBytesPerTx;
    int sendDataLength;

    RS::ReedSolomon * rsData = nullptr;
    RS::ReedSolomon * rsLength = nullptr;

    float averageRxTime_ms = 0.0;

    std::string textToSend;
};

int init() {
    if (g_isInitialized) return 0;

    printf("Initializing ...\n");

    SDL_AudioSpec desiredSpec;
    desiredSpec.freq = ::kBaseSampleRate;
    desiredSpec.format = AUDIO_S16LSB;
    desiredSpec.channels = 1;
    desiredSpec.samples = 16*1024;

    SDL_AudioSpec obtainedSpec;
    obtainedSpec.format == desiredSpec.format;
    obtainedSpec.channels == desiredSpec.channels;
    obtainedSpec.samples == desiredSpec.samples;
    obtainedSpec.freq = ::kBaseSampleRate;

    SDL_AudioSpec captureSpec;
    captureSpec = obtainedSpec;
    captureSpec.freq = ::kBaseSampleRate;
    captureSpec.format = AUDIO_F32LSB;
    captureSpec.samples = 1024;

    int sampleSizeBytes = 4;

    g_data = new DataRxTx(obtainedSpec.freq, ::kBaseSampleRate, captureSpec.samples, sampleSizeBytes, "");

    g_isInitialized = true;
    return 0;
}

extern "C" {
    int setText(int textLength, const char * text) {
        g_data->init(textLength, text);
        return 0;
    }

    int setTxMode(int txMode) { g_data->txMode = (::TxMode)(txMode); return 0; }

    void setParameters(
        int paramFreqDelta,
        int paramFreqStart,
        int paramFramesPerTx,
        int paramBytesPerTx,
        int /*paramECCBytesPerTx*/,
        int paramVolume) {
        if (g_data == nullptr) return;

        g_data->paramFreqDelta = paramFreqDelta;
        g_data->paramFreqStart = paramFreqStart;
        g_data->paramFramesPerTx = paramFramesPerTx;
        g_data->paramBytesPerTx = paramBytesPerTx;
        g_data->paramVolume = paramVolume;

        g_data->needUpdate = true;
    }
}

static std::map<std::string, std::string> parseCmdArguments(int argc, char ** argv) {
    int last = argc;
    std::map<std::string, std::string> res;
    for (int i = 1; i < last; ++i) {
        if (argv[i][0] == '-') {
            if (strlen(argv[i]) > 1) {
                res[std::string(1, argv[i][1])] = strlen(argv[i]) > 2 ? argv[i] + 2 : "";
            }
        }
    }

    return res;
}

int main(int argc, char** argv) {
#ifdef __EMSCRIPTEN__
    printf("Build time: %s\n", BUILD_TIMESTAMP);
    printf("Press the Init button to start\n");

    g_captureDeviceName = argv[1];
#else
    printf("Usage: %s [-cN] [-pN] [-tN]\n", argv[0]);
    printf("    -sString - input string to convert String\n");
    printf("    -fF      - input path and name wav file F\n");
    printf("    -cN      - select capture device N\n");
    printf("    -pN      - select playback device N\n");
    printf("    -tN      - transmission protocol:\n");
    printf("          -t0 : Normal\n");
    printf("          -t1 : Fast (default)\n");
    printf("          -t2 : Fastest\n");
    printf("          -t3 : Ultrasonic\n");
    printf("\n");

    g_captureDeviceName = nullptr;

    auto argm = parseCmdArguments(argc, argv);
    g_captureId = argm["c"].empty() ? 0 : std::stoi(argm["c"]);
    g_playbackId = argm["p"].empty() ? 0 : std::stoi(argm["p"]);
    int txProtocol = argm["t"].empty() ? 1 : std::stoi(argm["t"]);
#endif

#ifdef __EMSCRIPTEN__
    emscripten_set_main_loop(update, 60, 1);
#else
    init();
    setTxMode(1);
    printf("Selecting Tx protocol %d\n", txProtocol);
    switch (txProtocol) {
        case 0:
            {
                printf("Using 'Normal' Tx Protocol\n");
                setParameters(1, 40, 9, 3, 0, 50);
            }
            break;
        case 1:
            {
                printf("Using 'Fast' Tx Protocol\n");
                setParameters(1, 40, 6, 3, 0, 50);
            }
            break;
        case 2:
            {
                printf("Using 'Fastest' Tx Protocol\n");
                setParameters(1, 40, 3, 3, 0, 50);
            }
            break;
        case 3:
            {
                printf("Using 'Ultrasonic' Tx Protocol\n");
                setParameters(1, 320, 9, 3, 0, 50);
            }
            break;
        default:
            {
                printf("Using 'Fast' Tx Protocol\n");
                setParameters(1, 40, 6, 3, 0, 50);
            }
    };
    printf("\n");
    if (!argm["s"].empty() && !argm["f"].empty()){

        std::string wav_file_str = argm["f"];
        wav_file = &wav_file_str[0];

        std::string input = argm["s"];
        std::string inputOld = "";
        if (input.empty()) {
            std::cout << "Re-sending ... " << std::endl;
            input = inputOld;
        } else {
            std::cout << "Sending ... " << std::endl;
        }
        setText(input.size(), input.data());
        inputOld = input;
        if (g_data->hasData == true) {
            g_data->send();
        }
    }
    else
    {
        printf("Please input string to convert sound and wav file name\n");
    }
#endif

    delete g_data;
    return 0;
}
