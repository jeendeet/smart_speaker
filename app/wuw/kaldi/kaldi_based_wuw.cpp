//
// Created by ducnd on 15/10/2021.
//
#include "../common/defs.h"
#ifdef USE_KALDI

#include <utility>
#include <vector>
#include <cmath>

#include "../common/log.h"
#include "kaldi_based_wuw.h"
#include "decoder/decodable-matrix.h"

KaldiBasedWuW::KaldiBasedWuW() {
    sample_frequency_ = 16000;
}

KaldiBasedWuW::~KaldiBasedWuW() {

}

bool KaldiBasedWuW::acceptWaveform(std::tuple<short*, size_t>& feature) {
    auto sdata = std::get<0>(feature);
    auto len = std::get<1>(feature);

    Vector<BaseFloat> wave;
    wave.Resize(len / 2, kUndefined);
    for (auto i = 0; i < len; i++) {
        wave(i) = sdata[i];
    }

    return acceptWaveform(wave);
}

void KaldiBasedWuW::setModel(Model *model) {
    this->model_ = model;
}

bool KaldiBasedWuW::acceptWaveform(Vector<BaseFloat> &wdata) {
    if (!(state_ == RECOGNIZER_RUNNING || state_ == RECOGNIZER_INITIALIZED))
    {
        clean();
    }
    state_ = RECOGNIZER_RUNNING;

    feature_pipeline_->AcceptWaveform(sample_frequency_, wdata);

    updateSilenceWeights();

    decoder_->AdvanceDecoding();
    samples_processed_ += wdata.Dim();

    return getWakeupResult();
}

bool KaldiBasedWuW::getWakeupResult() {
    if (firstPass()) {
        if (secondPass()) {
            return true;
        }
    }

    return false;
}

bool KaldiBasedWuW::firstPass() {
    if (decoder_->NumFramesDecoded() ==0) {
        return false;
    }

    bool result = false;
    kaldi::CompactLattice analyze_clat;
    kaldi::Lattice best_lat;

    // Get 1 best path
    decoder_->GetBestPath(true, &best_lat);
    ConvertLatticeToPhones(*model_->trans_model_, &best_lat);
    ConvertLattice(best_lat, &analyze_clat);

    fst::ScaleLattice(fst::GraphLatticeScale(1 / model_->am_weight), &analyze_clat);
    this->latticeAnalysis_.Analyze(analyze_clat);

    result = this->wakeUpDecisionMaker_->WakeUp(this->latticeAnalysis_.NumArcPerFrameAvg(),
                                                this->latticeAnalysis_.NumArcPerFrameStdVar(),
                                                this->latticeAnalysis_.AcNoSilAvg(),
                                                this->latticeAnalysis_.AcNoSilStdVar());
    return result;
}

bool KaldiBasedWuW::secondPass() {
    auto vinfastIndex = -1;
    bool result = false;

    auto data = decoder_->GetDecodable()->GetBuffer();
    if (data.NumRows() > 0) {
        verify_decoder_ = new LatticeFasterDecoder (*model_->vhclg_fst_, model_->nnet3_decoding_config_);
        verify_decoder_->InitDecoding();

        DecodableMatrixMapped decodable(*model_->vtrans_model_, data, 0);

        verify_decoder_->AdvanceDecoding(&decodable);
        vframe_offset_ += data.NumRows();

        kaldi::Lattice best_lat_check;
        verify_decoder_->GetBestPath(&best_lat_check, false);

        result = checkFalseAlarm(best_lat_check);

        delete verify_decoder_;
    }

    return result;
}

void KaldiBasedWuW::clean()
{
    delete silence_weighting_;
    silence_weighting_ = new OnlineSilenceWeightingBdi(*model_->trans_model_, model_->feature_info_.silence_weighting_config, 3);

    if (decoder_)
        frame_offset_ += decoder_->NumFramesDecoded();

    // Also restart if we retrieved final result already
    if (decoder_ == NULL || state_ == RECOGNIZER_FINALIZED || frame_offset_ > 20000) {
        samples_round_start_ += samples_processed_;
        samples_processed_ = 0;
        frame_offset_ = 0;

        delete decoder_;
        delete feature_pipeline_;

        feature_pipeline_ = new kaldi::OnlineNnet2FeaturePipeline (model_->feature_info_);
        decoder_ = new BdiSingleUtteranceNnet3Decoder(model_->nnet3_decoding_config_,
                                                      *model_->trans_model_,
                                                      *model_->decodable_info_,
                                                      *model_->hclg_fst_,
                                                      feature_pipeline_);

        if (model_->recognizer_config_.false_alarm_check) {
            decoder_->GetDecodable()->SetBufferSize(model_->recognizer_config_.num_chunk_buffer);
        }
        else {
            verify_decoder_ = NULL;
        }
    }
    else
    {
        decoder_->InitDecoding(frame_offset_);
    }
}

