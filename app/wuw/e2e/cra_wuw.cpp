//
// Created by ducnd on 15/10/2021.
//

#include <utility>
#include <vector>
#include <cmath>
#include <cassert>
#include "../common/log.h"
#include "cra_wuw.h"

const int CraWuW::frameLength = 194;

CraWuW::CraWuW(std::string path): TfLite<std::vector<fvec_t*>, bool>(path) {
    this->threshold = 0.9f;

    // Initialize model
    this->init();
}

CraWuW::~CraWuW() {

}

bool CraWuW::acceptWaveform(std::vector<fvec_t*>& features) {
    try {
        return infer(features);
    }
    catch (InferenceException e) {
        LOGE("Exception when infer siri wuw %s", e.what());
        return false;
    }
}

void CraWuW::init() {
    TfLite::initModel();
}

/***
 * Copy data from array to tensorflow lite
 * Apply gain amount, note that
 * - features use log_e
 * - gainDB use log_10
 * --> Need to convert gainDB to log_e scale
 * @param features
 * @param data
 * @param width
 * @param height
 */
void CraWuW::copyFeatureToData(std::vector<fvec_t *> features, float data[][64], int width, int height) {
// Copy from features to data buffer
    assert(features.size() == width);
    int f_index = 0;
    int padding = 0;

    auto gainInLogE = gainDB * log(10) / 10;

    for (int fid = padding; fid < features.size(); fid++) {
        f_index = fid - padding;
        auto f = features[fid];
        for (int i = 0; i < height; i++) {
            data[f_index][i] = f->data[i] + gainInLogE;
        }
    }

    for (int fid = 0; fid < padding; fid++) {
        f_index = (features.size() - padding + fid) % features.size();
        auto f = features[fid];
        for (int i = 0; i < height; i++) {
            data[f_index][i] = f->data[i] - fid * 0.2f;
        }
    }
}

void CraWuW::setInputTensor(std::vector<fvec_t *> features) throw(InferenceException) {
    // Create data buffer
    const int length = frameLength;
    const int size = length * 64;
    float data[length][64] = {0};

    if (features.size() < length) {
        LOGI("Less than 195");
        throw InferenceException("Less than 195");
    }

    copyFeatureToData(features, data, length, 64);

    // Copy buffer to tensor
    TfLiteStatus status = TfLiteTensorCopyFromBuffer(input, data, size * sizeof(float));
    if (status != TfLiteStatus::kTfLiteOk) {
        LOGE("TfLiteTensorCopyFromBuffer");
        throw InferenceException("TfLiteTensorCopyFromBuffer");
    }
}

bool CraWuW::getOutputResult() throw(InferenceException) {
    // Copy from output tensor to buffer
    // output: 1 x 14
    float result[1] = {0};
    TfLiteStatus status = TfLiteTensorCopyToBuffer(output, result, 1 * sizeof(float));
    if (status != TfLiteStatus::kTfLiteOk) {
        LOGE("TfLiteTensorCopyToBuffer");
        throw InferenceException("TfLiteTensorCopyToBuffer");
    }

    LOGD("Verifier %f", result[0]);

    return result[0] >= this->threshold;
}