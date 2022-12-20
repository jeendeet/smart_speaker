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


//
// For details of possible model layout see doc/models.md section model-structure

#include "../common/defs.h"
#ifdef USE_KALDI
#include "model.h"

#include <sys/stat.h>
#include <fst/fst.h>
#include <fst/register.h>
#include <fst/matcher-fst.h>
#include <fst/extensions/ngram/ngram-fst.h>
#include <csignal>
#include <dirent.h>
#include <csignal>
#include "../common/config_reader.h"

namespace fst {

static FstRegisterer<StdOLabelLookAheadFst> OLabelLookAheadFst_StdArc_registerer;
static FstRegisterer<NGramFst<StdArc>> NGramFst_StdArc_registerer;

}  // namespace fst

#ifdef __ANDROID__
#include <android/log.h>
static void KaldiLogHandler(const LogMessageEnvelope &env, const char *message)
{
  int priority;
  if (env.severity > GetVerboseLevel())
      return;

  if (env.severity > LogMessageEnvelope::kInfo) {
    priority = ANDROID_LOG_VERBOSE;
  } else {
    switch (env.severity) {
    case LogMessageEnvelope::kInfo:
      priority = ANDROID_LOG_INFO;
      break;
    case LogMessageEnvelope::kWarning:
      priority = ANDROID_LOG_WARN;
      break;
    case LogMessageEnvelope::kAssertFailed:
      priority = ANDROID_LOG_FATAL;
      break;
    case LogMessageEnvelope::kError:
    default: // If not the ERROR, it still an error!
      priority = ANDROID_LOG_ERROR;
      break;
    }
  }

  std::stringstream full_message;
  full_message << env.func << "():" << env.file << ':'
               << env.line << ") " << message;

  __android_log_print(priority, "VoskAPI", "%s", full_message.str().c_str());
}
#else
static void KaldiLogHandler(const LogMessageEnvelope &env, const char *message)
{
  if (env.severity > GetVerboseLevel())
      return;

  // Modified default Kaldi logging so we can disable LOG messages.
  std::stringstream full_message;
  if (env.severity > LogMessageEnvelope::kInfo) {
    full_message << "VLOG[" << env.severity << "] (";
  } else {
    switch (env.severity) {
    case LogMessageEnvelope::kInfo:
      full_message << "LOG (";
      break;
    case LogMessageEnvelope::kWarning:
      full_message << "WARNING (";
      break;
    case LogMessageEnvelope::kAssertFailed:
      full_message << "ASSERTION_FAILED (";
      break;
    case LogMessageEnvelope::kError:
    default: // If not the ERROR, it still an error!
      full_message << "ERROR (";
      break;
    }
  }
  // Add other info from the envelope and the message text.
  full_message << "VoskAPI" << ':'
               << env.func << "():" << env.file << ':'
               << env.line << ") " << message;

  // Print the complete message to stderr.
  full_message << "\n";
  std::cerr << full_message.str();
}
#endif

int str_ends_with(const char * str, const char * suffix) {

  if( str == NULL || suffix == NULL )
    return 0;

  size_t str_len = strlen(str);
  size_t suffix_len = strlen(suffix);

  if(suffix_len > str_len)
    return 0;

  return 0 == strncmp( str + str_len - suffix_len, suffix, suffix_len );
}

Model::Model(const char *model_path) throw(IOException) : model_path_str_(model_path) {
    signal(SIGABRT, 0);

    this->am_weight = 1.2;

    this->num_chunk_buffer = 0;

    conf_path_str_ = model_path_str_ + "/wakeup";

    SetLogHandler(KaldiLogHandler);

    Configure();

    ReadDataFiles();

    ref_cnt_ = 1;
}

