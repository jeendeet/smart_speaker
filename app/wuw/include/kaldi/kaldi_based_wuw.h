//
// Created by ducnd on 15/10/2021.
//

#ifndef KALDI_WUW_H
#define KALDI_WUW_H

#include "../common/defs.h"
#ifdef USE_KALDI

#include "../wuw.h"
#include <string>
#include "c_api.h"
#include "model.h"
#include "lattice_analysis.h"
#include "classifier.h"
#include "lattice_analysis.h"
#include "classifier.h"
#include "online_nnet3_decoding_bdi.h"
#include "online_ivector_feature_bdi.h"

enum KaldiRecognizerState {
    RECOGNIZER_INITIALIZED,
    RECOGNIZER_RUNNING,
    RECOGNIZER_ENDPOINT,
    RECOGNIZER_FINALIZED
};

class KaldiBasedWuW: public WuW<std::tuple<short*, size_t>, bool> {
public:
    KaldiBasedWuW();
    ~KaldiBasedWuW();
    bool acceptWaveform         (std::tuple<short*, size_t>& feature);
    void setModel               (Model* model);

private:
    bool acceptWaveform         (Vector<BaseFloat>& wdata);
    void clean                  ();
    void updateSilenceWeights   ();
    bool getWakeupResult        ();
    bool checkFalseAlarm        (kaldi::Lattice& best_lat);
    bool firstPass              ();
    bool secondPass             ();
    void skip                   ();

private:
    KaldiRecognizerState                state_;

    // Model holder
    Model                               *model_;

    // For lattice analysis and output the confident score
    LatticeAnalysis                     latticeAnalysis_;

    // Classifier for decide whether wake up or not
    WakeUpDecisionMaker                 *wakeUpDecisionMaker_;

    // Decoder properties
    int32                               frame_offset_;
    int32                               vframe_offset_;
    int64                               samples_processed_;
    int64                               samples_round_start_;
    int64                               sample_frequency_;

    // Decoder
    BdiSingleUtteranceNnet3Decoder      *decoder_;
    LatticeFasterDecoder                *verify_decoder_;

    // Feature
    OnlineNnet2FeaturePipeline          *feature_pipeline_;
    OnlineSilenceWeightingBdi           *silence_weighting_;

    // Verifier properties
    int keyword_size_;
    int total_keyword_buffer_size_;
    int vinfastIndex;
};

#endif

#endif //WAKEUP_SIRI_WUW_H
