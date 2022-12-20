//
// Created by ducnd on 29/04/2021.
//

#include "../common/defs.h"
#ifdef USE_KALDI

#ifndef VOSK_ANDROID_DEMO_ONLINE_NNET3_DECODING_BDI_H
#define VOSK_ANDROID_DEMO_ONLINE_NNET3_DECODING_BDI_H

#include "online2/online-nnet3-decoding.h"
#include "decodable_online_looped_bdi.h"
#include "lattice_faster_online_decoder_bdi.h"

using namespace kaldi;

template <typename FST>
class BdiSingleUtteranceNnet3DecoderTpl {
public:

    // Constructor. The pointer 'features' is not being given to this class to own
    // and deallocate, it is owned externally.
    BdiSingleUtteranceNnet3DecoderTpl(const LatticeFasterDecoderConfig &decoder_opts,
                                   const TransitionModel &trans_model,
                                   const nnet3::DecodableNnetSimpleLoopedInfo &info,
                                   const FST &fst,
                                   OnlineNnet2FeaturePipeline *features);

    /// Initializes the decoding and sets the frame offset of the underlying
    /// decodable object. This method is called by the constructor. You can also
    /// call this method when you want to reset the decoder state, but want to
    /// keep using the same decodable object, e.g. in case of an endpoint.
    void InitDecoding(int32 frame_offset = 0);

    /// Advances the decoding as far as we can.
    void AdvanceDecoding();

    /// Finalizes the decoding. Cleans up and prunes remaining tokens, so the
    /// GetLattice() call will return faster.  You must not call this before
    /// calling (TerminateDecoding() or InputIsFinished()) and then Wait().
    void FinalizeDecoding();

    int32 NumFramesDecoded() const;

    /// Gets the lattice.  The output lattice has any acoustic scaling in it
    /// (which will typically be desirable in an online-decoding context); if you
    /// want an un-scaled lattice, scale it using ScaleLattice() with the inverse
    /// of the acoustic weight.  "end_of_utterance" will be true if you want the
    /// final-probs to be included.
    void GetLattice(bool end_of_utterance,
                    CompactLattice *clat) const;

    /// Outputs an FST corresponding to the single best path through the current
    /// lattice. If "use_final_probs" is true AND we reached the final-state of
    /// the graph then it will include those as final-probs, else it will treat
    /// all final-probs as one.
    void GetBestPath(bool end_of_utterance,
                     Lattice *best_path) const;


    /// This function calls EndpointDetected from online-endpoint.h,
    /// with the required arguments.
    bool EndpointDetected(const OnlineEndpointConfig &config);

    const LatticeFasterOnlineDecoderBdiTpl<FST> &Decoder() const { return decoder_; }

    DecodableNnetLoopedOnlineBdi* GetDecodable() {return &decodable_;}

    ~BdiSingleUtteranceNnet3DecoderTpl() { }
private:

    const LatticeFasterDecoderConfig &decoder_opts_;

    // this is remembered from the constructor; it's ultimately
    // derived from calling FrameShiftInSeconds() on the feature pipeline.
    BaseFloat input_feature_frame_shift_in_seconds_;

    // we need to keep a reference to the transition model around only because
    // it's needed by the endpointing code.
    const TransitionModel &trans_model_;

    DecodableNnetLoopedOnlineBdi decodable_;

    LatticeFasterOnlineDecoderBdiTpl<FST> decoder_;
};

typedef BdiSingleUtteranceNnet3DecoderTpl<fst::Fst<fst::StdArc> > BdiSingleUtteranceNnet3Decoder;

#endif //VOSK_ANDROID_DEMO_ONLINE_NNET3_DECODING_BDI_H

#endif