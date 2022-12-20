//
// Created by ducnd on 15/10/2021.
//

#include <utility>
#include <vector>
#include <cmath>
#include <cassert>
#include "../common/log.h"
#include "siri_wuw.h"

float norm_length(float length) {
    float var = 0.5;
    float mean = 0.9;
    float result = 1 / (var * sqrt(2 * M_PI)) * exp(- ((length - mean) / var) * ((length - mean) / var) / 2);
    return result;
}

SiriWuW::SiriWuW(std::string path): TfLite<std::vector<fvec_t*>, bool>(path) {
    // Init for HMM
    this->numOutput = 14;       // 14 output of dnn
    this->prob_sum = new float[this->numOutput];
    this->length = new int[this->numOutput];

    for (int i = 0; i < numOutput; i++) {
        prob_sum[i] = -100;
        length[i] = 1;
    }

    // Init for wakeup word phones
    threshold_wuw = -4;
    heyvinfast = new int[9]{0, 10, 11, 2, 3, 4, 5, 6, 7};    // Phones of hey vinfast
    heyvinfast_length = 9;
    final_index = 7;

    // Set first time
    decode_counter = 0;

    last_result = false;

    // Initialize model
    this->init();
}

SiriWuW::~SiriWuW() {
    if (this->prob_sum != nullptr)  {
        delete [] this->prob_sum;
    }

    if (this->length != nullptr)  {
        delete [] this->length;
    }

    if (this->heyvinfast != nullptr)  {
        delete [] this->heyvinfast;
    }
}

bool SiriWuW::acceptWaveform(std::vector<fvec_t *>& features) {
    try {
        return infer(features);
    }
    catch (InferenceException e) {
        LOGE("Exception when infer siri wuw %s", e.what());
        return false;
    }
}

void SiriWuW::setInputTensor(std::vector<fvec_t *> features) throw(InferenceException) {
    // Create data buffer
    const int size = 13 * 19;
    float data[13][19] = {0};

    // Copy from features to data buffer
    assert(features.size() == 19);
    int f_index = 0;
    for(fvec_t* f : features) {
        for (int i = 0; i < 13; i++) {
            data[i][f_index] = f->data[i];
        }
        f_index++;
    }

    // Copy buffer to tensor
    TfLiteStatus status = TfLiteTensorCopyFromBuffer(input, data, size * sizeof(float));
    if (status != TfLiteStatus::kTfLiteOk) {
        LOGE("TfLiteTensorCopyFromBuffer");
        throw InferenceException("TfLiteTensorCopyFromBuffer");
    }
}

bool SiriWuW::getOutputResult() throw(InferenceException) {
    // Copy from output tensor to buffer
    // output: 1 x 14
    float result[14] = {0};
    TfLiteStatus status = TfLiteTensorCopyToBuffer(output, result, 14 * sizeof(float));
    if (status != TfLiteStatus::kTfLiteOk) {
        LOGE("TfLiteTensorCopyToBuffer");
        return false;
    }

    // Compute HMM
    advancedDecode(result, 14);

    // Result
    return makeDecision();
}

void SiriWuW::init() {
    TfLite::initModel();
}

