//
// Created by ducnd on 15/10/2021.
//

#include "common/defs.h"

#include "main_process.h"

#include "common/defs.h"
#include "common/feature_extractor.h"
#include "wuw.h"
#include "e2e/siri_wuw.h"
#include "e2e/cra_wuw.h"

#include <utility>
#include <ctime>
#include <chrono>
#include <vector>
#include <cmath>

#include "common/feature_extractor.h"
#include "common/log.h"

/**************************************************************************************************/
/******************************************* E2E BASED ********************************************/
/**************************************************************************************************/

E2EMainProcess::E2EMainProcess(std::string path) {
    counter = 0;
    mfccFeature = new AubioMfccFeature();
    fBankFeature = new AubioFBankFeature();
    wuw = new SiriWuWV2(std::move(path));
    cra = nullptr;
    lastWakeupTime = 0;

    vadThreshold = -40;
    vadEnable = false;
    lastSpeechTime = 0;

    autoGain = true;
    gainTarget = -30;

    LOGI("Create new main process");
}

E2EMainProcess::~E2EMainProcess() {
    delete mfccFeature;
    delete fBankFeature;
    delete wuw;
    delete cra;

    mfccFeature = nullptr;
    fBankFeature = nullptr;
    wuw = nullptr;
    cra = nullptr;

    LOGI("Delete main process");
}

bool E2EMainProcess::acceptWaveform(std::tuple<short*, size_t>& features) {
    auto data = std::get<0>(features);
    auto length = std::get<1>(features);

    uint64_t checkpoint1 = NOW_MS();

    auto accepted = checkRMSEVad(data, length);

    // Insert white if all conditions
    // - not a speech
    // - not speech for 300ms
    if (!accepted && checkpoint1 - lastSpeechTime > 300) {
        generateWhiteNoise(data, length);
    }

    // Extract feature
    std::vector<fvec_t*> mfccs;
    std::vector<fvec_t*> fbanks;

    auto input = std::tuple<const short*, size_t>(data, length);
    mfccFeature->acceptWaveform(input, mfccs);
    fBankFeature->acceptWaveform(input, fbanks);

    counter ++;

    list_mfccs.insert(list_mfccs.end(), mfccs.begin(), mfccs.end());
    appendFbank(fbanks);

    // Get wakeup result
    bool result = false;

    if (list_mfccs.size() >= 19) {
        for (int i = 0; i < list_mfccs.size() - 19; i++) {
            std::vector<fvec_t*> tmp(list_mfccs.begin() + i, list_mfccs.begin() + i + 19);
            result = wuw->acceptWaveform(tmp);
            if(result) {
                break;
            }
        }

//        if (checkpoint1 - lastWakeupTime > 1500) {
            if (result && cra != nullptr) {
                if (autoGain) {
                    cra->setGainDB(gainAmount);
                }
                else {
                    cra->setGainDB(0.0f);
                }
                result = cra->acceptWaveform(list_fbanks);

                if (result) {
                    lastWakeupTime = checkpoint1;
                }
            }
//        }
//        else {
//            result = false;
//        }


        // Clean
        for (auto it = list_mfccs.begin(); it != list_mfccs.end() - 18; it++) {
            del_fvec(*it);
        }
        list_mfccs.erase(list_mfccs.begin(), list_mfccs.end() - 18);
    }

    if (result) {
        wuw->clearBuffer();
    }

    uint64_t checkpoint2 = NOW_MS();

    LOGD("Done accept wave form for all process %lu ms", checkpoint2 - checkpoint1);

    return result;
}

void E2EMainProcess::refreshGainTarget(float chunk_power) {
    list_power.push_back(chunk_power);
    int rmNo = list_power.size() - 20;
    if (rmNo > 0) {
        list_power.erase(list_power.begin(), list_power.begin() + rmNo);
    }
    currentPower = 0.0;
    gainAmount = 0.0;

    if (list_power.size() > 0) {
        for (auto power : list_power) {
            currentPower += power;
        }

        currentPower /= list_power.size();
        gainAmount = gainTarget - 10 * log10(currentPower);
    }
}

