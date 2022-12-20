//
// Created by ducnd on 15/10/2020.
//
#include "../common/defs.h"
#ifdef USE_KALDI

#include "lattice_analysis.h"
#include "util/common-utils.h"
#include <cmath>
#include <sstream>

LatticeAnalysis::LatticeAnalysis() {
    this->Reset();
}

void LatticeAnalysis::Reset() {
    this->ac_average_ = 0;
    this->lm_average_ = 0;
    this->ac_wo_sil_average_ = 0;
    this->lm_wo_sil_average_ = 0;
    this->num_frame_ = 0;
    this->num_frame_wo_sil_ = 0;
    this->ac_wo_sil_std_var_ = 0;
    this->ac_std_var_ = 0;
    this->have_blank_word_ = false;
    this->num_non_sil_phones = 0;
    this->num_arc_per_phone_std_var_ = 0;
    this->num_arc_per_phone_average_ = 0;
}

void LatticeAnalysis::Analyze(kaldi::CompactLattice lat) {
    this->lat_ = kaldi::CompactLattice(lat);
    this->Reset();
    this->DoAnalysisCompactLattice();
}

//void LatticeAnalysis::Analyze(kaldi::Lattice lat) {
//    this->best_lat_ = kaldi::Lattice(lat);
//    this->Reset();
//    this->DoAnalysisLattice();
//}

void LatticeAnalysis::DoAnalysisCompactLattice() {
    // Duyet qua tat ca cac state cua best path lattice
    // int len = this->lat_.NumStates();

    for (kaldi::CompactLattice::StateId s = 0; s < this->lat_.NumStates(); s++) {

//        if (s == this->lat_.Start()) {
//            KALDI_LOG << "Start state " << s;
//            continue;
//        }
//        else if (lat_.Final(s) != kaldi::CompactLattice::Weight::Zero()) {
//            KALDI_LOG << "Final state " << s;
//            continue;
//        }

        // Duyet qua tat ca cac canh lien ket voi lattice
        for(fst::ArcIterator<kaldi::CompactLattice> iter(this->lat_, s); !iter.Done(); iter.Next()) {
            kaldi::CompactLatticeArc arc = iter.Value();
            kaldi::CompactLatticeArc::Label ilabel = arc.ilabel;
            kaldi::CompactLatticeArc::Label olabel = arc.olabel;
            kaldi::CompactLatticeArc::Weight weight = arc.weight;
            int size = weight.String().size();
            //  << arc.weight;

            // if (olabel != this->sil_id_) { // Only compute for non-sil
            if (list_sil_phone.find(olabel) == list_sil_phone.end()) { // Fix cho cac phone la sil tu 1 -> 29; sua sau
                // Accumulate value
                this->lm_wo_sil_average_ += weight.Weight().Value1();
                this->ac_wo_sil_average_ += weight.Weight().Value2();
                this->num_frame_wo_sil_ += weight.String().size();

                // Check xem word hien tai co bi blank khong
                if(weight.String().size() == 0) {
                    this->have_blank_word_ = true;
                }

                this->num_non_sil_phones += 1;

                // KALDI_LOG << "ANALYSIS: " << ilabel << " " << olabel << " " << weight.Weight().Value1() << " " << weight.Weight().Value2() << " " << weight.String().size();
            }
            // Accumulate value
            this->lm_average_ += weight.Weight().Value1();
            this->ac_average_ += weight.Weight().Value2();
            this->num_frame_ += weight.String().size();
        }
    }
    // Compute average
    this->lm_average_ = this->lm_average_ / this->num_frame_;
    this->ac_average_ = this->ac_average_ / this->num_frame_;
    this->lm_wo_sil_average_ = this->lm_wo_sil_average_ / this->num_frame_wo_sil_;
    this->ac_wo_sil_average_ = this->ac_wo_sil_average_ / this->num_frame_wo_sil_;
    this->num_arc_per_phone_average_ = (float) this->num_frame_wo_sil_ / this->num_non_sil_phones;

    // Compute standard variance
    for (kaldi::CompactLattice::StateId s = 0; s < this->lat_.NumStates(); s++) {
//        if (s == this->lat_.Start()) {
//            KALDI_LOG << "Start state " << s;
//            continue;
//        }
//        else if (lat_.Final(s) != kaldi::CompactLattice::Weight::Zero()) {
//            KALDI_LOG << "Final state " << s;
//            continue;
//        }

        // Duyet qua tat ca cac canh lien ket voi lattice
        for(fst::ArcIterator<kaldi::CompactLattice> iter(this->lat_, s); !iter.Done(); iter.Next()) {
            kaldi::CompactLatticeArc arc = iter.Value();
            kaldi::CompactLatticeArc::Label ilabel = arc.ilabel;
            kaldi::CompactLatticeArc::Label olabel = arc.olabel;
            kaldi::CompactLatticeArc::Weight weight = arc.weight;
            std::string type = arc.Type();

            // KALDI_LOG << "Olabel " << olabel << " Size " << weight.String().size();

            // if (olabel != this->sil_id_) { // Only compute for non-sil
            if (list_sil_phone.find(olabel) == list_sil_phone.end()) { // Fix cho cac phone la sil tu 1 -> 29; sua sau
                // Accumulate value
                if(weight.String().size() > 0) {
                    float tmp = (weight.Weight().Value2() / weight.String().size()) - this->ac_wo_sil_average_;
                    this->ac_wo_sil_std_var_ += tmp * tmp * weight.String().size();

                    float tmp2 = weight.String().size() - this->num_arc_per_phone_average_;
                    this->num_arc_per_phone_std_var_ += tmp2 * tmp2;
                }
            }
        }
    }

    this->ac_wo_sil_std_var_ = sqrt(this->ac_wo_sil_std_var_ / this->num_frame_wo_sil_);
    this->num_arc_per_phone_std_var_ = sqrt(this->num_arc_per_phone_std_var_ / this->num_non_sil_phones);

    // Output log
//    KALDI_LOG  // << "LM = " << this->lm_average_ << " AC = " << this->ac_average_
//              << " LM w/o SIL = " << this->lm_wo_sil_average_ << " AC w/o SIL = " << this->ac_wo_sil_average_ << " AC  w/o SIL STDVAR = " << this->ac_wo_sil_std_var_\
//              << " NUM ARC PER PHONE = " << this->num_arc_per_phone_average_ << " NUM ARC PER PHONE STD = " << this->num_arc_per_phone_std_var_
//              << " BLANK WORD: " << (this->have_blank_word_ ? "YES" : "NO");
}