void SiriWuW::advancedDecode(float *result, int _length) {
    assert(_length == this->numOutput);

    float cutoffThreshold = -3;
    float cutoffReplacement = -32;
    float next_transition = 0;
    float loop_transition = 0;

    // Compute log probability
    float logProb[_length];
    for (int i = 0; i < _length; i++) {
        logProb[i] = log(result[i]);

        if (logProb[i] < cutoffThreshold) {
//            logProb[i] = cutoffReplacement;
            logProb[i] += (logProb[i] - cutoffThreshold) * 5;
        }
    }

    // Initialize tmp array
    float current_prob_sum[this->numOutput];
    int current_length[this->numOutput];

    for (int i = 0; i < numOutput; i++) {
        current_prob_sum[i] = -100;
        current_length[i] = 1;
    }

    // Compute HMM
    for (int _j = 0; _j < heyvinfast_length; _j++) {
        int j = heyvinfast[_j];                         // phoneme j th in dnn output
        int prev_j = _j > 0 ? heyvinfast[_j - 1] : 0;   // phoneme prev_j th in dnn output

        if (decode_counter > j) {
            if (j == 0) {
                current_prob_sum[j] = result[j];
                current_length[j] = 1;
            }
            else {
                float loop_prob = prob_sum[j] + loop_transition;
                float next_prob = prob_sum[prev_j] + next_transition;

                if ((loop_prob / length[j]) > (next_prob / length[prev_j])) {
                    current_prob_sum[j] = loop_prob + logProb[j];
                    current_length[j] = length[j] + 1;
                }
                else {
                    current_prob_sum[j] = next_prob + logProb[j];
                    current_length[j] = length[prev_j] + 1;
                }
            }
        }
        else {
            current_prob_sum[j] = 0;
            decode_counter++;
        }
    }

    // Swap to main
    for (int i = 0; i < numOutput; i++) {
        prob_sum[i] = current_prob_sum[i];
        length[i] = current_length[i];
    }
}

void SiriWuW::clearBuffer() {
    for (int i = 0; i < numOutput; i++) {
        prob_sum[i] = -100;
        length[i] = 1;
    }

    decode_counter = 0;
}

bool SiriWuW::makeDecision() {
    if (length[final_index] != 0) {
        float score = prob_sum[final_index] / ((float) length[final_index] * norm_length((float) length[final_index] / 100.0f));

        if (score > threshold_wuw) {
            LOGD("TING");

            if (!last_result) {
                last_result = true;
                return true;
            }

            last_result = true;
        }
        else {
            last_result = false;
        }
        LOGD("HMM for hey vinfast %f", score);
    }

    return false;
}

/**************************************************************************************************/

#define INDEX(x, y) (x * 14 + y)

#define COPY(x,y,len){              \
    for(int i = 0; i < len; i++) {  \
        x[i] = i[i];                \
    }                               \
}

SiriWuWV2::SiriWuWV2(std::string path) : SiriWuW(path) {
    // Init for wakeup word phones
    threshold_wuw = -6;
    heyvinfast = new int[10]{0, 10, 11, 12, 2, 3, 4, 5, 6, 7};    // Phones of hey vinfast with sil in the middle
    heyvinfast_length = 10;

    label_count = new int[14 * 14];

    for (int i = 0; i < 14; i++) {
        for (int j = 0; j < 14; j++) {
            label_count[INDEX(i, j)] = 0;
        }
    }
}

SiriWuWV2::~SiriWuWV2() {
    delete [] label_count;
}

