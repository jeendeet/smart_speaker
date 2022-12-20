//
// Created by ducnd on 03/05/2021.
//

#include "../common/defs.h"
#ifdef USE_KALDI

#include "online_ivector_feature_bdi.h"

OnlineSilenceWeightingBdi::OnlineSilenceWeightingBdi(
        const TransitionModel &trans_model,
        const OnlineSilenceWeightingConfig &config,
        int32 frame_subsampling_factor):
        trans_model_(trans_model), config_(config),
        frame_subsampling_factor_(frame_subsampling_factor),
        num_frames_output_and_correct_(0) {
    KALDI_ASSERT(frame_subsampling_factor_ >= 1);
    std::vector<int32> silence_phones;
    SplitStringToIntegers(config.silence_phones_str, ":,", false,
                          &silence_phones);
    for (size_t i = 0; i < silence_phones.size(); i++)
        silence_phones_.insert(silence_phones[i]);
}


template <typename FST>
void OnlineSilenceWeightingBdi::ComputeCurrentTraceback(
        const LatticeFasterOnlineDecoderBdiTpl<FST> &decoder,
        bool use_final_probs) {
    int32 num_frames_decoded = decoder.NumFramesDecoded(),
            num_frames_prev = frame_info_.size();
    // note, num_frames_prev is not the number of frames previously decoded,
    // it's the generally-larger number of frames that we were requested to
    // provide weights for.
    if (num_frames_prev < num_frames_decoded)
        frame_info_.resize(num_frames_decoded);
    if (num_frames_prev > num_frames_decoded &&
        frame_info_[num_frames_decoded].transition_id != -1)
        KALDI_ERR << "Number of frames decoded decreased";  // Likely bug

    if (num_frames_decoded == 0)
        return;
    int32 frame = num_frames_decoded - 1;
    typename LatticeFasterOnlineDecoderBdiTpl<FST>::BestPathIterator iter =
            decoder.BestPathEnd(use_final_probs, NULL);
    while (frame >= 0) {
        LatticeArc arc;
        arc.ilabel = 0;
        while (arc.ilabel == 0)  // the while loop skips over input-epsilons
            iter = decoder.TraceBackBestPath(iter, &arc);
        // note, the iter.frame values are slightly unintuitively defined,
        // they are one less than you might expect.
        KALDI_ASSERT(iter.frame == frame - 1);

        if (frame_info_[frame].token == iter.tok) {
            // we know that the traceback from this point back will be identical, so
            // no point tracing back further.  Note: we are comparing memory addresses
            // of tokens of the decoder; this guarantees it's the same exact token
            // because tokens, once allocated on a frame, are only deleted, never
            // reallocated for that frame.
            break;
        }

        if (num_frames_output_and_correct_ > frame)
            num_frames_output_and_correct_ = frame;

        frame_info_[frame].token = iter.tok;
        frame_info_[frame].transition_id = arc.ilabel;
        frame--;
        // leave frame_info_.current_weight at zero for now (as set in the
        // constructor), reflecting that we haven't already output a weight for that
        // frame.
    }
}

template <typename FST>
void OnlineSilenceWeightingBdi::ComputeCurrentTraceback(
        const LatticeIncrementalOnlineDecoderTpl<FST> &decoder,
        bool use_final_probs) {
    int32 num_frames_decoded = decoder.NumFramesDecoded(),
            num_frames_prev = frame_info_.size();
    // note, num_frames_prev is not the number of frames previously decoded,
    // it's the generally-larger number of frames that we were requested to
    // provide weights for.
    if (num_frames_prev < num_frames_decoded)
        frame_info_.resize(num_frames_decoded);
    if (num_frames_prev > num_frames_decoded &&
        frame_info_[num_frames_decoded].transition_id != -1)
        KALDI_ERR << "Number of frames decoded decreased";  // Likely bug

    if (num_frames_decoded == 0)
        return;
    int32 frame = num_frames_decoded - 1;
    typename LatticeIncrementalOnlineDecoderTpl<FST>::BestPathIterator iter =
            decoder.BestPathEnd(use_final_probs, NULL);
    while (frame >= 0) {
        LatticeArc arc;
        arc.ilabel = 0;
        while (arc.ilabel == 0)  // the while loop skips over input-epsilons
            iter = decoder.TraceBackBestPath(iter, &arc);
        // note, the iter.frame values are slightly unintuitively defined,
        // they are one less than you might expect.
        KALDI_ASSERT(iter.frame == frame - 1);

        if (frame_info_[frame].token == iter.tok) {
            // we know that the traceback from this point back will be identical, so
            // no point tracing back further.  Note: we are comparing memory addresses
            // of tokens of the decoder; this guarantees it's the same exact token,
            // because tokens, once allocated on a frame, are only deleted, never
            // reallocated for that frame.
            break;
        }

        if (num_frames_output_and_correct_ > frame)
            num_frames_output_and_correct_ = frame;

        frame_info_[frame].token = iter.tok;
        frame_info_[frame].transition_id = arc.ilabel;
        frame--;
        // leave frame_info_.current_weight at zero for now (as set in the
        // constructor), reflecting that we haven't already output a weight for that
        // frame.
    }
}