void Model::Configure() throw(IOException)
{
    /* COMMON PART */
    // Net
    nnet3_rxfilename_ = model_path_str_ + "/am/final.mdl";
    global_cmvn_stats_rxfilename_ = model_path_str_ + "/am/global_cmvn.stats";
    // i-vector
    final_ie_rxfilename_ = model_path_str_ + "/ivector/final.ie";
    vad_rxfilename_ = model_path_str_ + "/vad/vad.conf";

    /* WAKEUP PART */
    // Graph
    hclg_fst_rxfilename_ = conf_path_str_ + "/graph/HCLG.fst";
    vhclg_fst_rxfilename_ = conf_path_str_ + "/graph/vHCLG.fst";
    disambig_rxfilename_ = conf_path_str_ + "/graph/disambig_tid.int";
    word_syms_rxfilename_ = conf_path_str_ + "/graph/words.txt";
    vword_syms_rxfilename_ = conf_path_str_ + "/graph/vwords.txt";
    winfo_rxfilename_ = conf_path_str_ + "/graph/phones/word_boundary.int";
    // config
    mfcc_conf_rxfilename_ = conf_path_str_ + "/conf/mfcc.conf";
    silence_conf_rxfilename_ = conf_path_str_ + "/conf/silence.conf";

    // Check file
    struct stat buffer;
    if (stat((model_path_str_ + "/conf/recognizer.conf").c_str(), &buffer) != 0) throw IOException(model_path_str_ + "/conf/recognizer.conf is not found");
    ::ParseOptions po1("something1");

    recognizer_config_.Register(&po1);
    po1.ReadConfigFile(model_path_str_ + "/conf/recognizer.conf");
    KALDI_LOG << "do-cipher: " << recognizer_config_.do_cipher;
    KALDI_LOG << "wakeup threshold: " << recognizer_config_.wakeup_threshold;

    if (stat((conf_path_str_ + "/conf/model.conf").c_str(), &buffer) != 0) throw IOException(conf_path_str_ + "/conf/model.conf is not found");


    if (recognizer_config_.do_cipher) {
        AESParseOptions p1("something");
        nnet3_decoding_config_.Register(&p1);
        endpoint_config_.Register(&p1);
        decodable_opts_.Register(&p1);
        p1.ReadConfigFile(conf_path_str_ + "/conf/model.conf");
    }
    else {
        ::ParseOptions p1("something");
        nnet3_decoding_config_.Register(&p1);
        endpoint_config_.Register(&p1);
        decodable_opts_.Register(&p1);
        p1.ReadConfigFile(conf_path_str_ + "/conf/model.conf");
    }
}

