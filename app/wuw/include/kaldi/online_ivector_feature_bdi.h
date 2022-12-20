//
// Created by ducnd on 03/05/2021.
//

#include "../common/defs.h"
#ifdef USE_KALDI

#ifndef VOSK_ANDROID_DEMO_ONLINE_IVECTOR_FEATURE_BDI_H
#define VOSK_ANDROID_DEMO_ONLINE_IVECTOR_FEATURE_BDI_H

#include <string>
#include <vector>
#include <deque>

#include "matrix/matrix-lib.h"
#include "util/common-utils.h"
#include "base/kaldi-error.h"
#include "itf/online-feature-itf.h"
#include "gmm/diag-gmm.h"
#include "feat/online-feature.h"
#include "ivector/ivector-extractor.h"
#include "decoder/lattice-faster-online-decoder.h"
#include "decoder/lattice-incremental-online-decoder.h"
#include "lattice_faster_online_decoder_bdi.h"
#include "online2/online-ivector-feature.h"

class OnlineSilenceWeightingBdi {
public:
    // Note: you would initialize a new copy of this object for each new
    // utterance.
    // The frame-subsampling-factor is used for newer nnet3 models, especially
    // chain models, when the frame-rate of the decoder is different from the
    // frame-rate of the input features.  E.g. you might set it to 3 for such
    // models.

    OnlineSilenceWeightingBdi(const TransitionModel &trans_model,
                           const OnlineSilenceWeightingConfig &config,
                           int32 frame_subsampling_factor = 1);

    bool Active() const { return config_.Active(); }

    // This should be called before GetDeltaWeights, so this class knows about the
    // traceback info from the decoder.  It records the traceback information from
    // the decoder using its BestPathEnd() and related functions.
    // It will be instantiated for FST == fst::Fst<fst::StdArc> and fst::GrammarFst.
    template <typename FST>
    void ComputeCurrentTraceback(const LatticeFasterOnlineDecoderBdiTpl<FST> &decoder,
                                 bool use_final_probs = false);
    template <typename FST>
    void ComputeCurrentTraceback(const LatticeIncrementalOnlineDecoderTpl<FST> &decoder,
                                 bool use_final_probs = false);

    // Calling this function gets the changes in weight that require us to modify
    // the stats... the output format is (frame-index, delta-weight).
    //
    // The num_frames_ready argument is the number of frames available at
    // the input (or equivalently, output) of the online iVector feature in the
    // feature pipeline from the stream start. It may be more than the currently
    // available decoder traceback.
    //
    // The first_decoder_frame is the offset from the start of the stream in
    // pipeline frames when decoder was restarted last time. We do not change
    // weight for the frames earlier than first_decoder_frame. Set it to 0 in
    // case of compilation error to reproduce the previous behavior or for a
    // single utterance decoding.
    //
    // How many frames of weights it outputs depends on how much "num_frames_ready"
    // increased since last time we called this function, and whether the decoder
    // traceback changed.  Negative delta_weights might occur if frames previously
    // classified as non-silence become classified as silence if the decoder's
    // traceback changes.  You must call this function with "num_frames_ready"
    // arguments that only increase, not decrease, with time.  You would provide
    // this output to class OnlineIvectorFeature by calling its function
    // UpdateFrameWeights with the output.
    //
    // Returned frame-index is in pipeline frames from the pipeline start.
    void GetDeltaWeights(
            int32 num_frames_ready, int32 first_decoder_frame,
            std::vector<std::pair<int32, BaseFloat> > *delta_weights);

    // A method for backward compatibility, same as above, but for a single
    // utterance.
    void GetDeltaWeights(
            int32 num_frames_ready,
            std::vector<std::pair<int32, BaseFloat> > *delta_weights) {
        GetDeltaWeights(num_frames_ready, 0, delta_weights);
    }

    // Gets a list of nonsilence frames collected on traceback. Useful
    // for algorithms to extract speaker properties like speaker identification
    // vectors.
    void GetNonsilenceFrames(
            int32 num_frames_ready, int32 first_decoder_frame,
            std::vector<int32> *frames);

private:
    const TransitionModel &trans_model_;
    const OnlineSilenceWeightingConfig &config_;

    int32 frame_subsampling_factor_;

    unordered_set<int32> silence_phones_;

    struct FrameInfo {
        // The only reason we need the token pointer is to know far back we have to
        // trace before the traceback is the same as what we previously traced back.
        void *token;
        int32 transition_id;
        // current_weight is the weight we've previously told the iVector
        // extractor to use for this frame, if any.  It may not equal the
        // weight we "want" it to use (any difference between the two will
        // be output when the user calls GetDeltaWeights().
        BaseFloat current_weight;
        FrameInfo(): token(NULL), transition_id(-1), current_weight(0.0) {}
    };

    // This contains information about any previously computed traceback;
    // when the traceback changes we use this variable to compare it with the
    // previous traceback.
    // It's indexed at the frame-rate of the decoder (may be different
    // by 'frame_subsampling_factor_' from the frame-rate of the features.
    std::vector<FrameInfo> frame_info_;

    // This records how many frames have been output and that currently reflect
    // the traceback accurately.  It is used to avoid GetDeltaWeights() having to
    // visit each frame as far back as t = 0, each time it is called.
    // GetDeltaWeights() sets this to the number of frames that it output, and
    // ComputeCurrentTraceback() then reduces it to however far it traced back.
    // However, we may have to go further back in time than this in order to
    // properly honor the "max-state-duration" config.  This, if needed, is done
    // in GetDeltaWeights() before outputting the delta weights.
    int32 num_frames_output_and_correct_;
};

#endif //VOSK_ANDROID_DEMO_ONLINE_IVECTOR_FEATURE_BDI_H

#endif
