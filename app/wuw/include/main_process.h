//
// Created by ducnd on 15/10/2021.
//

#ifndef WAKEUP_MAIN_PROCESS_H
#define WAKEUP_MAIN_PROCESS_H

#ifdef USE_AUBIO
#include <aubio/fvec.h>
#endif //USE_AUBIO
#include <tuple>
#include <string>
#include <vector>
#include "wuw.h"
#include "common/bundle.h"

class AubioMfccFeature;
class AubioFBankFeature;
class SiriWuW;
class CraWuW;
class CraWuWWithInput;
class AsrBackendFBankFeature;
class AsrBackendWuW;
class AsrBackendMFCCFeature;
class CraWuWV2;
class AsrBackendWuWSubsampling2;


#ifdef USE_KALDI
class KaldiBasedWuW;
class Model;
#endif

class MainProcess: public WuW<std::tuple<short*, size_t>, bool> {
    virtual void acceptWaveformForContext   (std::tuple<short*, size_t>&)   = 0;
    virtual void setVerifyThreshold         (float d)                       = 0;
    virtual void setThreshold               (float d)                       = 0;
};

#ifdef USE_AUBIO
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

protected:
    virtual void appendFbank                        (std::vector<fvec_t*> fbanks);
    virtual void refreshGainTarget                  (float power);
    virtual bool checkRMSEVad                       (short* data, size_t length);
    virtual void generateWhiteNoise                 (short* data, size_t length);

protected:
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

class E2EMainProcessTuned: public E2EMainProcess {
public:
    E2EMainProcessTuned(std::string path);
    virtual ~E2EMainProcessTuned();

    virtual bool acceptWaveform             (std::tuple<short*, size_t>&);
    virtual void acceptWaveformForContext   (std::tuple<short*, size_t>&);
    virtual void initVerifier               (std::string basicString);
    virtual void initFeature                (std::string basicString);
    virtual void setVerifyThreshold         (float d);
    virtual void appendFBankV2              (std::vector<float*> &fbanks);
protected:
    AsrBackendFBankFeature      *fBankFeature2;
    CraWuWV2                    *craV2;
};

#endif

struct E2EV2MainProcessConfig {
    int modelIndex;
    int modelSize;
    int extractorIndex;
    int extractorSize;
    float threshold;
    
    E2EV2MainProcessConfig(): modelIndex(0),
    modelSize(0),
    extractorIndex(0),
    extractorSize(0),
    threshold(0.975f){}
    
    void Register(OptionsItf* opts) {
        opts->Register("model-index", &modelIndex, "Index of model");
        opts->Register("model-size", &modelSize, "Size of model");
        opts->Register("extractor-index", &extractorIndex, "Index of extractor");
        opts->Register("extractor-size", &extractorSize, "Size of extractor");
        opts->Register("threshold", &threshold, "Threshold");
    }
};

class E2EV2MainProcessBundle: public AESBundle {
public:
    E2EV2MainProcessBundle(): AESBundle() {}

    virtual void onRegisterConfig();
    virtual void onReadedConfig();

    E2EV2MainProcessConfig config;
};

class E2EV2MainProcess: public WuW<std::tuple<short*, size_t>, bool> {
public:
    E2EV2MainProcess();
    virtual ~E2EV2MainProcess();
    
    virtual void initEngineFromFile(std::string model, std::string extractor);
    virtual void initEngineFromBundle(std::string info, std::string data);

    virtual bool acceptWaveform             (std::tuple<short*, size_t>&);
    virtual void setThreshold               (float d);

private:
    AsrBackendWuW               *wuw;
    AsrBackendMFCCFeature       *feature;
};

class E2EV2MainProcessSubsampling2: public WuW<std::tuple<short*, size_t>, bool> {
public:
    E2EV2MainProcessSubsampling2();
    virtual ~E2EV2MainProcessSubsampling2();

    virtual void initEngineFromFile(std::string model, std::string extractor);
    virtual void initEngineFromBundle(std::string info, std::string data);

    virtual bool acceptWaveform             (std::tuple<short*, size_t>&);
    virtual void setThreshold               (float d);

private:
    AsrBackendWuWSubsampling2       *wuw;
    AsrBackendMFCCFeature           *feature;
    float                           *stackdata;
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
