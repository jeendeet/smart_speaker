//
// Created by ducnd on 29/04/2021.
//
#include "../common/defs.h"
#ifdef USE_KALDI

#include "decodable_online_looped_bdi.h"

static int32 last_subsampled_frame = -1;

DecodableNnetLoopedOnlineBaseBdi::DecodableNnetLoopedOnlineBaseBdi(
        const DecodableNnetSimpleLoopedInfo &info,
        OnlineFeatureInterface *input_features,
        OnlineFeatureInterface *ivector_features):
        num_chunks_computed_(0),
        current_log_post_subsampled_offset_(-1),
        info_(info),
        frame_offset_(0),
        input_features_(input_features),
        ivector_features_(ivector_features),
        num_chunk_inserted(0),
        buffer_size_by_num_chunk(0),
        buffer_size_by_num_output(0),
        num_output_per_chunk(0),
        computer_(info_.opts.compute_config,
                  info_.computation,
                  info_.nnet, NULL) {   // NULL is 'nnet_to_update'
    // Check that feature dimensions match.
    KALDI_ASSERT(input_features_ != NULL);
    int32 nnet_input_dim = info_.nnet.InputDim("input"),
            nnet_ivector_dim = info_.nnet.InputDim("ivector"),
            feat_input_dim = input_features_->Dim(),
            feat_ivector_dim = (ivector_features_ != NULL ?
                                ivector_features_->Dim() : -1);
    if (nnet_input_dim != feat_input_dim) {
        KALDI_ERR << "Input feature dimension mismatch: got " << feat_input_dim
                  << " but network expects " << nnet_input_dim;
    }
    if (nnet_ivector_dim != feat_ivector_dim) {
        KALDI_ERR << "Ivector feature dimension mismatch: got " << feat_ivector_dim
                  << " but network expects " << nnet_ivector_dim;
    }

    KALDI_LOG << "LEFT CONTEXT " << info_.frames_left_context;
    KALDI_LOG << "RIGHT CONTEXT " << info_.frames_right_context;
}


int32 DecodableNnetLoopedOnlineBaseBdi::NumFramesReady() const {
    // note: the ivector_features_ may have 2 or 3 fewer frames ready than
    // input_features_, but we don't wait for them; we just use the most recent
    // iVector we can.
    int32 features_ready = input_features_->NumFramesReady();
    if (features_ready == 0)
        return 0;
    bool input_finished = input_features_->IsLastFrame(features_ready - 1);

    int32 sf = info_.opts.frame_subsampling_factor;

    int32 result;
    if (input_finished) {
        // if the input has finished,... we'll pad with duplicates of the last frame
        // as needed to get the required right context.
        result = (features_ready + sf - 1) / sf - frame_offset_;
    } else {
        // note: info_.right_context_ includes both the model context and any
        // extra_right_context_ (but this
        int32 non_subsampled_output_frames_ready =
                std::max<int32>(0, features_ready - info_.frames_right_context);
        int32 num_chunks_ready = non_subsampled_output_frames_ready /
                                 info_.frames_per_chunk;
        // note: the division by the frame subsampling factor 'sf' below
        // doesn't need any attention to rounding because info_.frames_per_chunk
        // is always a multiple of 'sf' (see 'frames_per_chunk = GetChunksize..."
        // in decodable-simple-looped.cc).
        result = num_chunks_ready * info_.frames_per_chunk / sf - frame_offset_;
    }

    // result += GetExtraBufferFrame();

    return result;
}


// note: the frame-index argument is on the output of the network, i.e. after any
// subsampling, so we call it 'subsampled_frame'.
bool DecodableNnetLoopedOnlineBaseBdi::IsLastFrame(
        int32 subsampled_frame) const {
    // To understand this code, compare it with the code of NumFramesReady(),
    // it follows the same structure.
    int32 features_ready = input_features_->NumFramesReady();
    if (features_ready == 0) {
        if (subsampled_frame == -1 && input_features_->IsLastFrame(-1)) {
            // the attempt to handle this rather pathological case (input finished
            // but no frames ready) is a little quixotic as we have not properly
            // tested this and other parts of the code may die.
            return true;
        } else {
            return false;
        }
    }
    bool input_finished = input_features_->IsLastFrame(features_ready - 1);
    if (!input_finished)
        return false;
    int32 sf = info_.opts.frame_subsampling_factor,
            num_subsampled_frames_ready = (features_ready + sf - 1) / sf;
    return (subsampled_frame + frame_offset_ == num_subsampled_frames_ready - 1);
}

