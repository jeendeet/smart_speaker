/*
 * Copyright 2017-2018 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     http://aws.amazon.com/apache2.0/
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

#include <memory>
#include <iostream>

#include <AVSCommon/Utils/Logger/Logger.h>

#include "VinBigData/VinBigDataKeywordDetector.h"
#include "common/log.h"
#include <chrono>
#include <inttypes.h>

namespace alexaClientSDK {
namespace kwd {

using namespace avsCommon::utils::logger;
using namespace std::chrono;

/// String to identify log entries originating from this file.
static const std::string TAG("VinBigDataKeywordDetector");

/**
 * Create a LogEntry using this file's TAG and the specified event string.
 *
 * @param The event string for this @c LogEntry.
 */
#define LX(event) alexaClientSDK::avsCommon::utils::logger::LogEntry(TAG, event)

/// The number of hertz per kilohertz.
static const size_t HERTZ_PER_KILOHERTZ = 1000;

/// The timeout to use for read calls to the SharedDataStream.
const std::chrono::milliseconds TIMEOUT_FOR_READ_CALLS = std::chrono::milliseconds(1000);

/// The Sensory compatible AVS sample rate of 16 kHz.
static const unsigned int SENSORY_COMPATIBLE_SAMPLE_RATE = 16000;

static const int BUFFER_SIZE = 3200;

/// The Sensory compatible bits per sample of 16.
static const unsigned int SENSORY_COMPATIBLE_SAMPLE_SIZE_IN_BITS = 16;

/// The Sensory compatible number of channels, which is 1.
static const unsigned int SENSORY_COMPATIBLE_NUM_CHANNELS = 1;

/// The Sensory compatible audio encoding of LPCM.
static const avsCommon::utils::AudioFormat::Encoding SENSORY_COMPATIBLE_ENCODING =
    avsCommon::utils::AudioFormat::Encoding::LPCM;

/// The Sensory compatible endianness which is little endian.
static const avsCommon::utils::AudioFormat::Endianness SENSORY_COMPATIBLE_ENDIANNESS =
    avsCommon::utils::AudioFormat::Endianness::LITTLE;

/**
 * Checks to see if an @c avsCommon::utils::AudioFormat is compatible with Sensory.
 *
 * @param audioFormat The audio format to check.
 * @return @c true if the audio format is compatible with Sensory and @c false otherwise.
 */
 static void isAudioFormatCompatible(avsCommon::utils::AudioFormat audioFormat) {
     std::cout << "Audio format information" << std::endl \
               << "- audioFormat.encoding : " << audioFormat.encoding << std::endl \
               << "- audioFormat.endianness : " << audioFormat.endianness << std::endl \
               << "- audioFormat.sampleRateHz : " << audioFormat.sampleRateHz << std::endl \
               << "- audioFormat.sampleSizeInBits : " << audioFormat.sampleSizeInBits << std::endl \
               << "- audioFormat.numChannels : " << audioFormat.numChannels << std::endl;
//     if (SENSORY_COMPATIBLE_ENCODING != audioFormat.encoding) {
//         ACSDK_ERROR(LX("isAudioFormatCompatibleFailed")
//                         .d("reason", "incompatibleEncoding")
//                         .d("sensoryEncoding", SENSORY_COMPATIBLE_ENCODING)
//                         .d("encoding", audioFormat.encoding));
//         return false;
//     }
//     if (SENSORY_COMPATIBLE_ENDIANNESS != audioFormat.endianness) {
//         ACSDK_ERROR(LX("isAudioFormatCompatibleFailed")
//                         .d("reason", "incompatibleEndianess")
//                         .d("sensoryEndianness", SENSORY_COMPATIBLE_ENDIANNESS)
//                         .d("endianness", audioFormat.endianness));
//         return false;
//     }
//     if (SENSORY_COMPATIBLE_SAMPLE_RATE != audioFormat.sampleRateHz) {
//         ACSDK_ERROR(LX("isAudioFormatCompatibleFailed")
//                         .d("reason", "incompatibleSampleRate")
//                         .d("sensorySampleRate", SENSORY_COMPATIBLE_SAMPLE_RATE)
//                         .d("sampleRate", audioFormat.sampleRateHz));
//         return false;
//     }
//     if (SENSORY_COMPATIBLE_SAMPLE_SIZE_IN_BITS != audioFormat.sampleSizeInBits) {
//         ACSDK_ERROR(LX("isAudioFormatCompatibleFailed")
//                         .d("reason", "incompatibleSampleSizeInBits")
//                         .d("sensorySampleSizeInBits", SENSORY_COMPATIBLE_SAMPLE_SIZE_IN_BITS)
//                         .d("sampleSizeInBits", audioFormat.sampleSizeInBits));
//         return false;
//     }
//     if (SENSORY_COMPATIBLE_NUM_CHANNELS != audioFormat.numChannels) {
//         ACSDK_ERROR(LX("isAudioFormatCompatibleFailed")
//                         .d("reason", "incompatibleNumChannels")
//                         .d("sensoryNumChannels", SENSORY_COMPATIBLE_NUM_CHANNELS)
//                         .d("numChannels", audioFormat.numChannels));
//         return false;
//     }
//     return true;
 }

 void VinBigDataKeywordDetector::keyWordDetectedCallback(const char* key, void* userData, uint64_t begin, uint64_t end) {
     VinBigDataKeywordDetector* engine = static_cast<VinBigDataKeywordDetector*>(userData);
     const char* keyword = "hey vinfast";

     std::cout << "engine->m_beginIndexOfStreamReader : " << engine->m_beginIndexOfStreamReader << std::endl;

     engine->notifyKeyWordObservers(engine->m_stream, keyword, begin, end);
 }

