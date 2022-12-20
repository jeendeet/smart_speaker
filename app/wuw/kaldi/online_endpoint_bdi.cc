//
// Created by ducnd on 03/05/2021.
//

#include "../common/defs.h"
#ifdef USE_KALDI

#include "online_endpoint_bdi.h"
#include "lattice_faster_online_decoder_bdi.h"

using namespace kaldi;

template <typename DEC>
int32 TrailingSilenceLengthBdi(const TransitionModel &tmodel,
                            const std::string &silence_phones_str,
                            const DEC &decoder) {
    std::vector<int32> silence_phones;
    if (!SplitStringToIntegers(silence_phones_str, ":", false, &silence_phones))
        KALDI_ERR << "Bad --silence-phones option in endpointing config: "
                  << silence_phones_str;
    std::sort(silence_phones.begin(), silence_phones.end());
    KALDI_ASSERT(IsSortedAndUniq(silence_phones) &&
                 "Duplicates in --silence-phones option in endpointing config");
    KALDI_ASSERT(!silence_phones.empty() &&
                 "Endpointing requires nonempty --endpoint.silence-phones option");
    ConstIntegerSet<int32> silence_set(silence_phones);

    bool use_final_probs = false;
    typename DEC::BestPathIterator iter =
            decoder.BestPathEnd(use_final_probs, NULL);
    int32 num_silence_frames = 0;
    while (!iter.Done()) {  // we're going backwards in time from the most
        // recently decoded frame...
        LatticeArc arc;
        iter = decoder.TraceBackBestPath(iter, &arc);
        if (arc.ilabel != 0) {
            int32 phone = tmodel.TransitionIdToPhone(arc.ilabel);
            if (silence_set.count(phone) != 0) {
                num_silence_frames++;
            } else {
                break; // stop counting as soon as we hit non-silence.
            }
        }
    }
    return num_silence_frames;
}

template <typename DEC>
bool EndpointDetectedBdi(
        const OnlineEndpointConfig &config,
        const TransitionModel &tmodel,
        BaseFloat frame_shift_in_seconds,
        const DEC &decoder) {
    if (decoder.NumFramesDecoded() == 0) return false;

    BaseFloat final_relative_cost = decoder.FinalRelativeCost();

    int32 num_frames_decoded = decoder.NumFramesDecoded(),
            trailing_silence_frames = TrailingSilenceLengthBdi(tmodel,
                                                            config.silence_phones,
                                                            decoder);

    return EndpointDetected(config, num_frames_decoded, trailing_silence_frames,
                            frame_shift_in_seconds, final_relative_cost);
}


// Instantiate EndpointDetected for the types we need.
// It will require TrailingSilenceLength so we don't have to instantiate that.
template
bool EndpointDetectedBdi<LatticeFasterOnlineDecoderTpl<fst::Fst<fst::StdArc> > >(
const OnlineEndpointConfig &config,
const TransitionModel &tmodel,
        BaseFloat frame_shift_in_seconds,
const LatticeFasterOnlineDecoderTpl<fst::Fst<fst::StdArc> > &decoder);


template
bool EndpointDetectedBdi<LatticeFasterOnlineDecoderTpl<fst::ConstGrammarFst > >(
const OnlineEndpointConfig &config,
const TransitionModel &tmodel,
        BaseFloat frame_shift_in_seconds,
const LatticeFasterOnlineDecoderTpl<fst::ConstGrammarFst > &decoder);


template
bool EndpointDetectedBdi<LatticeFasterOnlineDecoderTpl<fst::VectorGrammarFst > >(
const OnlineEndpointConfig &config,
const TransitionModel &tmodel,
        BaseFloat frame_shift_in_seconds,
const LatticeFasterOnlineDecoderTpl<fst::VectorGrammarFst > &decoder);


template
bool EndpointDetectedBdi<LatticeIncrementalOnlineDecoderTpl<fst::Fst<fst::StdArc> > >(
const OnlineEndpointConfig &config,
const TransitionModel &tmodel,
        BaseFloat frame_shift_in_seconds,
const LatticeIncrementalOnlineDecoderTpl<fst::Fst<fst::StdArc> > &decoder);

template
bool EndpointDetectedBdi<LatticeIncrementalOnlineDecoderTpl<fst::ConstGrammarFst > >(
const OnlineEndpointConfig &config,
const TransitionModel &tmodel,
        BaseFloat frame_shift_in_seconds,
const LatticeIncrementalOnlineDecoderTpl<fst::ConstGrammarFst > &decoder);


template
bool EndpointDetectedBdi<LatticeIncrementalOnlineDecoderTpl<fst::VectorGrammarFst > >(
const OnlineEndpointConfig &config,
const TransitionModel &tmodel,
        BaseFloat frame_shift_in_seconds,
const LatticeIncrementalOnlineDecoderTpl<fst::VectorGrammarFst > &decoder);

template
bool EndpointDetectedBdi<LatticeFasterOnlineDecoderBdiTpl<fst::Fst<fst::StdArc> > >(
const OnlineEndpointConfig &config,
const TransitionModel &tmodel,
        BaseFloat frame_shift_in_seconds,
const LatticeFasterOnlineDecoderBdiTpl<fst::Fst<fst::StdArc> > &decoder);

template
bool EndpointDetectedBdi<LatticeFasterOnlineDecoderBdiTpl<fst::ConstGrammarFst > >(
        const OnlineEndpointConfig &config,
        const TransitionModel &tmodel,
        BaseFloat frame_shift_in_seconds,
        const LatticeFasterOnlineDecoderBdiTpl<fst::ConstGrammarFst > &decoder);


template
bool EndpointDetectedBdi<LatticeFasterOnlineDecoderBdiTpl<fst::VectorGrammarFst > >(
        const OnlineEndpointConfig &config,
        const TransitionModel &tmodel,
        BaseFloat frame_shift_in_seconds,
        const LatticeFasterOnlineDecoderBdiTpl<fst::VectorGrammarFst > &decoder);

#endif