void DecodableNnetLoopedOnlineBaseBdi::SetFrameOffset(int32 frame_offset) {
    KALDI_ASSERT(0 <= frame_offset &&
                 frame_offset <= frame_offset_ + NumFramesReady());
    frame_offset_ = frame_offset;
}

void DecodableNnetLoopedOnlineBaseBdi::SetBufferSize(int32 num_chunk) {
    int32 frames_per_chunk = this->info_.opts.frames_per_chunk;
    int32 frame_subsampling_factor = this->info_.opts.frame_subsampling_factor;

    KALDI_LOG << "frames_per_chunk " << frames_per_chunk;
    KALDI_LOG << "frame_subsampling_factor " << frame_subsampling_factor;
    this->buffer_size_by_num_chunk = num_chunk;
    this->num_output_per_chunk = info_.frames_per_chunk / info_.opts.frame_subsampling_factor;
    this->buffer_size_by_num_output = this->buffer_size_by_num_chunk * this->num_output_per_chunk;

    KALDI_LOG << "buffer_size " << buffer_size_by_num_output;
}

Matrix<BaseFloat> DecodableNnetLoopedOnlineBaseBdi::GetBuffer() {
    KALDI_LOG << "Get GetBuffer size " << this->buffer_log_post_.NumRows();
    return this->buffer_log_post_;
}

Matrix<BaseFloat> DecodableNnetLoopedOnlineBaseBdi::GetLastBuffer() {
    KALDI_LOG << "Get DecodableNnetLoopedOnlineBaseBdi size " << this->last_buffer_log_post_.NumRows();
    return this->last_buffer_log_post_;
}

Matrix<BaseFloat> DecodableNnetLoopedOnlineBaseBdi::GetLastFramesBeforeBuffer() {
    KALDI_LOG << "Get GetLastFramesBeforeBuffer size " << this->last_frames_before_buffer_log_post.NumRows();
    return this->last_frames_before_buffer_log_post;
}

void DecodableNnetLoopedOnlineBaseBdi::InsertBuffer(Matrix<BaseFloat> buffer) {
    KALDI_LOG << "Insert buffer with size " << buffer.NumRows();
    KALDI_LOG << "Old log post size " << this->current_log_post_.NumRows();
    // KALDI_ASSERT(buffer.NumRows() % this->num_output_per_chunk == 0);

    int num_chunk_inserted = buffer.NumRows() / this->num_output_per_chunk;

    if (num_chunk_inserted == 0) {
        return;
    }

//    // compute new size
//    int32 new_buffer_row = buffer.NumRows() + current_log_post_.NumRows();
//
//    Matrix<BaseFloat> tmp;
//    // init tmp
//    tmp.Resize(new_buffer_row, buffer_log_post_.NumCols(), kUndefined);
//    // copy tail of buffer to head of tmp
//    tmp.Range(0, current_log_post_.NumRows(), 0, buffer_log_post_.NumCols()).CopyFromMat(current_log_post_);
//    // copy new log post to tail of tmp
//    tmp.Range(current_log_post_.NumRows(), buffer.NumRows(), 0, buffer_log_post_.NumCols()).CopyFromMat(buffer);
//
//    // Swap to current_log_post_
//    current_log_post_.Resize(new_buffer_row, buffer_log_post_.NumCols(), kUndefined);
//    current_log_post_.Swap(&tmp);

    // Xoa di log post cu
    current_log_post_.Resize(0, 0);
    current_log_post_.Swap(&buffer);

    KALDI_LOG << "New log post size " << this->current_log_post_.NumRows();

    this->num_chunks_computed_ += num_chunk_inserted;

    this->num_chunk_inserted = num_chunk_inserted;

    current_log_post_subsampled_offset_ =
            (num_chunks_computed_ - num_chunk_inserted) *
            (info_.frames_per_chunk / info_.opts.frame_subsampling_factor);
}