//void LatticeAnalysis::DoAnalysisLattice() {
//    // Duyet qua tat ca cac state cua best path lattice
//    int len = this->best_lat_.NumStates();
//    for (kaldi::Lattice::StateId s = 0; s < this->best_lat_.NumStates(); s++) {
//        // Duyet qua tat ca cac canh lien ket voi lattice
//        for(fst::ArcIterator<kaldi::Lattice> iter(this->best_lat_, s); !iter.Done(); iter.Next()) {
//            kaldi::LatticeArc arc = iter.Value();
//            kaldi::LatticeArc::Label ilabel = arc.ilabel;
//            kaldi::LatticeArc::Label olabel = arc.olabel;
//            kaldi::LatticeArc::Weight weight = arc.weight;
//            std::string type = arc.Type();
//
//            if (olabel != this->sil_id_) { // Only compute for non-sil
//                // Accumulate value
//                this->lm_wo_sil_average_ += weight.Value1();
//                this->ac_wo_sil_average_ += weight.Value2();
//                // this->num_frame_wo_sil_ += weight.String().size();
//
//                // Check xem word hien tai co bi blank khong
//                // if(weight.String().size() == 0) {
//                //     this->have_blank_word_ = true;
//                // }
//            }
//            // Accumulate value
//            this->lm_average_ += weight.Value1();
//            this->ac_average_ += weight.Value2();
//            // this->num_frame_ += weight.String().size();
//            this->num_frame_ += 1;
//
//            KALDI_LOG << ilabel << " " << olabel << " " << weight.Value1() << " " << weight.Value2();
//        }
//    }
//    // Compute average
//    this->lm_average_ = this->lm_average_ / this->num_frame_;
//    this->ac_average_ = this->ac_average_ / this->num_frame_;
//    this->lm_wo_sil_average_ = this->lm_wo_sil_average_ / this->num_frame_wo_sil_;
//    this->ac_wo_sil_average_ = this->ac_wo_sil_average_ / this->num_frame_wo_sil_;
//
//    // Compute standard variance
//    for (kaldi::Lattice::StateId s = 0; s < this->best_lat_.NumStates(); s++) {
//        // Duyet qua tat ca cac canh lien ket voi lattice
//        for(fst::ArcIterator<kaldi::Lattice> iter(this->best_lat_, s); !iter.Done(); iter.Next()) {
//            kaldi::LatticeArc arc = iter.Value();
//            kaldi::LatticeArc::Label ilabel = arc.ilabel;
//            kaldi::LatticeArc::Label olabel = arc.olabel;
//            kaldi::LatticeArc::Weight weight = arc.weight;
//
//            if (olabel != this->sil_id_) { // Only compute for non-sil
//                // Accumulate value
////                if(weight.String().size() > 0) {
////                    float tmp = (weight.Weight().Value2() / weight.String().size()) - this->ac_wo_sil_average_;
////                    this->ac_wo_sil_std_var_ += tmp * tmp * weight.String().size();
////                }
//            }
//        }
//    }
//
//    this->ac_wo_sil_std_var_ = sqrt(this->ac_wo_sil_std_var_ / this->num_frame_wo_sil_);
//
//    // Output log
//    KALDI_LOG << "LM = " << this->lm_average_ << " AC = " << this->ac_average_ \
//              << " LM w/o SIL = " << this->lm_wo_sil_average_ << " AC w/o SIL = " << this->ac_wo_sil_average_ << " AC  w/o SIL STDVAR = " << this->ac_wo_sil_std_var_\
//              << " BLANK WORD: " << (this->have_blank_word_ ? "YES" : "NO");
//}