// Instantiate the template OnlineSilenceWeightingBdi::ComputeCurrentTraceback().
template
void OnlineSilenceWeightingBdi::ComputeCurrentTraceback<fst::Fst<fst::StdArc> >(
const LatticeFasterOnlineDecoderBdiTpl<fst::Fst<fst::StdArc> > &decoder,
bool use_final_probs);
template
void OnlineSilenceWeightingBdi::ComputeCurrentTraceback<fst::ConstGrammarFst >(
        const LatticeFasterOnlineDecoderBdiTpl<fst::ConstGrammarFst > &decoder,
        bool use_final_probs);
template
void OnlineSilenceWeightingBdi::ComputeCurrentTraceback<fst::VectorGrammarFst >(
        const LatticeFasterOnlineDecoderBdiTpl<fst::VectorGrammarFst > &decoder,
        bool use_final_probs);

template
void OnlineSilenceWeightingBdi::ComputeCurrentTraceback<fst::Fst<fst::StdArc> >(
const LatticeIncrementalOnlineDecoderTpl<fst::Fst<fst::StdArc> > &decoder,
bool use_final_probs);
template
void OnlineSilenceWeightingBdi::ComputeCurrentTraceback<fst::ConstGrammarFst >(
        const LatticeIncrementalOnlineDecoderTpl<fst::ConstGrammarFst > &decoder,
        bool use_final_probs);
template
void OnlineSilenceWeightingBdi::ComputeCurrentTraceback<fst::VectorGrammarFst >(
        const LatticeIncrementalOnlineDecoderTpl<fst::VectorGrammarFst > &decoder,
        bool use_final_probs);


