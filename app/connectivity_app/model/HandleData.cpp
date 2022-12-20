#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <sstream> 
#include <vector>
#include <random>
#include <algorithm>

#include "cJSON/cJSON.h"
#include "SHA256/SHA256.h"
#include "HandleData.h"
#include "../config/log.h"

// Definition
HandleData::HandleData(){}
HandleData::~HandleData(){}
HandleData* HandleData::_instance = 0;
HandleData* HandleData::Instance() {
   if(_instance == 0) {
       _instance = new HandleData();
   }
   return _instance;
}

/* get string between two string */
std::string HandleData::get_str_between_two_str(const std::string &s,
    const std::string &start_delim,
    const std::string &stop_delim)
{
    LOGI("HandleData::get_str_between_two_str");
    unsigned first_delim_pos = s.find(start_delim);
    unsigned end_pos_of_first_delim = first_delim_pos + start_delim.length();
    unsigned last_delim_pos = s.find(stop_delim);

    return s.substr(end_pos_of_first_delim,
        last_delim_pos - end_pos_of_first_delim);
};

/* converts character array to string and returns it */
std::string HandleData::convert_to_string(char* a)
{
    LOGI("HandleData::convert_to_string");
    int size = strlen(a);
    int i;
    std::string s = "";
    for (i = 0; i < size; i++) {
        s = s + a[i];
    }
    return s;
}

/* Read a file, parse, render back, etc. */
cJSON *HandleData::file_to_json(char *filename)
{
    LOGI("HandleData::file_to_json");
    FILE *f;long len;char *data;
    cJSON *content;
    
    f=fopen(filename,"rb");fseek(f,0,SEEK_END);len=ftell(f);fseek(f,0,SEEK_SET);
    data=(char*)malloc(len+1);fread(data,1,len,f);fclose(f);
    content=cJSON_Parse(data);
    free(data);
    return content;
}

/* split string from string */
void HandleData::split(std::string const &str, const char* delim,
            std::vector<std::string> &out)
{
    char *token = strtok(const_cast<char*>(str.c_str()), delim);
    while (token != nullptr)
    {
        out.push_back(std::string(token));
        token = strtok(nullptr, delim);
    }
}

/* url decode */
void HandleData::urldecode2(char *dst, const char *src)
{
    char a, b;
    while (*src) {
            if ((*src == '%') &&
                ((a = src[1]) && (b = src[2])) &&
                (isxdigit(a) && isxdigit(b))) {
                    if (a >= 'a')
                            a -= 'a'-'A';
                    if (a >= 'A')
                            a -= ('A' - 10);
                    else
                            a -= '0';
                    if (b >= 'a')
                            b -= 'a'-'A';
                    if (b >= 'A')
                            b -= ('A' - 10);
                    else
                            b -= '0';
                    *dst++ = 16*a+b;
                    src+=3;
            } else if (*src == '+') {
                    *dst++ = ' ';
                    src++;
            } else {
                    *dst++ = *src++;
            }
    }
    *dst++ = '\0';
}

/* Make json string only for wifi scanner */
/* text_json = 
{
    "cell_1":
    {
        "essid": "AndroidAP_5775",
        "quality": "52/70",
        "encryption": "on",
        "signal_level": "-65 dBm",
        "last_beacon": "8ms ago"
    },
    "cell_2":
    {
        "essid": "VinBigdata-Staff",
        "quality": "52/70",
        "encryption": "off",
        "signal_level": "-65 dBm",
        "last_beacon": "8ms ago"
    },
}
*/
std::string HandleData::to_json_string(std::string text)
{
    LOGI("HandleData::to_json_string");
    std::string text_json = "{";
    std::string delim = "Cell "; /* delimiter */

    size_t pos = 0;
    int count = 0;
    std::string token1; /* define a string variable*/
    std::vector<std::string> wifi_cell;

    /* use find() function to get the position of the delimiters */ 
    while (( pos = text.find (delim)) != std::string::npos)  
    {
        /* store the substring */
        token1 = text.substr(0, pos); 
        wifi_cell.push_back(token1);
        /* erase() function store the current positon and move to next token. */
        text.erase(0, pos + delim.length());  
    }
    wifi_cell.push_back(text);
    wifi_cell.erase(wifi_cell.begin());
    for (auto it = wifi_cell.begin(); it != wifi_cell.end(); it++)
    {   std::string temp = this->get_str_between_two_str(*it, "ESSID:", "Quality:");
        if ((text_json.find(temp) == std::string::npos) &&
            (temp.find("x00") == std::string::npos) && ("" != temp)) {

            /* add encryption key to check wifi has pass or not*/
            std::string encryption_key = this->get_str_between_two_str(*it, "key:", "Bit");
            remove(encryption_key.begin(), encryption_key.end(), '\n');
            encryption_key.erase(remove(encryption_key.begin(), encryption_key.end(), ' '), encryption_key.end());

            text_json = text_json + "\"cell_" + std::to_string(count++) + "\":{";
            text_json = text_json + "\"essid\": " + temp + "," +\
                "\"quality\": \"" + this->get_str_between_two_str(*it, "Quality:", "Signal") + "\"," +\
                "\"encryption\": \"" + encryption_key + "\"," +\
                "\"signal_level\": \"" + this->get_str_between_two_str(*it, "level:", "dBm") + "dBm\"},";
        }
    }
    text_json = text_json.substr(0, text_json.size()-1);
    text_json = text_json + "} ";  
    return text_json;
}

/* generate a random string*/
std::string HandleData::random_string(std::size_t length)
{
    const std::string CHARACTERS = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

    std::random_device random_device;
    std::mt19937 generator(random_device());
    std::uniform_int_distribution<> distribution(0, CHARACTERS.size() - 1);

    std::string random_string;

    for (std::size_t i = 0; i < length; ++i)
    {
        random_string += CHARACTERS[distribution(generator)];
    }

    return random_string;
}

/* generate SHA256 string*/
std::string HandleData::gen_SHA256(std::string code)
{
    SHA256 sha;
    std::string code_sha256;
    sha.update(code);
    uint8_t * digest = sha.digest();
    code_sha256 = SHA256::toString(digest);
    std::cout << SHA256::toString(digest) << std::endl;
    return code_sha256;
}