std::string LatticeAnalysis::ToString() {
    std::string result = std::to_string(this->num_arc_per_phone_average_) + ", " \
       + std::to_string(this->num_arc_per_phone_std_var_) + ", " \
       + std::to_string(this->lm_wo_sil_average_) + ", " \
       + std::to_string(this->ac_wo_sil_average_) + ", " \
       + std::to_string(this->ac_wo_sil_std_var_) + ", " \
       + std::to_string(this->have_blank_word_) + ", " \
       + std::to_string(this->start_time_) + ", " \
       + std::to_string(this->end_time_) + ", " \
       + this->text_;

    return result;
}

const char* LatticeAnalysis::ToCString() {
    std::string result = std::to_string(this->num_arc_per_phone_average_) + ", " \
       + std::to_string(this->num_arc_per_phone_std_var_) + ", " \
       + std::to_string(this->lm_wo_sil_average_) + ", " \
       + std::to_string(this->ac_wo_sil_average_) + ", " \
       + std::to_string(this->ac_wo_sil_std_var_) + ", " \
       + std::to_string(this->have_blank_word_) + ", " \
       + std::to_string(this->start_time_) + ", " \
       + std::to_string(this->end_time_) + ", " \
       + this->text_;

    return result.c_str();
}

void LatticeAnalysis::ReadListSilPhone(std::string list_phone) {
    // Chuoi input co dang a:b:c:...:e:f
    // Can doc a, b, c ... va push vao list sil phone
    std::string delimiter = ":";
    size_t pos = 0;
    std::string token;

    while((pos = list_phone.find(delimiter)) != std::string::npos) {
        token = list_phone.substr(0, pos);
        KALDI_LOG << "SIL PHONE " << token;
        list_sil_phone.insert(std::stoi(token));
        list_phone.erase(0, pos + delimiter.length());
    }
    KALDI_LOG << "SIL PHONE " << list_phone;
    try {
        list_sil_phone.insert(std::stoi(list_phone));
    }
    catch (...) {
        KALDI_LOG << "NO SIL PHONE";
    }

}

#endif