void OnlineSilenceWeightingBdi::GetDeltaWeights(
        int32 num_frames_ready, int32 first_decoder_frame,
        std::vector<std::pair<int32, BaseFloat> > *delta_weights) {
    // num_frames_ready is at the feature frame-rate, most of the code
    // in this function is at the decoder frame-rate.
    // round up, so we are sure to get weights for at least the frame
    // 'num_frames_ready - 1', and maybe one or two frames afterward.
    KALDI_ASSERT(num_frames_ready > first_decoder_frame || num_frames_ready == 0);
    int32 fs = frame_subsampling_factor_,
            num_decoder_frames_ready = (num_frames_ready - first_decoder_frame + fs - 1) / fs;

    const int32 max_state_duration = config_.max_state_duration;
    const BaseFloat silence_weight = config_.silence_weight;

    delta_weights->clear();

    int32 prev_num_frames_processed = frame_info_.size();
    if (frame_info_.size() < static_cast<size_t>(num_decoder_frames_ready))
        frame_info_.resize(num_decoder_frames_ready);

    // Don't go further backward into the past then 100 frames before the most
    // recent frame previously than 100 frames when modifying the traceback.
    // C.f. the value 200 in template
    // OnlineGenericBaseFeature<C>::OnlineGenericBaseFeature in online-feature.cc,
    // which needs to be more than this value of 100 plus the amount of context
    // that LDA might use plus the chunk size we're likely to decode in one time.
    // The user can always increase the value of --max-feature-vectors in case one
    // of these conditions is broken.  Search for ONLINE_IVECTOR_LIMIT in
    // online-feature.cc
    int32 begin_frame = std::max<int32>(0, prev_num_frames_processed - 100),
            frames_out = static_cast<int32>(frame_info_.size()) - begin_frame;
    // frames_out is the number of frames we will output.
    KALDI_ASSERT(frames_out >= 0);
    std::vector<BaseFloat> frame_weight(frames_out, 1.0);
    // we will set frame_weight to the value silence_weight for silence frames and
    // for transition-ids that repeat with duration > max_state_duration.  Frames
    // newer than the most recent traceback will get a weight equal to the weight
    // for the most recent frame in the traceback; or the silence weight, if there
    // is no traceback at all available yet.

    // First treat some special cases.
    if (frames_out == 0)  // Nothing to output.
        return;
    if (frame_info_[begin_frame].transition_id == -1) {
        // We do not have any traceback at all within the frames we are to output...
        // find the most recent weight that we output and apply the same weight to
        // all the new output; or output the silence weight, if nothing was output.
        BaseFloat weight = (begin_frame == 0 ? silence_weight :
                            frame_info_[begin_frame - 1].current_weight);
        for (int32 offset = 0; offset < frames_out; offset++)
            frame_weight[offset] = weight;
    } else {
        int32 current_run_start_offset = 0;
        for (int32 offset = 0; offset < frames_out; offset++) {
            int32 frame = begin_frame + offset;
            int32 transition_id = frame_info_[frame].transition_id;
            if (transition_id == -1) {
                // this frame does not yet have a decoder traceback, so just
                // duplicate the silence/non-silence status of the most recent
                // frame we have a traceback for (probably a reasonable guess).
                frame_weight[offset] = frame_weight[offset - 1];
            } else {
                int32 phone = trans_model_.TransitionIdToPhone(transition_id);
                bool is_silence = (silence_phones_.count(phone) != 0);
                if (is_silence)
                    frame_weight[offset] = silence_weight;
                // now deal with max-duration issues.
                if (max_state_duration > 0 &&
                    (offset + 1 == frames_out ||
                     transition_id != frame_info_[frame + 1].transition_id)) {
                    // If this is the last frame of a run...
                    int32 run_length = offset - current_run_start_offset + 1;
                    if (run_length >= max_state_duration) {
                        // treat runs of the same transition-id longer than the max, as
                        // silence, even if they were not silence.
                        for (int32 offset2 = current_run_start_offset;
                             offset2 <= offset; offset2++)
                            frame_weight[offset2] = silence_weight;
                    }
                    if (offset + 1 < frames_out)
                        current_run_start_offset = offset + 1;
                }
            }
        }
    }
    // Now commit the stats...
    for (int32 offset = 0; offset < frames_out; offset++) {
        int32 frame = begin_frame + offset;
        BaseFloat old_weight = frame_info_[frame].current_weight,
                new_weight = frame_weight[offset],
                weight_diff = new_weight - old_weight;
        frame_info_[frame].current_weight = new_weight;
        // Even if the delta-weight is zero for the last frame, we provide it,
        // because the identity of the most recent frame with a weight is used in
        // some debugging/checking code.
        if (weight_diff != 0.0 || offset + 1 == frames_out) {
            KALDI_VLOG(6) << "Weight for frame " << frame << " changing from "
                          << old_weight << " to " << new_weight;
            for(int32 i = 0; i < frame_subsampling_factor_; i++) {
                int32 input_frame = first_decoder_frame + (frame * frame_subsampling_factor_) + i;
                delta_weights->push_back(std::make_pair(input_frame, weight_diff));
            }
        }
    }
}

void OnlineSilenceWeightingBdi::GetNonsilenceFrames(
        int32 num_frames_ready, int32 first_decoder_frame,
        std::vector<int32> *frames) {
    // num_frames_ready is at the feature frame-rate, most of the code
    // in this function is at the decoder frame-rate.
    // round up, so we are sure to get weights for at least the frame
    // 'num_frames_ready - 1', and maybe one or two frames afterward.
    KALDI_ASSERT(num_frames_ready > first_decoder_frame || num_frames_ready == 0);
    int32 fs = frame_subsampling_factor_,
            num_decoder_frames_ready = (num_frames_ready - first_decoder_frame + fs - 1) / fs;

    frames->clear();

    int32 prev_num_frames_processed = frame_info_.size();
    if (frame_info_.size() < static_cast<size_t>(num_decoder_frames_ready))
        frame_info_.resize(num_decoder_frames_ready);

    // Don't go further backward into the past then 500 frames before the most
    // recent frame
    int32 begin_frame = std::max<int32>(0, prev_num_frames_processed - 500),
            frames_out = static_cast<int32>(frame_info_.size()) - begin_frame;
    // frames_out is the number of frames we will output.
    KALDI_ASSERT(frames_out >= 0);

    for (int32 offset = 0; offset < frames_out; offset++) {
        int32 frame = begin_frame + offset;
        int32 transition_id = frame_info_[frame].transition_id;
        if (transition_id != -1) {
            int32 phone = trans_model_.TransitionIdToPhone(transition_id);
            bool is_silence = (silence_phones_.count(phone) != 0);
            if (!is_silence) {
                frames->push_back(frame);
            }
        }
    }
}

#endif