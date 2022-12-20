//
// Created by ducnd on 15/10/2021.
//

#ifndef WAKEUP_FEATURE_EXTRACTOR_H
#define WAKEUP_FEATURE_EXTRACTOR_H

#include <aubio/aubio.h>
#include <aubio/fvec.h>
#include <aubio/cvec.h>
#include <cstdlib>
#include <vector>
#include <cmath>
#include <tuple>

template <class RawInputType, class FeatureType>
class Feature {
    virtual void acceptWaveform     (RawInputType&, FeatureType&) = 0;
};

class AubioMfccFeature: Feature<std::tuple<const short*, size_t>, std::vector<fvec_t*>>{
public:
    AubioMfccFeature();
public:
    void acceptWaveform             (std::tuple<const short*, size_t>&, std::vector<fvec_t*>&);
private:
    void lifter                     (std::vector<fvec_t*>&);

    int             sample_rate;
    int             buffer_size;
    int             hop_size;
    int             n_filters;
    int             n_coefs;

    bool            is_liftered;

    aubio_pvoc_t    *pv;
    cvec_t          *fftgrain;
    aubio_mfcc_t    *mfcc;

    std::vector<fvec_t*>  left_context;
};

class AubioFBankFeature: Feature<std::tuple<const short*, size_t>, std::vector<fvec_t*>> {
public:
    AubioFBankFeature();
public:
    void acceptWaveform             (std::tuple<const short*, size_t>&, std::vector<fvec_t*>&);
private:
    void setFilterBanks();

    int                     sample_rate;
    int                     buffer_size;
    int                     hop_size;
    int                     n_filters;

    aubio_pvoc_t            *pv;
    cvec_t                  *fftgrain;
    aubio_filterbank_t      *fbank;

    std::vector<fvec_t*>    left_context;
};


#endif //WAKEUP_FEATURE_EXTRACTOR_H