void SiriWuWV2::advancedDecode(float *result, int _length) {
    assert(_length == this->numOutput);

    float cutoffThreshold = -3;
    float cutoffReplacement = -32;
    float next_transition = 0;
    float loop_transition = 0;
    int   tmp_label_count[14 * 14];

    for (int i = 0; i < 14; i++) {
        for (int j = 0; j < 14; j++) {
            tmp_label_count[INDEX(i, j)] = 0;
        }
    }

    // Compute log probability
    float logProb[_length];
    for (int i = 0; i < _length; i++) {
        logProb[i] = log(result[i]);

        if (logProb[i] < cutoffThreshold) {
//            logProb[i] = cutoffReplacement;
            logProb[i] += (logProb[i] - cutoffThreshold) * 5;
        }
    }

    // Initialize tmp array
    float current_prob_sum[this->numOutput];
    int current_length[this->numOutput];

    for (int i = 0; i < numOutput; i++) {
        current_prob_sum[i] = -100;
        current_length[i] = 1;
    }

    // Compute HMM
    for (int _j = 0; _j < heyvinfast_length; _j++) {
        int j = heyvinfast[_j];                         // phoneme j th in dnn output
        int prev_j = _j > 0 ? heyvinfast[_j - 1] : 0;   // phoneme prev_j th in dnn output

        if (decode_counter > j) {
            if (j == 0) {
                current_prob_sum[j] = result[j];
                current_length[j] = 1;
            }
            else {
                float loop_prob = prob_sum[j] + loop_transition;
                float next_prob = prob_sum[prev_j] + next_transition;

                if ((loop_prob / length[j]) > (next_prob / length[prev_j])) {
                    if (j == 12) { // SIL
                        current_prob_sum[j] = loop_prob;
                        current_length[j] = length[j];
                    }
                    else { // Non - SIL
                        current_prob_sum[j] = loop_prob + logProb[j];
                        current_length[j] = length[j] + 1;
                    }

                    // Copy last label count
                    for (int t = 0; t < 14; t++) {
                        tmp_label_count[INDEX(j, t)] = label_count[INDEX(j, t)];
                    }

                    tmp_label_count[INDEX(j, j)] += 1;
                }
                else {
                    if (j == 12) {
                        current_prob_sum[j] = next_prob;
                        current_length[j] = length[prev_j];
                    }
                    else {
                        current_prob_sum[j] = next_prob + logProb[j];
                        current_length[j] = length[prev_j] + 1;
                    }

                    // Copy last label count
                    for (int t = 0; t < 14; t++) {
                        tmp_label_count[INDEX(j, t)] = label_count[INDEX(prev_j, t)];
                    }

                    tmp_label_count[INDEX(j, j)] += 1;
                }
            }
        }
        else {
            current_prob_sum[j] = -100;
            decode_counter++;
        }
    }

    // Swap to main
    for (int i = 0; i < numOutput; i++) {
        prob_sum[i] = current_prob_sum[i];
        length[i] = current_length[i];
    }

    for (int i = 0; i < 14; i++) {
        for (int j = 0; j < 14; j++) {
            label_count[INDEX(i, j)] = tmp_label_count[INDEX(i, j)];
        }
    }
}

bool SiriWuWV2::makeDecision() {
    LOGD("SIL COUNT %d", label_count[INDEX(final_index, 12)]);
    LOGD("length %d", length[final_index]);
    if (length[final_index] != 0 && label_count[INDEX(final_index, 12)] < 40) {
        float score = prob_sum[final_index] / ((float) length[final_index] * norm_length((float) length[final_index] / 100.0f));

        float total = 0.0;
        float mean = 0.0;
        float std = 0.0;

        for (int i = 0; i < this->heyvinfast_length; i++) {
            int phoneId = this->heyvinfast[i];

            if (phoneId == 0 || phoneId == 12) { // not count first phone and sil
                continue;
            }

            total += label_count[INDEX(final_index, phoneId)];
        }

        for (int i = 0; i < this->heyvinfast_length; i++) {
            int phoneId = this->heyvinfast[i];

            if (phoneId == 0 || phoneId == 12) { // not count first phone and sil
                continue;
            }

            mean += label_count[INDEX(final_index, phoneId)] / total;
        }

        mean = mean / (this->heyvinfast_length - 2);

        for (int i = 0; i < this->heyvinfast_length; i++) {
            int phoneId = this->heyvinfast[i];

            if (phoneId == 0 || phoneId == 12) { // not count first phone and sil
                continue;
            }
            float tmp = label_count[INDEX(final_index, phoneId)] / total - mean;
            std += tmp * tmp;
        }

        std = sqrt(std / (this->heyvinfast_length - 2));

        LOGD("TOTAL %f", total);
        LOGD("MEAN %f", mean);
        LOGD("STD %f", std);

        if (score > threshold_wuw && std <= 0.12) {
            LOGD("TING");

            for (int i = 0; i < this->numOutput; i++) {
                LOGD("Num frame for phone %d %d", i, label_count[INDEX(final_index, i)]);
            }

            return true;

            if (!last_result) {
                last_result = true;
                return true;
            }

            last_result = true;
        }
        else {
            last_result = false;
        }
        LOGD("HMM for hey vinfast %f", score);
    }
    else {
        clearBuffer();
    }

    return false;
}

void SiriWuWV2::clearBuffer() {
    SiriWuW::clearBuffer();

    for (int i = 0; i < 14; i++) {
        for (int j = 0; j < 14; j++) {
            label_count[INDEX(i, j)] = 0;
        }
    }
}
