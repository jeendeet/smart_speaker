//
// Created by ducnd on 15/10/2020.
//

#include "../common/defs.h"
#ifdef USE_KALDI

#ifndef C_LATTICE_ANALYSIS_H
#define C_LATTICE_ANALYSIS_H

#include "lat/kaldi-lattice.h"
#include <string>
#include <set>

class LatticeAnalysis {
public:
    LatticeAnalysis();

    void Reset();
    void Analyze(kaldi::CompactLattice lat);
    //void Analyze(kaldi::Lattice lat);

    float LmAvg(){return lm_average_;}
    float AcAvg(){return ac_average_;}
    float LmNoSilAvg(){return lm_wo_sil_average_;}
    float AcNoSilAvg(){return ac_wo_sil_average_;}
    float AcNoSilStdVar(){return ac_wo_sil_std_var_;}
    float NumArcPerFrameAvg(){return num_arc_per_phone_average_;}
    float NumArcPerFrameStdVar(){return num_arc_per_phone_std_var_;}
    int NumFrames(){return num_frame_;}
    int NumFramesNoSil(){return num_frame_wo_sil_;}
    bool HaveBlankWord(){return have_blank_word_;}
    float StartTime(){return start_time_;}
    float EndTime(){return end_time_;}
    std::string Text(){return text_;}
    void SetStartTime(float start_time){start_time_ = start_time;}
    void SetEndTime(float end_time){end_time_ = end_time;}
    void SetText(std::string text){text_ = text;}
    std::string ToString();
    const char* ToCString();
    void SetSilID(int sil_id){sil_id_ = sil_id;}
    void ReadListSilPhone(std::string list_phone);

private:
    void DoAnalysisCompactLattice();
    // void DoAnalysisLattice();
    int sil_id_;

    kaldi::CompactLattice lat_;
    //kaldi::Lattice best_lat_;
    std::set<int> list_sil_phone;
    float lm_average_;
    float ac_average_;
    float ac_std_var_;
    float lm_wo_sil_average_;
    float ac_wo_sil_average_;
    float ac_wo_sil_std_var_;
    float num_arc_per_phone_average_;
    float num_arc_per_phone_std_var_;
    int num_frame_;
    int num_frame_wo_sil_;
    bool have_blank_word_;
    float start_time_;
    float end_time_;
    std::string text_;
    int num_non_sil_phones;
};

#endif //C_LATTICE_ANALYSIS_H

#endif