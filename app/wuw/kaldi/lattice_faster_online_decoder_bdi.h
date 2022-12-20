//
// Created by ducnd on 03/05/2021.
//
#include "../common/defs.h"
#ifdef USE_KALDI

#ifndef VOSK_ANDROID_DEMO_LATTICE_FASTER_ONLINE_DECODER_BDI_H
#define VOSK_ANDROID_DEMO_LATTICE_FASTER_ONLINE_DECODER_BDI_H

#include "util/stl-utils.h"
#include "util/hash-list.h"
#include "fst/fstlib.h"
#include "itf/decodable-itf.h"
#include "fstext/fstext-lib.h"
#include "lat/determinize-lattice-pruned.h"
#include "lat/kaldi-lattice.h"
// #include "decoder/lattice-faster-decoder.h"

#include "lattice_faster_decoder_vbdi.h"

using namespace kaldi;

template <typename FST>
class LatticeFasterOnlineDecoderBdiTpl: public LatticeFasterDecoderTpl<FST, decoder::BackpointerToken> {
public:
    using Arc = typename FST::Arc;
    using Label = typename Arc::Label;
    using StateId = typename Arc::StateId;
    using Weight = typename Arc::Weight;
    using Token = decoder::BackpointerToken;
    using ForwardLinkT = decoder::ForwardLink<Token>;

    // Instantiate this class once for each thing you have to decode.
    // This version of the constructor does not take ownership of
    // 'fst'.
    LatticeFasterOnlineDecoderBdiTpl(const FST &fst,
                                  const LatticeFasterDecoderConfig &config):
            LatticeFasterDecoderTpl<FST, Token>(fst, config) { }

    // This version of the initializer takes ownership of 'fst', and will delete
    // it when this object is destroyed.
    LatticeFasterOnlineDecoderBdiTpl(const LatticeFasterDecoderConfig &config,
                                  FST *fst):
            LatticeFasterDecoderTpl<FST, Token>(config, fst) { }


    struct BestPathIterator {
        void *tok;
        int32 frame;
        // note, "frame" is the frame-index of the frame you'll get the
        // transition-id for next time, if you call TraceBackBestPath on this
        // iterator (assuming it's not an epsilon transition).  Note that this
        // is one less than you might reasonably expect, e.g. it's -1 for
        // the nonemitting transitions before the first frame.
        BestPathIterator(void *t, int32 f): tok(t), frame(f) { }
        bool Done() const { return tok == NULL; }
    };


    /// Outputs an FST corresponding to the single best path through the lattice.
    /// This is quite efficient because it doesn't get the entire raw lattice and find
    /// the best path through it; instead, it uses the BestPathEnd and BestPathIterator
    /// so it basically traces it back through the lattice.
    /// Returns true if result is nonempty (using the return status is deprecated,
    /// it will become void).  If "use_final_probs" is true AND we reached the
    /// final-state of the graph then it will include those as final-probs, else
    /// it will treat all final-probs as one.
    bool GetBestPath(Lattice *ofst,
                     bool use_final_probs = true) const;


    /// This function does a self-test of GetBestPath().  Returns true on
    /// success; returns false and prints a warning on failure.
    bool TestGetBestPath(bool use_final_probs = true) const;


    /// This function returns an iterator that can be used to trace back
    /// the best path.  If use_final_probs == true and at least one final state
    /// survived till the end, it will use the final-probs in working out the best
    /// final Token, and will output the final cost to *final_cost (if non-NULL),
    /// else it will use only the forward likelihood, and will put zero in
    /// *final_cost (if non-NULL).
    /// Requires that NumFramesDecoded() > 0.
    BestPathIterator BestPathEnd(bool use_final_probs,
                                 BaseFloat *final_cost = NULL) const;


    /// This function can be used in conjunction with BestPathEnd() to trace back
    /// the best path one link at a time (e.g. this can be useful in endpoint
    /// detection).  By "link" we mean a link in the graph; not all links cross
    /// frame boundaries, but each time you see a nonzero ilabel you can interpret
    /// that as a frame.  The return value is the updated iterator.  It outputs
    /// the ilabel and olabel, and the (graph and acoustic) weight to the "arc" pointer,
    /// while leaving its "nextstate" variable unchanged.
    BestPathIterator TraceBackBestPath(
            BestPathIterator iter, LatticeArc *arc) const;


    /// Behaves the same as GetRawLattice but only processes tokens whose
    /// extra_cost is smaller than the best-cost plus the specified beam.
    /// It is only worthwhile to call this function if beam is less than
    /// the lattice_beam specified in the config; otherwise, it would
    /// return essentially the same thing as GetRawLattice, but more slowly.
    bool GetRawLatticePruned(Lattice *ofst,
                             bool use_final_probs,
                             BaseFloat beam) const;

    KALDI_DISALLOW_COPY_AND_ASSIGN(LatticeFasterOnlineDecoderBdiTpl);
};

typedef LatticeFasterOnlineDecoderBdiTpl<fst::StdFst> LatticeFasterOnlineDecoderBdi;

#endif //VOSK_ANDROID_DEMO_LATTICE_FASTER_ONLINE_DECODER_BDI_H

#endif