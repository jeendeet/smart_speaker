#include <iostream>
#include <string>
#include <algorithm>

#include "WifiRequest.h"
#include "../model/HandleSystem.h"
#include "../model/HandleData.h"
#include "../model/HttpResponse.h"
#include "../config/define.h"
#include "../config/log.h"

// Definition
WifiRequest::WifiRequest(){}
WifiRequest::~WifiRequest(){}
WifiRequest* WifiRequest::_instance = 0;
WifiRequest* WifiRequest::Instance() {
   if(_instance == 0) {
       _instance = new WifiRequest();
   }
   return _instance;
}

/* wifi connect request */
std::string WifiRequest::wifi_connect_request(char* ssid, char* psk) {

    LOGI("WifiRequest::wifi_connect_request");
    std::string path_file = HandleSystem::Instance()->exec("pwd");
    path_file.erase(remove(path_file.begin(), path_file.end(), '\n'), path_file.end()); 
    std::string cmd = \
        "source " + \
        path_file + \
        "/script/wifi_connect_request.sh \"" + \
        HandleData::Instance()->convert_to_string(ssid) +\
        "\" \"" + HandleData::Instance()->convert_to_string(psk) + "\"";
    LOGI("WifiRequest::wifi_connect_request cmd %s", cmd.c_str());
    std::string result = HandleSystem::Instance()->exec(cmd.c_str());
    return result;
}

/* wifi scanning */
std::string WifiRequest::wifi_scanner(bool &_result) {

    LOGI("WifiRequest::wifi_scanner");
    std::string result_json;
    std::string path_file = HandleSystem::Instance()->exec("pwd");
    path_file.erase(remove(path_file.begin(), path_file.end(), '\n'), path_file.end());

    std::string cmd = \
        path_file + \
        "/../common/wifi/iwlist " +\
        (std::string)INTERFACE_ETHERNET +\
        " scanning | egrep 'Cell |Encryption|Quality|Signal level|Last beacon|ESSID|Bit Rates'";
    LOGI("WifiRequest::wifi_scanner cmd %s", cmd.c_str());
    std::string result = HandleSystem::Instance()->exec(cmd.c_str());
    if (result.find("ESSID") != std::string::npos) {
        result_json = HandleData::Instance()->to_json_string(result);
        _result = true;
    }
    else {
        _result = false;
    }
    
    return result_json;
}

/* wifi checking */
char *WifiRequest::wifi_checking(std::string text){
    char *response;
    
    if ((text.find("INTERNET_READY") != std::string::npos) ||
        (text.find("INTERNET_READY_WITH_DHCP") != std::string::npos)) {
            response = HttpResponse::Instance()->create_response(
                (char*)HTTP_200_OK,
                (char*)WIFI_CONNECTION
            );
    }
    else if (text.find("WRONG_KEY") != std::string::npos) {
            response = HttpResponse::Instance()->create_response(
                    (char*)HTTP_406_ERROR,
                    (char*)WIFI_WRONG_KEY
                );
    }
    else if (text.find("NO_INTERNET") != std::string::npos) {
            response = HttpResponse::Instance()->create_response(
                    (char*)HTTP_407_ERROR,
                    (char*)WIFI_NO_INTERNET
                );
    }
    else {
            response = HttpResponse::Instance()->create_response(
                    (char*)HTTP_405_ERROR,
                    (char*)WIFI_CONNECTION_FAIL
                );
    }
    return response;
}
