#ifndef CONFIG_READER_H
#define CONFIG_READER_H

#include "defs.h"
#ifdef USE_CIPHER
#ifdef USE_CONFIG

#include "base/kaldi-common.h"
#include "util/parse-options.h"
#include "cipher.h"

using namespace kaldi;

namespace config {
    class AESParseOptions: public kaldi::ParseOptions {
    public:
        AESParseOptions(const char *usage):                             kaldi::ParseOptions(usage)          {}
        AESParseOptions(const std::string &prefix, OptionsItf *other):  kaldi::ParseOptions(prefix, other)  {}
        ~AESParseOptions(){}

        void ReadConfigFile(const std::string& filename);
    private:
        AESDecoder aesDecoder;
    };

    template<class C> void ReadConfigFromAESFile(const std::string &config_filename, C *c) {
        std::ostringstream usage_str;
        usage_str << "Parsing config from "
                  << "from '" << config_filename << "'";
        AESParseOptions po(usage_str.str().c_str());
        c->Register(&po);
        po.ReadConfigFile(config_filename);
    }

    class ParseOptions: public kaldi::ParseOptions {
    public:
        ParseOptions(const char *usage):                            kaldi::ParseOptions(usage)          {}
        ParseOptions(const std::string &prefix, OptionsItf *other): kaldi::ParseOptions(prefix, other)  {}
        ~ParseOptions(){}

        void ReadConfigFile(const std::string& filename);
    };

    template<class C> void ReadConfigFromFile(const std::string &config_filename, C *c) {
        std::ostringstream usage_str;
        usage_str << "Parsing config from "
                  << "from '" << config_filename << "'";
        ::ParseOptions po(usage_str.str().c_str());
        c->Register(&po);
        po.ReadConfigFile(config_filename);
    }
};


bool ReadAESIntegerVectorSimple(const std::string &rxfilename, std::vector<int32> *list);
#endif
#endif

#endif
