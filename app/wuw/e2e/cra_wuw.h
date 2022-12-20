//
// Created by ducnd on 15/10/2021.
//

#ifndef WAKEUP_CRA_WUW_H
#define WAKEUP_CRA_WUW_H

#include <string>
#include <aubio/fvec.h>
#include <c_api.h>
#include "../wuw.h"
#include "tflite.h"

class CraWuW:   public WuW      <std::vector<fvec_t*>, bool>,
                public TfLite   <std::vector<fvec_t*>, bool> {
public:
    CraWuW(std::string);
    ~CraWuW();

    virtual bool acceptWaveform     (std::vector<fvec_t*>& features);
    virtual void setThreshold       (float threshold) {this->threshold = threshold;}
    virtual void setGainDB          (float gainDB){this->gainDB = gainDB;}

protected:
    virtual void    setInputTensor  (std::vector<fvec_t*> features) throw(InferenceException);
    virtual bool    getOutputResult () throw(InferenceException);

private:
    void init                       ();
    void copyFeatureToData          (std::vector<fvec_t*>, float data[][64], int width, int height);

public:
    static const int frameLength;

private:
    // HMM area
    float               threshold;
    float               gainDB;

};

#endif //WAKEUP_SIRI_WUW_H
