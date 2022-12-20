// Copyright 2019 Alpha Cephei Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//       http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef VOSK_MODEL_H
#define VOSK_MODEL_H

#include "../common/defs.h"
#ifdef USE_KALDI
#include <base/kaldi-common.h>
#include <fstext/fstext-lib.h>
#include <fstext/fstext-utils.h>
#include <online2/onlinebin-util.h>
#include <online2/online-timing.h>
#include <online2/online-endpoint.h>
#include <online2/online-nnet3-decoding.h>
#include <online2/online-feature-pipeline.h>
#include <lat/lattice-functions.h>
#include <lat/sausages.h>
#include <lat/word-align-lattice.h>
#include <lm/const-arpa-lm.h>
#include <util/parse-options.h>
#include <nnet3/nnet-utils.h>
#include <rnnlm/rnnlm-utils.h>
#include "../common/cipher.h"
#include "../common/exception.h"

using namespace kaldi;
using namespace std;

class KaldiRecognizer;

struct RecognizerConfig {
    bool            do_cipher;
    float           wakeup_threshold;
    int             feature_type;
    bool            use_pitch;
    bool            false_alarm_check;
    int             num_chunk_buffer;
    int             cutoff_phone;

    RecognizerConfig():
    do_cipher                   (true),
    wakeup_threshold            (0.9),
    feature_type                (0),
    use_pitch                   (true),
    false_alarm_check           (false),
    num_chunk_buffer            (0),
    cutoff_phone                (510){
    }

    void Register(OptionsItf *opts) {
        opts->Register("do-cipher", &do_cipher, "do_cipher");
        opts->Register("wakeup-threshold", &wakeup_threshold, "wakeup_threshold");
        opts->Register("use-pitch", &use_pitch, "use_pitch");
        opts->Register("feature-type", &feature_type, "feature_type");
        opts->Register("false-alarm-check", &false_alarm_check, "false_alarm_check");
        opts->Register("num-chunk-buffer", &num_chunk_buffer, "num_chunk_buffer");
        opts->Register("cutoff-phone", &cutoff_phone, "cutoff_phone");
    }

    void Check() const {

    }
};


class Model {

public:
    Model(const char *model_path) throw(IOException);
    ~Model();

    void                        Ref();
    void                        Unref();
    int                         FindWord(const char *word);

    void                        ReadSilID();
    int                         GetSilID();
    kaldi::TransitionModel*     getTransitionModel()    {return this->trans_model_;}
    std::string                 getListSilPhone()       {return this->feature_info_.silence_weighting_config.silence_phones_str;}

protected:
    void                        Configure()           throw(IOException);
    void                        ReadDataFiles()         throw(IOException);

    string                      model_path_str_;
    string                      conf_path_str_;

    /* COMMON PART*/
    string                      nnet3_rxfilename_;
    string                      global_cmvn_stats_rxfilename_;
    string                      final_ie_rxfilename_;
    string                      vad_rxfilename_;

    kaldi::TransitionModel      *trans_model_;
    kaldi::TransitionModel      *vtrans_model_;
    kaldi::nnet3::AmNnetSimple  *nnet_;

    AESDecoder                  aesDecoder;

    RecognizerConfig            recognizer_config_;

    int                         ref_cnt_;

    std::map<int, string>       mapping_;

    /* WAKEUP PART */
    string                      hclg_fst_rxfilename_;
    string                      vhclg_fst_rxfilename_;
    string                      disambig_rxfilename_;
    string                      word_syms_rxfilename_;
    string                      vword_syms_rxfilename_;
    string                      winfo_rxfilename_;
    string                      mfcc_conf_rxfilename_;
    string                      silence_conf_rxfilename_;

    kaldi::OnlineNnet2FeaturePipelineInfo               feature_info_;
    kaldi::OnlineEndpointConfig                         endpoint_config_;
    kaldi::LatticeFasterDecoderConfig                   nnet3_decoding_config_;
    kaldi::nnet3::NnetSimpleLoopedComputationOptions    decodable_opts_;
    kaldi::nnet3::DecodableNnetSimpleLoopedInfo         *decodable_info_;

    const fst::SymbolTable      *word_syms_;
    const fst::SymbolTable      *vword_syms_;
    kaldi::WordBoundaryInfo     *winfo_;
    vector<int32>               disambig_;

    fst::Fst<fst::StdArc>       *hclg_fst_;
    fst::Fst<fst::StdArc>       *vhclg_fst_;
    fst::Fst<fst::StdArc>       *hcl_fst_;
    fst::Fst<fst::StdArc>       *g_fst_;

    int                         sil_id_;

    fst::VectorFst<fst::StdArc> *std_lm_fst_;
    kaldi::ConstArpaLm          const_arpa_;

    float                       am_weight;

    int                         num_chunk_buffer;

    friend class KaldiBasedWuW;
};
#endif

#endif /* VOSK_MODEL_H */