void KaldiBasedWuW::updateSilenceWeights()
{
    if (silence_weighting_->Active() && feature_pipeline_->NumFramesReady() > 0 &&
        feature_pipeline_->IvectorFeature() != NULL)
    {
        vector<pair<int32, BaseFloat>> delta_weights;
        silence_weighting_->ComputeCurrentTraceback(decoder_->Decoder());
        silence_weighting_->GetDeltaWeights(feature_pipeline_->NumFramesReady(),
                                            frame_offset_ * 3,
                                            &delta_weights);
        feature_pipeline_->UpdateFrameWeights(delta_weights);
    }
}

bool KaldiBasedWuW::checkFalseAlarm(Lattice &best_lat) {
    fst::ScaleLattice(fst::GraphLatticeScale(0), &best_lat);

    kaldi::Lattice phone_best_lat(best_lat);
    ConvertLatticeToPhones(*model_->getTransitionModel(), &phone_best_lat);

    std::vector<int> words;
    int num_sil_head = 0;
    int num_sil_tail = 0;
    bool add_sil_head = true;
    bool is_sil = false;

    // identify vinfast position
    bool is_checking_vinfast = true;
    bool is_checking_vphoneme = true;
    this->keyword_size_ = 0;
    this->total_keyword_buffer_size_ = 0;
    // check if false alarm
    kaldi::Lattice::StateId s = best_lat.Start();
    kaldi::Lattice::StateId sPhone = phone_best_lat.Start();
    do {
        for(fst::ArcIterator<kaldi::Lattice> iter(best_lat, s), iterPhone(phone_best_lat, s); !iter.Done(); iter.Next(), iterPhone.Next()) {
            kaldi::LatticeArc arc = iter.Value();
            kaldi::LatticeArc::Label ilabel = arc.ilabel;
            kaldi::LatticeArc::Label olabel = arc.olabel;

            kaldi::LatticeArc arcPhone = iterPhone.Value();
            kaldi::LatticeArc::Label ilabelPhone = arcPhone.ilabel;
            kaldi::LatticeArc::Label olabelPhone = arcPhone.olabel;

            KALDI_LOG << "ILABEL " << ilabel;
            KALDI_LOG << "OLABEL " << olabel;

            // increase size of lattice
            this->total_keyword_buffer_size_ ++;

            // Check for vinfast position
            if (is_checking_vinfast) {
                if (words.size() < 1) {
                    if (ilabel != 0) {
                        this->keyword_size_ ++;
                    }
                }
                else if (words.size() == 1) {
                    if (ilabel != 0) {
                        this->keyword_size_ ++;
                    }
                    else {
                        is_checking_vinfast = false;
                    }
                }
            }

// Check for t phoneme position
//            if (is_checking_vinfast && ilabel != 0) {
//                this->keyword_size_ ++;
//            }

            // KALDI_LOG << "IN " << ilabel << " OUTPUT " << olabel;

            if (olabel == 1) {
                is_sil = true;
            }

            if (olabel >= 5) {   // 0: eps, 1: SIL
                is_sil = false;
                words.push_back(olabel);
                add_sil_head = false;

                // reset num of sil at tail
                num_sil_tail = 0;

// Check for t phoneme position
//                if (model_->vWordSyms()->Find(olabel) == "vinfast") {
//                    is_checking_vinfast = false;
//                }
            }

            // Count from begin until second word is pushed
            if (is_checking_vphoneme) {
                if (olabelPhone != model_->recognizer_config_.cutoff_phone) {
                    vinfastIndex++;
                }
                else {
                    is_checking_vphoneme = false;
                }
            }

            if (is_sil){
                if (add_sil_head) {
                    num_sil_head += 1;
                }
                else {
                    num_sil_tail += 1;
                    // Does not count sil at tail
                }
            }

            s = arc.nextstate;
            break;
        }
    }
    while(best_lat.Final(s) == kaldi::Lattice::Weight::Zero());

    // Reverse counter
    vinfastIndex = total_keyword_buffer_size_ - vinfastIndex;

    KALDI_LOG << "VINFAST INDEX " << vinfastIndex;
    KALDI_LOG << "TOTAL KEYWORD COUNTER " << total_keyword_buffer_size_;

//    for (int word : words) {
//        KALDI_VLOG(1) << "WORD " << model_->vWordSyms()->Find(word);
//    }

    if (words.size() == 2) {
        if ((model_->vword_syms_->Find(words[0]) == "hey") &&
            (model_->vword_syms_->Find(words[1]) == "vinfast")) {

//            KALDI_LOG << "Num sil tail " << num_sil_tail;
//            KALDI_LOG << "KW length " << this->keyword_size_;
            return true;
        }
    }

//    KALDI_LOG << "Num sil tail " << num_sil_tail;
//    if (words.size() == 2) {
//        if ((model_->vWordSyms()->Find(words[0]) == "hey") &&
//            (model_->vWordSyms()->Find(words[1]) == "vinfast" && num_sil_tail > 0)) {
//            return true;
//        }
//    }

    return false;
}

void KaldiBasedWuW::skip() {
    state_ = RECOGNIZER_FINALIZED;
    decoder_->GetDecodable()->ResetBuffer();
}

#endif