void Model::ReadDataFiles() throw(IOException)
{
    struct stat buffer;

    try {

        if (recognizer_config_.feature_type == 0) { // MFCC
            feature_info_.feature_type = "mfcc";
            if (recognizer_config_.do_cipher) {
                ReadConfigFromAESFile(mfcc_conf_rxfilename_, &feature_info_.mfcc_opts);
            }
            else {
                ::ReadConfigFromFile(mfcc_conf_rxfilename_, &feature_info_.mfcc_opts);
            }
        }
        else if (recognizer_config_.feature_type == 1) { // FBANK
            feature_info_.feature_type = "fbank";
            if (recognizer_config_.use_pitch) {
                feature_info_.add_pitch = true;
                if (recognizer_config_.do_cipher) {
                    ReadConfigFromAESFile(mfcc_conf_rxfilename_, &feature_info_.pitch_opts);
                    ReadConfigFromAESFile(mfcc_conf_rxfilename_, &feature_info_.pitch_process_opts);
                }
                else {
                    ::ReadConfigFromFile(mfcc_conf_rxfilename_, &feature_info_.pitch_opts);
                    ::ReadConfigFromFile(mfcc_conf_rxfilename_, &feature_info_.pitch_process_opts);
                }
            }
            else {
                feature_info_.add_pitch = false;
            }

            feature_info_.fbank_opts.frame_opts.allow_downsample = true; // It is safe to downsample
        }

        feature_info_.mfcc_opts.frame_opts.allow_downsample = true; // It is safe to downsample

        if (recognizer_config_.do_cipher) {
            ReadConfigFromAESFile(silence_conf_rxfilename_, &feature_info_.silence_weighting_config);
        }
        else {
            ::ReadConfigFromFile(silence_conf_rxfilename_, &feature_info_.silence_weighting_config);
        }
        KALDI_LOG << "Silence phones " << feature_info_.silence_weighting_config.silence_phones_str;
        KALDI_LOG << "Silence phone weight " << feature_info_.silence_weighting_config.silence_weight;

        trans_model_ = new kaldi::TransitionModel();
        nnet_ = new kaldi::nnet3::AmNnetSimple();
        // if (stat(nnet3_rxfilename_.c_str(), &buffer) == 0)
        {
            bool binary;
            kaldi::Input ki(nnet3_rxfilename_, &binary);
            trans_model_->Read(ki.Stream(), binary);
            nnet_->Read(ki.Stream(), binary);
            SetBatchnormTestMode(true, &(nnet_->GetNnet()));
            SetDropoutTestMode(true, &(nnet_->GetNnet()));
            nnet3::CollapseModel(nnet3::CollapseModelConfig(), &(nnet_->GetNnet()));
        }

        decodable_info_ = new nnet3::DecodableNnetSimpleLoopedInfo(decodable_opts_, nnet_);

        // Read ivector
        if (stat(final_ie_rxfilename_.c_str(), &buffer) == 0) {
            KALDI_LOG << "Loading i-vector extractor from " << final_ie_rxfilename_;

            OnlineIvectorExtractionConfig ivector_extraction_opts;
            ivector_extraction_opts.splice_config_rxfilename = model_path_str_ + "/ivector/splice.conf";
            ivector_extraction_opts.cmvn_config_rxfilename = model_path_str_ + "/ivector/online_cmvn.conf";
            ivector_extraction_opts.lda_mat_rxfilename = model_path_str_ + "/ivector/final.mat";
            ivector_extraction_opts.global_cmvn_stats_rxfilename = model_path_str_ + "/ivector/global_cmvn.stats";
            ivector_extraction_opts.diag_ubm_rxfilename = model_path_str_ + "/ivector/final.dubm";
            ivector_extraction_opts.ivector_extractor_rxfilename = model_path_str_ + "/ivector/final.ie";
            ivector_extraction_opts.max_count = 100;

            feature_info_.use_ivectors = true;
            feature_info_.ivector_extractor_info.Init(ivector_extraction_opts);
        } else {
            feature_info_.use_ivectors = false;
        }

        // Read global cmvn
        if (stat(global_cmvn_stats_rxfilename_.c_str(), &buffer) == 0) {
            KALDI_LOG << "Reading CMVN stats from " << global_cmvn_stats_rxfilename_;
            feature_info_.use_cmvn = true;
            ReadKaldiObject(global_cmvn_stats_rxfilename_, &feature_info_.global_cmvn_stats);
        }

        if (stat(hclg_fst_rxfilename_.c_str(), &buffer) == 0) {
            KALDI_LOG << "Loading HCLG from " << hclg_fst_rxfilename_;
            hclg_fst_ = fst::ReadFstKaldiGeneric(hclg_fst_rxfilename_);
            hcl_fst_ = NULL;
            g_fst_ = NULL;
        }

        word_syms_ = NULL;
        if (hclg_fst_ && hclg_fst_->OutputSymbols()) {
            word_syms_ = hclg_fst_->OutputSymbols();
        } else if (g_fst_ && g_fst_->OutputSymbols()) {
            word_syms_ = g_fst_->OutputSymbols();
        }
        if (!word_syms_) {
            KALDI_LOG << "Loading words from " << word_syms_rxfilename_;

            if (recognizer_config_.do_cipher){
                std::stringstream ss(aesDecoder.decodeToString(word_syms_rxfilename_));
                word_syms_ = fst::SymbolTable::ReadText(ss, word_syms_rxfilename_);
            }
            else{
                word_syms_ = fst::SymbolTable::ReadText(word_syms_rxfilename_);
            }

            if (!word_syms_)
                KALDI_ERR << "Could not read symbol table from file "
                          << word_syms_rxfilename_;
        }
        KALDI_ASSERT(word_syms_);

        if (recognizer_config_.false_alarm_check) {
            if (stat(vhclg_fst_rxfilename_.c_str(), &buffer) == 0) {
                KALDI_LOG << "Loading HCLG from " << vhclg_fst_rxfilename_;
                vhclg_fst_ = fst::ReadFstKaldiGeneric(vhclg_fst_rxfilename_);

                vword_syms_ = NULL;
                if (vhclg_fst_ && vhclg_fst_->OutputSymbols()) {
                    vword_syms_ = vhclg_fst_->OutputSymbols();
                }

                if (!vword_syms_) {
                    KALDI_LOG << "Loading words from " << vword_syms_rxfilename_;

                    if (recognizer_config_.do_cipher){
                        std::stringstream ss(aesDecoder.decodeToString(vword_syms_rxfilename_));
                        vword_syms_ = fst::SymbolTable::ReadText(ss, vword_syms_rxfilename_);
                    }
                    else{
                        vword_syms_ = fst::SymbolTable::ReadText(vword_syms_rxfilename_);
                    }

                    if (!vword_syms_)
                        KALDI_ERR << "Could not read symbol table from file "
                                  << vword_syms_rxfilename_;
                }
                KALDI_ASSERT(vword_syms_);
            }

            vtrans_model_ = new kaldi::TransitionModel();
            {
                bool binary;
                kaldi::Input ki(nnet3_rxfilename_, &binary);
                vtrans_model_->Read(ki.Stream(), binary);
            }
            KALDI_ASSERT(vword_syms_);

            if (stat(winfo_rxfilename_.c_str(), &buffer) == 0) {
                KALDI_LOG << "Loading winfo " << winfo_rxfilename_;
                kaldi::WordBoundaryInfoNewOpts opts;

                if (recognizer_config_.do_cipher){
                    winfo_ = new kaldi::WordBoundaryInfo(opts);
                    std::stringstream is(aesDecoder.decodeToString(winfo_rxfilename_));
                    winfo_->Init(is);
                }
                else{
                    winfo_ = new kaldi::WordBoundaryInfo(opts, winfo_rxfilename_);
                }
            } else {
                winfo_ = NULL;
            }

            std_lm_fst_ = NULL;
        }
    }
    catch (...) {
        throw IOException("Error during reading model file");
    }
}