void E2EMainProcess::initVerifier(std::string path) {
    cra = new CraWuW(path);
}

void E2EMainProcess::appendFbank(std::vector<fvec_t *> fbanks) {
    int remove_length = list_fbanks.size() + fbanks.size() - cra->frameLength;

    if (remove_length > 0) {
        for (auto it = list_fbanks.begin(); it != list_fbanks.begin() + remove_length; it++) {
            del_fvec(*it);
        }

        list_fbanks.erase(list_fbanks.begin(), list_fbanks.begin() + remove_length);
    }

    list_fbanks.insert(list_fbanks.end(), fbanks.begin(), fbanks.end());
}

void E2EMainProcess::setVerifyThreshold(float threshold) {
    this->cra->setThreshold(threshold);
}

void E2EMainProcess::setThreshold(float threshold) {
    this->wuw->setThreshold(threshold);
}

void E2EMainProcess::acceptWaveformForContext(std::tuple<short*, size_t>& features) {
    auto data = std::get<0>(features);
    auto length = std::get<1>(features);

    checkRMSEVad(data, length);

    std::vector<fvec_t*> fbanks;
    auto input = std::tuple<const short*, size_t>(data, length);

    fBankFeature->acceptWaveform(input, fbanks);
    appendFbank(fbanks);
}

void E2EMainProcess::setVADThreshold(float d) {
    this->vadThreshold = d;
}

bool E2EMainProcess::checkRMSEVad(short *data, size_t length) {
    float rmse = 0;

    if (length > 0 ) {
        // Compute energy
        for (int index = 0; index < length; index++) {
            auto tmp = (float) data[index] / 32768.0f;
            rmse += tmp * tmp;
        }

        // Compute db
        float mean_rmse = rmse / length;

        // Append it to list power
        refreshGainTarget(mean_rmse);

        // Compute power in dB
        mean_rmse = 10 * log10(mean_rmse);

        // Compare with threshold
        auto isSpeech = mean_rmse >= this->vadThreshold;
        if (isSpeech) {
            lastSpeechTime = NOW_MS();
        }

        LOGD("WAKEUP RMSE %f - PASS %d, target %f, current %f NEED TO GAIN AMOUNT %f dB", mean_rmse, isSpeech, gainTarget, 10 * log10(currentPower), gainAmount);


        if (vadEnable) {
            return isSpeech;
        }
        else {
            return true;
        }
    }
    else {
        return false;
    }
}

void E2EMainProcess::setVADEnable(bool enable) {
    this->vadEnable = enable;
}

void E2EMainProcess::generateWhiteNoise(short* data, size_t length) {
    for (int i = 0; i < length; i++) {
        data[i] = rand() % 31 - 15;
    }
}

void E2EMainProcess::setAutoGain(bool enable) {
    this->autoGain = enable;
}

void E2EMainProcess::setAutoGainTarget(float target) {
    this->gainTarget = target;
}

/**************************************************************************************************/
/****************************************** KALDI BASED *******************************************/
/**************************************************************************************************/
#ifdef USE_KALDI
KaldiMainProcess::KaldiMainProcess(std::string path) {
    model = new Model(path.c_str());
    kaldiBasedWuW = new KaldiBasedWuW();
    kaldiBasedWuW->setModel(model);
}

KaldiMainProcess::~KaldiMainProcess() {
    delete model;
    delete kaldiBasedWuW;

    model = nullptr;
    kaldiBasedWuW = nullptr;
}

bool KaldiMainProcess::acceptWaveform(std::tuple<short*, size_t>& features) {
    return kaldiBasedWuW->acceptWaveform(features);
}

void KaldiMainProcess::acceptWaveformForContext(std::tuple<short*, size_t>& features) {

}

void KaldiMainProcess::setVerifierThreshold(float d) {

}

void KaldiMainProcess::setWuwThreshold(float d) {

}
#endif

