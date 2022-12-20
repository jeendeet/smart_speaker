//
// Created by ducnd on 12/15/21.
//

#ifndef WAKEUP_TFLITE_H
#define WAKEUP_TFLITE_H

#include <c_api.h>
#include <aubio/fvec.h>
#include <vector>
#include <cstring>

#include "../common/log.h"
#include "../common/exception.h"

template<typename FeatureType, typename ResultType>
class TfLite {
public:
    TfLite(std::string path): model_path(path) {
        this->interpreter = nullptr;
        this->options = nullptr;
        this->model = nullptr;

        this->cmodel_path = new char[512];
        memset(cmodel_path, 0, 512);
        memcpy(cmodel_path, path.c_str(), path.length());

        LOGI("Create new tensorflow lite object of path %s", cmodel_path);
    }

    ~TfLite() {
        LOGI("Delete tensorflow lite object of path %s", cmodel_path);

        if (this->interpreter != nullptr)  {
            TfLiteInterpreterDelete(this->interpreter);
            this->interpreter = nullptr;
        }

        if (this->options != nullptr) {
            TfLiteInterpreterOptionsDelete(this->options);
            this->options = nullptr;
        }

        if (this->model != nullptr) {
            TfLiteModelDelete(this->model);
            this->model = nullptr;
        }

        if (this->cmodel_path != nullptr) {
            delete [] cmodel_path;
            this->cmodel_path = nullptr;
        }
    }

    virtual void initModel() {
        LOGI("Initialize tensorflow lite model");

        if (this->interpreter != nullptr)  {
            LOGI("Delete old interpreter");
            TfLiteInterpreterDelete(this->interpreter);
            this->interpreter = nullptr;
        }

        if (this->options != nullptr) {
            LOGI("Delete old option");
            TfLiteInterpreterOptionsDelete(this->options);
            this->options = nullptr;
        }

        if (this->model != nullptr) {
            LOGI("Delete old model");
            TfLiteModelDelete(this->model);
            this->model = nullptr;
        }

        LOGI("Create new model");
        this->model = TfLiteModelCreateFromFile(cmodel_path);

        LOGI("Create new option");
        this->options = TfLiteInterpreterOptionsCreate();

        LOGI("Create new interpreter");
        this->interpreter = TfLiteInterpreterCreate(this->model, this->options);

        LOGI("Allocate tensors for interpreter");
        TfLiteInterpreterAllocateTensors(this->interpreter);
        input = TfLiteInterpreterGetInputTensor(this->interpreter, 0);
        output = TfLiteInterpreterGetOutputTensor(this->interpreter, 0);
    }

    /***
     * Template for call an inference
     * @param features
     * @return
     */
    virtual ResultType infer(FeatureType features) throw(InferenceException){
        this->setInputTensor(features);

        // Run infer
        TfLiteStatus status = TfLiteInterpreterInvoke(this->interpreter);
        if (status != TfLiteStatus::kTfLiteOk) {
            LOGE("TfLiteInterpreterInvoke");
            throw InferenceException("TfLiteInterpreterInvoke");
        }

        return getOutputResult();
    }

protected:
    virtual void        setInputTensor(FeatureType features)    throw(InferenceException) = 0;
    virtual ResultType  getOutputResult()                       throw(InferenceException) = 0;

protected:
    std::string                 model_path;

    TfLiteModel                 *model;
    TfLiteInterpreterOptions    *options;
    TfLiteInterpreter           *interpreter;
    char                        *cmodel_path;

    TfLiteTensor                *input;
    const TfLiteTensor          *output;
};

// template class TfLite<std::vector<fvec_t*>, bool>;

#endif //WAKEUP_TFLITE_H
