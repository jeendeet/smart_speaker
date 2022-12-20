#ifndef __HANDLEDATA__
#define __HANDLEDATA__

#include <iostream>
#include <vector>
#include "cJSON/cJSON.h"

/* Define handle data */

class HandleData
{
    public:
        static HandleData* Instance();
        std::string get_str_between_two_str(const std::string &s,
            const std::string &start_delim,
            const std::string &stop_delim);
        std::string convert_to_string(char* a);
        cJSON *file_to_json(char *filename);
        std::string to_json_string(std::string text);
        void split(std::string const &str, const char* delim,
            std::vector<std::string> &out);
        void urldecode2(char *dst, const char *src);
        std::string random_string(std::size_t length);
        std::string gen_SHA256(std::string code);
    private:
        static HandleData* _instance;
        HandleData();
        ~HandleData();

};

#endif /* __HANDLEDATA__ */