void DecodableNnetLoopedOnlineBaseBdi::AdvanceChunk() {
    // Prepare the input data for the next chunk of features.
    // note: 'end' means one past the last.
    int32 begin_input_frame, end_input_frame;
    if (num_chunks_computed_ == 0) {
        begin_input_frame = -info_.frames_left_context;
        // note: end is last plus one.
        end_input_frame = info_.frames_per_chunk + info_.frames_right_context;
    } else {
        // note: begin_input_frame will be the same as the previous end_input_frame.
        // you can verify this directly if num_chunks_computed_ == 0, and then by
        // induction.
        begin_input_frame = num_chunks_computed_ * info_.frames_per_chunk +
                            info_.frames_right_context;
        end_input_frame = begin_input_frame + info_.frames_per_chunk;
    }

    int32 num_feature_frames_ready = input_features_->NumFramesReady();

//    KALDI_LOG << "[DEBUG APPEND]" << "begin_input_frame " << begin_input_frame;
//    KALDI_LOG << "[DEBUG APPEND]" << "end_input_frame " << end_input_frame;
//    KALDI_LOG << "[DEBUG APPEND]" << "num_feature_frames_ready " << num_feature_frames_ready;

    bool is_finished = input_features_->IsLastFrame(num_feature_frames_ready - 1);

    if (end_input_frame > num_feature_frames_ready && !is_finished) {
        // we shouldn't be attempting to read past the end of the available features
        // until we have reached the end of the input (i.e. the end-user called
        // InputFinished(), announcing that there is no more waveform; at this point
        // we pad as needed with copies of the last frame, to flush out the last of
        // the output.
        // If the following error happens, it likely indicates a bug in this
        // decodable code somewhere (although it could possibly indicate the
        // user asking for a frame that was not ready, which would be a misuse
        // of this class.. it can be figured out from gdb as in either case it
        // would be a bug in the code.
        KALDI_ERR << "Attempt to access frame past the end of the available input";
    }

    CuMatrix<BaseFloat> feats_chunk;
    { // this block sets 'feats_chunk'.
        Matrix<BaseFloat> this_feats(end_input_frame - begin_input_frame,
                                     input_features_->Dim());
        for (int32 i = begin_input_frame; i < end_input_frame; i++) {
            SubVector<BaseFloat> this_row(this_feats, i - begin_input_frame);
            int32 input_frame = i;
            if (input_frame < 0) input_frame = 0;
            if (input_frame >= num_feature_frames_ready)
                input_frame = num_feature_frames_ready - 1;
            input_features_->GetFrame(input_frame, &this_row);
        }
        feats_chunk.Swap(&this_feats);
    }
    computer_.AcceptInput("input", &feats_chunk);

    uint64_t checkpoint1 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    if (info_.has_ivectors) {
        KALDI_ASSERT(ivector_features_ != NULL);
        KALDI_ASSERT(info_.request1.inputs.size() == 2);
        // all but the 1st chunk should have 1 iVector, but there is no need to
        // assume this.
        int32 num_ivectors = (num_chunks_computed_ == 0 ?
                              info_.request1.inputs[1].indexes.size() :
                              info_.request2.inputs[1].indexes.size());
        KALDI_ASSERT(num_ivectors > 0);

        Vector<BaseFloat> ivector(ivector_features_->Dim());
        // we just get the iVector from the last input frame we needed,
        // reduced as necessary
        // we don't bother trying to be 'accurate' in getting the iVectors
        // for their 'correct' frames, because in general using the
        // iVector from as large 't' as possible will be better.

        int32 most_recent_input_frame = num_feature_frames_ready - 1,
                num_ivector_frames_ready = ivector_features_->NumFramesReady();

        if (num_ivector_frames_ready > 0) {
            int32 ivector_frame_to_use = std::min<int32>(
                    most_recent_input_frame, num_ivector_frames_ready - 1);
            ivector_features_->GetFrame(ivector_frame_to_use,
                                        &ivector);
        }
        // else just leave the iVector zero (would only happen with very small
        // chunk-size, like a chunk size of 2 which would be very inefficient; and
        // only at file begin.

        // note: we expect num_ivectors to be 1 in practice.
        Matrix<BaseFloat> ivectors(num_ivectors,
                                   ivector.Dim());
        ivectors.CopyRowsFromVec(ivector);
        CuMatrix<BaseFloat> cu_ivectors;
        cu_ivectors.Swap(&ivectors);
        computer_.AcceptInput("ivector", &cu_ivectors);
    }
    uint64_t checkpoint2 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    computer_.Run();
    uint64_t checkpoint3 = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    if (info_.has_ivectors) {
        KALDI_VLOG(1) << "[BENCHMARK]" << " Compute ivector time " << checkpoint2 - checkpoint1;
    }
    KALDI_VLOG(1) << "[BENCHMARK]" << " NNet compute " << checkpoint3 - checkpoint2;

    {
        // Note: it's possible in theory that if you had weird recurrence that went
        // directly from the output, the call to GetOutputDestructive() would cause
        // a crash on the next chunk.  If that happens, GetOutput() should be used
        // instead of GetOutputDestructive().  But we don't anticipate this will
        // happen in practice.
        CuMatrix<BaseFloat> output;
        computer_.GetOutputDestructive("output", &output);

        if (info_.log_priors.Dim() != 0) {
            // subtract log-prior (divide by prior)
            output.AddVecToRows(-1.0, info_.log_priors);
        }
        // apply the acoustic scale
        output.Scale(info_.opts.acoustic_scale);
        current_log_post_.Resize(0, 0);
        current_log_post_.Swap(&output);

//        KALDI_LOG << "[DEBUG APPEND]" << "current_log_post_'s rows " << current_log_post_.NumRows();

        // APPEND TO BUFFER
//        KALDI_LOG << "CURRENT ROW " << current_log_post_.NumRows();
//        KALDI_LOG << "CURRENT COL " << current_log_post_.NumCols();
//        KALDI_LOG << "BUFFER ROW " << buffer_log_post_.NumRows();
//        KALDI_LOG << "BUFFER COL " << buffer_log_post_.NumCols();

        if (this->buffer_size_by_num_output > 0) {
            if (buffer_log_post_.NumRows() > 0) {
                // compute new size
                int32 new_buffer_row = buffer_log_post_.NumRows() + current_log_post_.NumRows();
                new_buffer_row = new_buffer_row > buffer_size_by_num_output ? buffer_size_by_num_output : new_buffer_row;
                int32 buffer_offset = buffer_log_post_.NumRows() + current_log_post_.NumRows() - new_buffer_row;

                Matrix<BaseFloat> tmp;
                // init tmp
                tmp.Resize(new_buffer_row, buffer_log_post_.NumCols(), kUndefined);
                // copy tail of buffer to head of tmp
                tmp.Range(0, buffer_log_post_.NumRows() - buffer_offset, 0, buffer_log_post_.NumCols())
                        .CopyFromMat(buffer_log_post_.Range(buffer_offset, buffer_log_post_.NumRows() - buffer_offset, 0, buffer_log_post_.NumCols()));
                // copy new log post to tail of tmp
                tmp.Range(buffer_log_post_.NumRows() - buffer_offset, current_log_post_.NumRows(), 0, buffer_log_post_.NumCols())
                        .CopyFromMat(current_log_post_);

                if (buffer_offset > 0) {
                    last_frames_before_buffer_log_post.Resize(0, 0);
                    last_frames_before_buffer_log_post.Resize(buffer_offset, current_log_post_.NumCols());
                    last_frames_before_buffer_log_post.CopyFromMat(buffer_log_post_.Range(0, buffer_offset, 0, buffer_log_post_.NumCols()));
                }
                else {
                    last_frames_before_buffer_log_post.Resize(0, 0);
                }

                buffer_log_post_.Swap(&tmp);
            }
            else {
                buffer_log_post_.Resize(current_log_post_.NumRows(), current_log_post_.NumCols());
                buffer_log_post_.CopyFromMat(current_log_post_);
            }
        }
        last_buffer_log_post_.Resize(0, 0);
        last_buffer_log_post_.Resize(current_log_post_.NumRows(), current_log_post_.NumCols());
        last_buffer_log_post_.CopyFromMat(current_log_post_);

//        KALDI_LOG << "BUFFER ROW AFTER " << buffer_log_post_.NumRows();
//        KALDI_LOG << "BUFFER COL AFTER " << buffer_log_post_.NumCols();

    }

    KALDI_ASSERT(current_log_post_.NumRows() == info_.frames_per_chunk /
                                                info_.opts.frame_subsampling_factor &&
                 current_log_post_.NumCols() == info_.output_dim);

    num_chunks_computed_++;

    current_log_post_subsampled_offset_ =
            (num_chunks_computed_ - 1) *
            (info_.frames_per_chunk / info_.opts.frame_subsampling_factor);
}

BaseFloat DecodableNnetLoopedOnlineBdi::LogLikelihood(int32 subsampled_frame,
                                                      int32 index) {
    if (subsampled_frame != last_subsampled_frame) {
        last_subsampled_frame = subsampled_frame;
//        KALDI_LOG << "[DEBUG APPEND]" << "LogLikelihood [subsampled_frame, index] = [" << subsampled_frame << "], with offset [" << subsampled_frame + frame_offset_ << "]";
    }

    subsampled_frame += frame_offset_;
    EnsureFrameIsComputed(subsampled_frame);
    return current_log_post_(
            subsampled_frame - current_log_post_subsampled_offset_,
            trans_model_.TransitionIdToPdfFast(index));
}

Matrix<BaseFloat> DecodableNnetLoopedOnlineBdi::GetLogLikelihood(int32 from, int32 length) {

}

#endif
