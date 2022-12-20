//
// Created by ducnd on 15/10/2021.
//

#ifndef WAKEUP_SIRI_WUW_H
#define WAKEUP_SIRI_WUW_H

#include <string>
#include <aubio/fvec.h>
#include <c_api.h>

#include "../wuw.h"
#include "tflite.h"

class SiriWuW:  public WuW      <std::vector<fvec_t*>, bool>,
                public TfLite   <std::vector<fvec_t*>, bool> {
public:
    SiriWuW(std::string);
    ~SiriWuW();

    virtual bool acceptWaveform     (std::vector<fvec_t*>&);
    virtual void setThreshold       (float threshold) {this->threshold_wuw = threshold;}
    virtual void clearBuffer        ();

protected:
    virtual void init();
    virtual void advancedDecode     (float* result, int length);
    virtual bool makeDecision       ();

    virtual void    setInputTensor  (std::vector<fvec_t*> features)     throw(InferenceException);
    virtual bool    getOutputResult ()                                  throw(InferenceException);

protected:
    // HMM area
    int                 numOutput;
    int                 *heyvinfast;
    int                 *heyvinfast_counter;
    int                 heyvinfast_length;
    float               *prob_sum;
    int                 *length;
    int                 decode_counter;
    int                 final_index;
    float               threshold_wuw;

    bool                last_result;
};

class SiriWuWV2: public SiriWuW {
public:
    SiriWuWV2(std::string path);
    ~SiriWuWV2();

    virtual void clearBuffer        ();

protected:
    virtual void advancedDecode     (float* result, int length);
    virtual bool makeDecision       ();

protected:
    int                 *label_count;
};

#endif //WAKEUP_SIRI_WUW_H
