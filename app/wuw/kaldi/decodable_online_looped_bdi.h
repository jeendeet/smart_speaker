//
// Created by ducnd on 29/04/2021.
//

#include "../common/defs.h"
#ifdef USE_KALDI

#ifndef VOSK_ANDROID_DEMO_DECODABLE_ONLINE_LOOPED_BDI_H
#define VOSK_ANDROID_DEMO_DECODABLE_ONLINE_LOOPED_BDI_H

#include "nnet3/decodable-online-looped.h"

using namespace kaldi;
using namespace kaldi::nnet3;

class DecodableNnetLoopedOnlineBaseBdi: public DecodableInterface {
public:
    // Constructor.  'input_feature' is for the feature that will be given
    // as 'input' to the neural network; 'ivector_feature' is for the iVector
    // feature, or NULL if iVectors are not being used.
    DecodableNnetLoopedOnlineBaseBdi(const DecodableNnetSimpleLoopedInfo &info,
                                  OnlineFeatureInterface *input_features,
                                  OnlineFeatureInterface *ivector_features);

    // note: the LogLikelihood function is not overridden; the child
    // class needs to do this.
    //virtual BaseFloat LogLikelihood(int32 subsampled_frame, int32 index);

    // note: the frame argument is on the output of the network, i.e. after any
    // subsampling, so we call it 'subsampled_frame'.
    virtual bool IsLastFrame(int32 subsampled_frame) const;

    virtual int32 NumFramesReady() const;

    // Note: this function, present in the base-class, is overridden by the child class.
    // virtual int32 NumIndices() const;

    // this is not part of the standard Decodable interface but I think is needed for
    // something.
    int32 FrameSubsamplingFactor() const {
        return info_.opts.frame_subsampling_factor;
    }

    int32 FrameRightContext() const {
        return info_.frames_right_context;
    }

    int32 NumFeatureFrameBuffer() const {
        return buffer_log_post_.NumRows() * info_.opts.frame_subsampling_factor;
    }

    int32 PrebufferSize() {
        return info_.frames_per_chunk + info_.frames_right_context;
    }

    /// Sets the frame offset value. Frame offset is initialized to 0 when the
    /// decodable object is constructed and stays as 0 unless this method is
    /// called. This method is useful when we want to reset the decoder state,
    /// i.e. call decoder.InitDecoding(), but we want to keep using the same
    /// decodable object, e.g. in case of an endpoint. The frame offset affects
    /// the behavior of IsLastFrame(), NumFramesReady() and LogLikelihood()
    /// methods.
    void SetFrameOffset(int32 frame_offset);

    /// Returns the frame offset value.
    int32 GetFrameOffset() const { return frame_offset_; }

    virtual void SetBufferSize(int32 num_chunk);

    virtual Matrix<BaseFloat> GetBuffer();
    virtual Matrix<BaseFloat> GetLastBuffer();
    virtual Matrix<BaseFloat> GetLastFramesBeforeBuffer();

    virtual void InsertBuffer(Matrix<BaseFloat> buffer);

     virtual int32 GetWaveBufferSize() const {
         int32 result = (int32)(16000 * (buffer_log_post_.NumRows() * info_.opts.frame_subsampling_factor * 0.01 + info_.frames_right_context * 0.01 + 0.015));
         // int32 result = (int32)(16000 * (buffer_log_post_.NumRows() * info_.opts.frame_subsampling_factor * 0.01 + 0.015));
         KALDI_LOG << "[DEBUG APPEND]" << "Wave buffer size " << result;
         return result;
     }

     virtual void ResetBuffer() {
         buffer_log_post_.Resize(0, 0);
     }

protected:

    /// If the neural-network outputs for this frame are not cached, this function
    /// computes them (and possibly also some later frames).  Note:
    /// the frame-index is called 'subsampled_frame' because if frame-subsampling-factor
    /// is not 1, it's an index that is "after subsampling", i.e. it changes more
    /// slowly than the input-feature index.
    inline void EnsureFrameIsComputed(int32 subsampled_frame) {
        KALDI_ASSERT(subsampled_frame >= current_log_post_subsampled_offset_ &&
                     "Frames must be accessed in order.");
        while (subsampled_frame >= current_log_post_subsampled_offset_ +
                                   current_log_post_.NumRows())
            AdvanceChunk();
    }

    // The current log-posteriors that we got from the last time we
    // ran the computation.
    Matrix<BaseFloat> current_log_post_;

    // The number of chunks we have computed so far.
    int32 num_chunks_computed_;

    int32 num_chunk_inserted;

    // The time-offset of the current log-posteriors, equals
    // (num_chunks_computed_ - 1) *
    //    (info_.frames_per_chunk_ / info_.opts_.frame_subsampling_factor).
    int32 current_log_post_subsampled_offset_;

    const DecodableNnetSimpleLoopedInfo &info_;

    // IsLastFrame(), NumFramesReady() and LogLikelihood() methods take into
    // account this offset value. We initialize frame_offset_ as 0 and it stays as
    // 0 unless SetFrameOffset() method is called.
    int32 frame_offset_;

protected:

    // This function does the computation for the next chunk.  It will change
    // current_log_post_ and current_log_post_subsampled_offset_, and
    // increment num_chunks_computed_.
    void AdvanceChunk();

    OnlineFeatureInterface *input_features_;
    OnlineFeatureInterface *ivector_features_;

    NnetComputer computer_;

    // Max time of buffer to store
    int32 buffer_size_by_num_chunk;

    // Max time of buffer to store
    int32 buffer_size_by_num_output;

    // Num of output per chunk
    int32 num_output_per_chunk;

    // Use to store amount of latest log likelihood result
    Matrix<BaseFloat> buffer_log_post_;

    // Use to store amount of latest log likelihood result
    Matrix<BaseFloat> last_buffer_log_post_;

    // Use to store amount of latest log likelihood result
    Matrix<BaseFloat> last_frames_before_buffer_log_post;

    KALDI_DISALLOW_COPY_AND_ASSIGN(DecodableNnetLoopedOnlineBaseBdi);
};

class DecodableNnetLoopedOnlineBdi: public DecodableNnetLoopedOnlineBaseBdi {
public:
    DecodableNnetLoopedOnlineBdi(
            const TransitionModel &trans_model,
            const DecodableNnetSimpleLoopedInfo &info,
            OnlineFeatureInterface *input_features,
            OnlineFeatureInterface *ivector_features):
            DecodableNnetLoopedOnlineBaseBdi(info, input_features, ivector_features),
            trans_model_(trans_model) { }


    // returns the output-dim of the neural net.
    virtual int32 NumIndices() const { return trans_model_.NumTransitionIds(); }

    // 'subsampled_frame' is a frame, but if frame-subsampling-factor != 1, it's a
    // reduced-rate output frame (e.g. a 't' index divided by 3).
    virtual BaseFloat LogLikelihood(int32 subsampled_frame,
                                    int32 transition_id);

    virtual Matrix<BaseFloat> GetLogLikelihood(int32 from, int32 length);

public:
    const TransitionModel &trans_model_;

    KALDI_DISALLOW_COPY_AND_ASSIGN(DecodableNnetLoopedOnlineBdi);
};

#endif //VOSK_ANDROID_DEMO_DECODABLE_ONLINE_LOOPED_BDI_H

#endif