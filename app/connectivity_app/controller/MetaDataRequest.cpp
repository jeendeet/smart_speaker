#include <algorithm>

#include "MetaDataRequest.h"
#include "../model/HandleSystem.h"
#include "../model/HandleData.h"
#include "../config/define.h"
#include "../model/cJSON/cJSON.h"
#include "../config/log.h"

// Definition
MetaDataRequest::MetaDataRequest(){}
MetaDataRequest::~MetaDataRequest(){}
MetaDataRequest* MetaDataRequest::_instance = 0;
MetaDataRequest* MetaDataRequest::Instance() {
   if(_instance == 0) {
       _instance = new MetaDataRequest();
   }
   return _instance;
}

/* Metadata request */
/* text_json = 
{
    "product_id": "VinBigdata-Speaker-123456",
    "mac_address": "b8:27:eb:8f:41:e6",
    "device_serial_number": "036000291452",
    "code_challenge": "T0HiVJtkKoX6"
}
*/
std::string MetaDataRequest::metadata_request() {

    LOGI("MetaDataRequest::metadata_request");
    std::string result_json;
    std::string mac, seri_num;

    /* create API format */
    cJSON *j_content;
    std::string code_challenge;
    char *json_file = (char*) DEVICE_INFO_FILE;
    j_content = HandleData::Instance()->file_to_json(json_file);

    mac = HandleSystem::Instance()->getMacAddress(
        HandleData::Instance()->convert_to_string(
            cJSON_GetObjectItem(j_content, "interface")->valuestring));
    seri_num = HandleData::Instance()->convert_to_string(
            cJSON_GetObjectItem(j_content, "device_serial_number")->valuestring);
    code_challenge = HandleSystem::Instance()->authenticate();

    result_json = "{\"product_id\": \"" +\
        HandleData::Instance()->convert_to_string(
            cJSON_GetObjectItem(j_content, "product_id")->valuestring) +\
            "\",\"mac_address\": \"" + mac +\
            "\",\"device_serial_number\": \"" + seri_num +\
            "\",\"code_challenge\": \"" + code_challenge + "\"}";
    return result_json;
}

std::string MetaDataRequest::ipaddress_request(bool &_result) {
    LOGI("MetaDataRequest::ipaddress_request");
    std::string result_json;
    std::string cmd = \
        "ifconfig " +\
        (std::string)INTERFACE_ETHERNET +\
        " 2>/dev/null|awk '/inet addr:/ {print $2}'|sed 's/addr://'";
    LOGI("MetaDataRequest::ipaddress_request cmd %s", cmd.c_str());
    std::string result = HandleSystem::Instance()->exec(cmd.c_str());
    if (result.empty()) {
        _result = false;
    }
    else {
        result.erase(remove(result.begin(), result.end(), '\n'), result.end()); 
        result_json = "{\"ip_address\": \"" + result + "\"}";
        _result = true;
    }
    return result_json;

}