std::unique_ptr<VinBigDataKeywordDetector> VinBigDataKeywordDetector::create(
    std::shared_ptr<avsCommon::avs::AudioInputStream> stream,
    avsCommon::utils::AudioFormat audioFormat,
    std::unordered_set<std::shared_ptr<avsCommon::sdkInterfaces::KeyWordObserverInterface>> keyWordObservers,
    std::unordered_set<std::shared_ptr<avsCommon::sdkInterfaces::KeyWordDetectorStateObserverInterface>>
        keyWordDetectorStateObservers,
    const std::string& modelFilePath,
    std::chrono::milliseconds msToPushPerIteration) {
     if (!stream) {
         ACSDK_ERROR(LX("createFailed").d("reason", "nullStream"));
         return nullptr;
     }

    // // TODO: ACSDK-249 - Investigate cpu usage of converting bytes between endianness and if it's not too much, do it.
    // if (isByteswappingRequired(audioFormat)) {
    //     ACSDK_ERROR(LX("createFailed").d("reason", "endianMismatch"));
    //     return nullptr;
    // }

    // if (!isAudioFormatCompatible(audioFormat)) {
    //     return nullptr;
    // }

    isAudioFormatCompatible(audioFormat);

    std::unique_ptr<VinBigDataKeywordDetector> detector(new VinBigDataKeywordDetector(
        stream, keyWordObservers, keyWordDetectorStateObservers, audioFormat, msToPushPerIteration));
    if (!detector->init(modelFilePath)) {
        ACSDK_ERROR(LX("createFailed").d("reason", "initDetectorFailed"));
        return nullptr;
    }
    return detector;
}

std::unique_ptr<VinBigDataKeywordDetector> VinBigDataKeywordDetector::create(
        std::shared_ptr<avsCommon::avs::AudioInputStream> stream,
        const std::shared_ptr<avsCommon::utils::AudioFormat>& audioFormat,
        const std::shared_ptr<acsdkKWDInterfaces::KeywordNotifierInterface> keywordNotifier,
        const std::shared_ptr<acsdkKWDInterfaces::KeywordDetectorStateNotifierInterface> keywordDetectorStateNotifier,
        const std::string& modelFilePath,
        std::chrono::milliseconds msToPushPerIteration) {
    if (!stream) {
        ACSDK_ERROR(LX("createFailed").d("reason", "nullStream"));
        return nullptr;
    }

//    // TODO: ACSDK-249 - Investigate cpu usage of converting bytes between endianness and if it's not too much, do it.
//    if (isByteswappingRequired(*audioFormat)) {
//        ACSDK_ERROR(LX("createFailed").d("reason", "endianMismatch"));
//        return nullptr;
//    }
//
//    if (!isAudioFormatCompatibleWithSensory(*audioFormat)) {
//        return nullptr;
//    }

    std::unique_ptr<VinBigDataKeywordDetector> detector(new VinBigDataKeywordDetector(
            stream, keywordNotifier, keywordDetectorStateNotifier, *audioFormat, msToPushPerIteration));
    if (!detector->init(modelFilePath)) {
        ACSDK_ERROR(LX("createFailed").d("reason", "initDetectorFailed"));
        return nullptr;
    }

    return detector;
}

VinBigDataKeywordDetector::VinBigDataKeywordDetector(
        std::shared_ptr<AudioInputStream> stream,
        std::shared_ptr<acsdkKWDInterfaces::KeywordNotifierInterface> keywordNotifier,
        std::shared_ptr<acsdkKWDInterfaces::KeywordDetectorStateNotifierInterface> keywordDetectorStateNotifier,
        avsCommon::utils::AudioFormat audioFormat,
        std::chrono::milliseconds msToPushPerIteration) :
        acsdkKWDImplementations::AbstractKeywordDetector(keywordNotifier, keywordDetectorStateNotifier),
        m_stream{stream},
        m_maxSamplesPerPush((audioFormat.sampleRateHz / HERTZ_PER_KILOHERTZ) * msToPushPerIteration.count()) {
    e2eMainProcess = nullptr;
}

VinBigDataKeywordDetector::~VinBigDataKeywordDetector() {
    m_isShuttingDown = true;
    if (m_detectionThread.joinable()) {
        m_detectionThread.join();
    }
    // snsrRelease(m_session);
    if (e2eMainProcess != nullptr) {
        delete e2eMainProcess;
        e2eMainProcess = nullptr;
    }
}

