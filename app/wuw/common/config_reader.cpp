//
// Created by ducnd on 27/10/2020.
//
#include "defs.h"
#ifdef USE_CIPHER
#ifdef USE_KALDI

#include "config_reader.h"
#include "base/kaldi-common.h"
#include "util/text-utils.h"
#include "util/parse-options.h"
#include "base/kaldi-common.h"
#include <sstream>

void AESParseOptions::ReadConfigFile(const std::string &filename) {
    std::stringstream is(aesDecoder.decodeToString(filename));

    std::string line, key, value;
    int32 line_number = 0;
    while (std::getline(is, line)) {
        line_number++;
        // trim out the comments
        size_t pos;
        if ((pos = line.find_first_of('#')) != std::string::npos) {
            line.erase(pos);
        }
        // skip empty lines
        kaldi::Trim(&line);
        if (line.length() == 0) continue;

        if (line.substr(0, 2) != "--") {
            KALDI_ERR << "Reading config file " << filename
                      << ": line " << line_number << " does not look like a line "
                      << "from a Kaldi command-line program's config file: should "
                      << "be of the form --x=y.  Note: config files intended to "
                      << "be sourced by shell scripts lack the '--'.";
        }

        // parse option
        bool has_equal_sign;
        SplitLongArg(line, &key, &value, &has_equal_sign);
        NormalizeArgName(&key);
        kaldi::Trim(&value);
        if (!SetOption(key, value, has_equal_sign)) {
            PrintUsage(true);
            KALDI_LOG << "Invalid option " << line << " in config file " << filename;
        }
    }
}

void ParseOptions::ReadConfigFile(const std::string &filename) {
    std::fstream is(filename);

    std::string line, key, value;
    int32 line_number = 0;
    while (std::getline(is, line)) {
        line_number++;
        // trim out the comments
        size_t pos;
        if ((pos = line.find_first_of('#')) != std::string::npos) {
            line.erase(pos);
        }
        // skip empty lines
        kaldi::Trim(&line);
        if (line.length() == 0) continue;

        if (line.substr(0, 2) != "--") {
            KALDI_ERR << "Reading config file " << filename
                      << ": line " << line_number << " does not look like a line "
                      << "from a Kaldi command-line program's config file: should "
                      << "be of the form --x=y.  Note: config files intended to "
                      << "be sourced by shell scripts lack the '--'.";
        }

        // parse option
        bool has_equal_sign;
        SplitLongArg(line, &key, &value, &has_equal_sign);
        NormalizeArgName(&key);
        kaldi::Trim(&value);
        if (!SetOption(key, value, has_equal_sign)) {
            PrintUsage(true);
            KALDI_LOG << "Invalid option " << line << " in config file " << filename;
        }
    }
}

bool ReadAESIntegerVectorSimple(const std::string &rxfilename, std::vector<int32> *list) {
    AESDecoder aesDecoder;
    std::stringstream is(aesDecoder.decodeToString(rxfilename));
    int32 i;
    list->clear();
    while ( !(is >> i).fail() )
        list->push_back(i);
    is >> std::ws;
    return is.eof();  // should be eof, or junk at end of file.
}

#endif
#endif