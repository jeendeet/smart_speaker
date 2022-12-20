#include "../common/defs.h"
#ifdef USE_KALDI
#include "classifier.h"
#include <string.h>
#include <fstream>
#include <sstream>
#include "base/kaldi-common.h"
#include <cmath>
#include <utility>

WakeUpDecisionMaker::WakeUpDecisionMaker(){
    this->interpreter = 0;
    this->options = 0;
    this->model = 0;
    this->last_score = 0;
    this->threshold = 0.8;
}

WakeUpDecisionMaker::~WakeUpDecisionMaker() {
    if (this->interpreter != 0)  {
        TfLiteInterpreterDelete(this->interpreter);
        this->interpreter = 0;
    }

    if (this->options != 0) {
        TfLiteInterpreterOptionsDelete(this->options);
        this->options = 0;
    }

    if (this->model != 0) {
        TfLiteModelDelete(this->model);
        this->model = 0;
    }
}

bool WakeUpDecisionMaker::WakeUp(float meanFPP,  float stdvarFPP, float meanAC, float stdvarAC) {
    if (meanFPP == 1) {
        return false;
    }

    if (std::isnan(meanFPP) or std::isnan(stdvarFPP) or std::isnan(meanAC) or std::isnan(stdvarAC)) {
        return false;
    }

    if (std::isinf(meanFPP) or std::isinf(stdvarFPP) or std::isinf(meanAC) or std::isinf(stdvarAC)) {
        return false;
    }

    this->init();
    TfLiteInterpreterAllocateTensors(this->interpreter);

    TfLiteTensor* input = TfLiteInterpreterGetInputTensor(this->interpreter, 0);
    float data[4];
    data[0] = (meanFPP - mean_meanfpp) / stdvar_meanfpp;
    data[1] = (stdvarFPP - mean_stdvarfpp) / stdvar_stdvarfpp;
    data[2] = (meanAC - mean_meanac) / stdvar_meanac;
    data[3] = (stdvarAC - mean_stdvarac) / stdvar_stdvarac;

    TfLiteStatus status = TfLiteTensorCopyFromBuffer(input, data, 4 * sizeof(float));

    if (status != TfLiteStatus::kTfLiteOk) {
        this->init();
        return false;
    }

    status = TfLiteInterpreterInvoke(this->interpreter);

    if (status != TfLiteStatus::kTfLiteOk) {
        this->init();
        return false;
    }

    const TfLiteTensor* output_tensor = TfLiteInterpreterGetOutputTensor(interpreter, 0);

    float result[1] = {0};

    status = TfLiteTensorCopyToBuffer(output_tensor, result, sizeof(float));

    if (status != TfLiteStatus::kTfLiteOk) {
        this->init();
        return false;
    }

    // Prevent bug of tensorflow
    if (this->last_score == result[0] || result[0] == 0.5) {
        this->init();
        return false;
    }
    else {
        this->last_score = result[0];
    }

    KALDI_LOG << "FPP " << meanFPP << " STD FPP " << stdvarFPP << " AC " << meanAC << " STD AC " << stdvarAC << " CLASS " << result[0] << " THRESHOLD " << this->threshold;

    return result[0] > this->threshold;
}

void WakeUpDecisionMaker::init(std::string model_path, std::string norm_path) {
    this->model_path = std::move(model_path);
    this->norm_path = std::move(norm_path);

    this->init();
}

void WakeUpDecisionMaker::init() {
    KALDI_LOG << "INITIALIZE DSMAKER";

    this->last_score = -1;

    if (this->interpreter != 0)  {
        TfLiteInterpreterDelete(this->interpreter);
        this->interpreter = 0;
    }

    if (this->options != 0) {
        TfLiteInterpreterOptionsDelete(this->options);
        this->options = 0;
    }

    if (this->model != 0) {
        TfLiteModelDelete(this->model);
        this->model = 0;
    }

    std::vector<uint8_t> model_data = aesDecoder.decodeToBytes(this->model_path);

    // this->model = TfLiteModelCreateFromFile(model_path.c_str());
    this->model = TfLiteModelCreate(model_data.data(), model_data.size());
    this->options = TfLiteInterpreterOptionsCreate();
    this->interpreter = TfLiteInterpreterCreate(this->model, this->options);

    std::stringstream infile(aesDecoder.decodeToString(norm_path));
    infile >> this->mean_meanfpp >> this->mean_stdvarfpp >> this->mean_meanac >> this->mean_stdvarac
           >> this->stdvar_meanfpp >> this->stdvar_stdvarfpp >>  this->stdvar_meanac >> this->stdvar_stdvarac;
}

void WakeUpDecisionMaker::setThreshold(float threshold) {
    KALDI_LOG << "SET THRESHOLD " << threshold;
    this->threshold = threshold;
}

#endif