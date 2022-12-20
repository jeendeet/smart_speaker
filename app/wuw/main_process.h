//
// Created by ducnd on 15/10/2021.
//

#ifndef WAKEUP_MAIN_PROCESS_H
#define WAKEUP_MAIN_PROCESS_H

#include <aubio/fvec.h>
#include <tuple>
#include <string>
#include <vector>
#include "wuw.h"

class AubioMfccFeature;
class AubioFBankFeature;
class SiriWuW;
class CraWuW;


#ifdef USE_KALDI
class KaldiBasedWuW;
class Model;
#endif

class MainProcess: public WuW<std::tuple<short*, size_t>, bool> {
    virtual void acceptWaveformForContext   (std::tuple<short*, size_t>&)   = 0;
    virtual void setVerifyThreshold         (float d)                       = 0;
    virtual void setThreshold               (float d)                       = 0;
};

class E2EMainProcess: public MainProcess {
public:
    E2EMainProcess(std::string path);
    virtual ~E2EMainProcess();

    virtual bool acceptWaveform             (std::tuple<short*, size_t>&);
    virtual void acceptWaveformForContext   (std::tuple<short*, size_t>&);
    virtual void initVerifier               (std::string basicString);
    virtual void setVerifyThreshold         (float d);
    virtual void setThreshold               (float d);
    virtual void setVADThreshold            (float d);
    virtual void setVADEnable               (bool enable);
    virtual void setAutoGain                (bool enable);
    virtual void setAutoGainTarget          (float target);

private:
    void appendFbank                        (std::vector<fvec_t*> fbanks);
    void refreshGainTarget                  (float power);
    bool checkRMSEVad                       (short* data, size_t length);
    void generateWhiteNoise                 (short* data, size_t length);

private:
    int                 counter;

    float               vadThreshold;
    bool                vadEnable;
    uint64_t            lastSpeechTime;
    uint64_t            lastWakeupTime;

    AubioMfccFeature         *mfccFeature;
    AubioFBankFeature        *fBankFeature;
    SiriWuW             *wuw;
    CraWuW              *cra;

    std::vector<fvec_t*>            list_mfccs;
    std::vector<fvec_t*>::iterator  current;
    std::vector<fvec_t*>            list_fbanks;

    std::vector<float>  list_power;
    bool                autoGain;
    float               gainTarget;
    float               currentPower;
    float               gainAmount;
};

#ifdef USE_KALDI

class KaldiMainProcess: public MainProcess {
public:
    KaldiMainProcess(std::string path);
    virtual ~KaldiMainProcess();

    virtual bool acceptWaveform             (std::tuple<short*, size_t>& features);
    virtual void acceptWaveformForContext   (std::tuple<short*, size_t>& features);
    virtual void setVerifierThreshold       (float d);
    virtual void setWuwThreshold            (float d);

private:
    KaldiBasedWuW   *kaldiBasedWuW;
    Model           *model;
};
#endif


#endif //WAKEUP_MAIN_PROCESS_H