void Model::Ref()
{
    ref_cnt_++;
}

void Model::Unref()
{
    ref_cnt_--;
    if (ref_cnt_ == 0) {
        delete this;
    }
}

int Model::FindWord(const char *word)
{
    if (!word_syms_)
        return -1;

    return word_syms_->Find(word);
}

Model::~Model() {
    delete trans_model_;
    delete nnet_;

    delete decodable_info_;
    delete winfo_;
    delete hclg_fst_;
    delete hcl_fst_;
    delete g_fst_;
}

void Model::ReadSilID() {
    std::unique_ptr<std::istream> is;
    std::string text;
    if (recognizer_config_.do_cipher) {
        text = aesDecoder.decodeToString(word_syms_rxfilename_);
        is = std::unique_ptr<std::istream>(new std::stringstream (text));
    }
    else {
        is = std::unique_ptr<std::istream>(new std::ifstream (word_syms_rxfilename_));
    }
    std::string word;
    int id;
    while(!(*is).eof()) {
        *is >> word >> id;
        std::transform(word.begin(), word.end(), word.begin(), [](unsigned char c){ return std::tolower(c); });
        if(word == "!sil") {
            this->sil_id_ = id;
            return;
        }
    }

    this->sil_id_ = -1;
}

int Model::GetSilID() {
    return this->sil_id_;
}

#endif