VinBigDataKeywordDetector::VinBigDataKeywordDetector(
    std::shared_ptr<AudioInputStream> stream,
    std::unordered_set<std::shared_ptr<KeyWordObserverInterface>> keyWordObservers,
    std::unordered_set<std::shared_ptr<KeyWordDetectorStateObserverInterface>> keyWordDetectorStateObservers,
    avsCommon::utils::AudioFormat audioFormat,
    std::chrono::milliseconds msToPushPerIteration) :
        AbstractKeywordDetector(keyWordObservers, keyWordDetectorStateObservers),
        m_stream{stream},
        m_maxSamplesPerPush((audioFormat.sampleRateHz / HERTZ_PER_KILOHERTZ) * msToPushPerIteration.count()) {
}

bool VinBigDataKeywordDetector::init(const std::string& modelFilePath) {
    m_streamReader = m_stream->createReader(AudioInputStream::Reader::Policy::BLOCKING);
    if (!m_streamReader) {
        ACSDK_ERROR(LX("initFailed").d("reason", "createStreamReaderFailed"));
        return false;
    }

    // wuw::setLogLevel(LOG_ERROR);
    wuw::setLogLevel(0);
    e2eMainProcess = new E2EV2MainProcessSubsampling2();
    // e2eMainProcess->initEngineFromFile(std::string("sdk-install/models/model"), std::string("sdk-install/models/feature"));
    e2eMainProcess->initEngineFromBundle(std::string("sdk-install/hey_vivi.bundle/info"), std::string("sdk-install/hey_vivi.bundle/data"));
    // e2eMainProcess = new E2EMainProcess(std::string("sdk-install/models/first"));
    // e2eMainProcess->initVerifier(std::string("sdk-install/models/second"));
    e2eMainProcess->setThreshold(0.9);
    m_isShuttingDown = false;
    m_detectionThread = std::thread(&VinBigDataKeywordDetector::detectionLoop, this);
    return true;
}

void VinBigDataKeywordDetector::detectionLoop() {
    std::cout << "detectionLoop" << std::endl;
    m_beginIndexOfStreamReader = m_streamReader->tell();
    notifyKeyWordDetectorStateObservers(KeyWordDetectorStateObserverInterface::KeyWordDetectorState::ACTIVE);
    std::vector<int16_t> audioDataToPush(m_maxSamplesPerPush);
    ssize_t wordsRead;
    uint64_t currentTime_ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    // SnsrRC result;
    while (!m_isShuttingDown) {
        bool didErrorOccur = false;
        wordsRead = readFromStream(
            m_streamReader,
            m_stream,
            audioDataToPush.data(),
            audioDataToPush.size(),
            TIMEOUT_FOR_READ_CALLS,
            &didErrorOccur);
        if (didErrorOccur) {
            std::cout << "didErrorOccur" << std::endl;
            /*
             * Note that this does not include the overrun condition, which the base class handles by instructing the
             * reader to seek to BEFORE_WRITER.
             */
            break;
        } else if (wordsRead == AudioInputStream::Reader::Error::OVERRUN) {
            std::cout << "AudioInputStream::Reader::Error::OVERRUN" << std::endl;
             /*
              * Updating reference point of Reader so that new indices that get emitted to keyWordObservers can be
              * relative to it.
              */
            m_beginIndexOfStreamReader = m_streamReader->tell();
        } else if (wordsRead > 0) {
            audio_buffer.insert(audio_buffer.end(), audioDataToPush.begin(), audioDataToPush.end());
            if (audio_buffer.size() > BUFFER_SIZE) {
                if (e2eMainProcess != nullptr) {
                    auto input = std::tuple<short*, size_t>(audio_buffer.data(), BUFFER_SIZE);
                    auto checkpoint = NOW_MS();
                    uint64_t now_time_ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
                    auto result = e2eMainProcess->acceptWaveform(input);
                    if(NOW_MS() - checkpoint > 200){
                        std::cout << "Total processing time for 200ms : " << NOW_MS() - checkpoint << " ms" << std::endl;
                    }
                    if (result && (now_time_ms - currentTime_ms) > 2000) {
                        printf("DIFF TIME %" PRIu64 "\n", now_time_ms - currentTime_ms);
                        currentTime_ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
                        std::cout << "Waked at " << m_streamReader->tell() << std::endl;
    //                    keyWordDetectedCallback(nullptr, reinterpret_cast<void*>(this), m_streamReader->tell() - 16000, m_streamReader->tell());
                        notifyKeyWordObservers(m_stream,
                                            "hey vinfast",
                                            avsCommon::sdkInterfaces::KeyWordObserverInterface::UNSPECIFIED_INDEX,
                                            m_streamReader->tell());
                    }
                }

                audio_buffer.erase(audio_buffer.begin(), audio_buffer.begin() + BUFFER_SIZE);
            }
        }
    }
    m_streamReader->close();
}

}  // namespace kwd
}  // namespace alexaClientSDK
