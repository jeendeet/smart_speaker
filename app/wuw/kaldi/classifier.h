#include "../common/defs.h"
#ifdef USE_KALDI

#ifndef  CLASSIFIER
#define CLASSIFIER
#include <string>
#include "builtin_ops.h"
#include "c_api.h"
#include "common.h"
#include "../common/cipher.h"

class WakeUpDecisionMaker {
public:
    WakeUpDecisionMaker();
    ~WakeUpDecisionMaker();
    void init(std::string, std::string);
    bool WakeUp(float meanFPP,  float stdvarFPP, float meanAC, float stdvarAC);
    void setThreshold(float threshold);

private:
    void init();

    float last_score;

    float mean_meanfpp;
    float stdvar_meanfpp;

    float mean_stdvarfpp;
    float stdvar_stdvarfpp;

    float mean_meanac;
    float stdvar_meanac;

    float mean_stdvarac;
    float stdvar_stdvarac;
    float threshold;

    std::string model_path;
    std::string norm_path;

    TfLiteModel* model;
    TfLiteInterpreterOptions* options;
    TfLiteInterpreter* interpreter;
    AESDecoder aesDecoder;
};

#endif

#endif