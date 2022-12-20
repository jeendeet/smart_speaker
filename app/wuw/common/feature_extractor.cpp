//
// Created by ducnd on 15/10/2021.
//

#include <vector>

#include "feature_extractor.h"
#include "log.h"

AubioMfccFeature::AubioMfccFeature() {
    this->sample_rate = 16000;
    this->buffer_size = 512;
    this->hop_size = 160;
    this->n_filters = 40;
    this->n_coefs = 13;

    this->pv = new_aubio_pvoc(buffer_size, hop_size);
    this->fftgrain = new_cvec(buffer_size);
    mfcc = new_aubio_mfcc(buffer_size, n_filters, n_coefs, sample_rate);

    this->is_liftered = true;
}

void AubioMfccFeature::lifter(std::vector<fvec_t *> &features) {
    int L = 22;
    if (L > 0) {
        float lift[this->n_coefs];
        for (int i = 0; i < n_coefs; i++) {
            lift[i] = 1 + (float) L / 2 * sin(M_PI * i / L);
        }

        for (auto feature : features) {
            for (int i = 0; i < n_coefs; i++) {
                feature->data[i] *= lift[i];
            }
        }
    }
}

void AubioMfccFeature::acceptWaveform(std::tuple<const short *, size_t> &rawData, std::vector<fvec_t *> &features) {
    const short *data = std::get<0>(rawData);
    int length = std::get<1>(rawData);

// Create feature vector
    fvec_t *in = new_fvec(hop_size); // ??? buffer_size or hop_s ??? . Training are using hop_s

    uint64_t checkpoint1 = NOW_MS();

    // Append current data
    for (int index = 0; index < length - hop_size; index += hop_size) {
        // prepare data
        for (int i = 0; i < hop_size; i++) {
            in->data[i] = (float) data[i + index] / 32768.0f;
        }
        fvec_t *_out = new_fvec(n_coefs);

        // Convert to feature
        aubio_pvoc_do(pv, in, fftgrain);
        aubio_mfcc_do(mfcc, fftgrain, _out);
        features.push_back(_out);
    }

    if (this->is_liftered) {
        lifter(features);
    }

    uint64_t checkpoint2 = NOW_MS();

    LOGI("Done accept wave form for feature cost %lu ms", checkpoint2 - checkpoint1);
    del_fvec(in);
}


AubioFBankFeature::AubioFBankFeature() {
    this->sample_rate = 16000;
    this->buffer_size = 512;
    this->hop_size = 160;
    this->n_filters = 64;

    this->pv = new_aubio_pvoc(buffer_size, hop_size);
    this->fftgrain = new_cvec(buffer_size);
    this->fbank = new_aubio_filterbank(this->n_filters, this->buffer_size);
    setFilterBanks();
}

void AubioFBankFeature::acceptWaveform(std::tuple<const short *, size_t> &rawData, std::vector<fvec_t *> &features) {
    const short *data = std::get<0>(rawData);
    int length = std::get<1>(rawData);

    // Create feature vector
    fvec_t *in = new_fvec(this->hop_size); // ??? buffer_size or hop_s ??? . Training are using hop_s

    uint64_t checkpoint1 = NOW_MS();

    // Append current data
    for (int index = 0; index < length - hop_size; index += hop_size) {
        // prepare data
        for (int i = 0; i < hop_size; i++) {
            in->data[i] = (float) data[i + index] / 32768.0f;
        }
        fvec_t *_out = new_fvec(this->n_filters);

        // Convert to feature
        aubio_pvoc_do(pv, in, fftgrain);
        aubio_filterbank_do(this->fbank, fftgrain, _out);

        // Compute log mel
        for (int i = 0; i < this->n_filters; i++) {
            _out->data[i] += 1e-6;
        }
        fvec_log(_out);

        features.push_back(_out);
    }

    uint64_t checkpoint2 = NOW_MS();

    LOGI("Done accept wave form for feature cost %lu ms", checkpoint2 - checkpoint1);
    del_fvec(in);
}

void AubioFBankFeature::setFilterBanks() {
    uint_t retval;

    smpl_t nyquist_hertz = this->sample_rate / 2;
    smpl_t minFrequency = 80;
    smpl_t maxFrequency = 7600;

    smpl_t minMelBankFrequency = 2595 * log10(1 + minFrequency / 700);
    smpl_t maxMelBankFrequency = 2595 * log10(1 + maxFrequency / 700);
    smpl_t linearSpacingMelBank = (maxMelBankFrequency - minMelBankFrequency) / (this->n_filters + 1);

    fvec_t *freqs = new_fvec (n_filters + 2);

    for (int i = 0; i < this->n_filters + 2; i++) {
        smpl_t melFrequency = minMelBankFrequency + linearSpacingMelBank * i;
        smpl_t hertzFrequency = (pow(10, melFrequency / 2595) - 1) * 700;
        freqs->data[i] = hertzFrequency;
    }

    /* now compute the actual coefficients */
    aubio_filterbank_set_triangle_bands (this->fbank, freqs, this->sample_rate);

    /* destroy vector used to store frequency limits */
    del_fvec (freqs);
}
