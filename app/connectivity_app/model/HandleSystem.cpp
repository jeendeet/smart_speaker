#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <net/if.h>
#include <unistd.h>

#include "cJSON/cJSON.h"
#include "HandleSystem.h"
#include "HandleData.h"
#include "../config/log.h"
#include "../config/define.h"


// Definition
HandleSystem::HandleSystem(){}
HandleSystem::~HandleSystem(){}
HandleSystem* HandleSystem::_instance = 0;
HandleSystem* HandleSystem::Instance() {
   if(_instance == 0) {
       _instance = new HandleSystem();
   }
   return _instance;
}

/* execute command line in linux */
std::string HandleSystem::exec(const char* cmd) {
    LOGI("HandleSystem::exec cmd %s", cmd);
    char buffer[128];
    std::string result = "";
    FILE* pipe = popen(cmd, "r");
    if (!pipe) throw std::runtime_error("popen() failed!");
    try 
    {
        while (fgets(buffer, sizeof buffer, pipe) != NULL) 
        {
            result += buffer;
        }
    } 
    catch (...) 
    {
        pclose(pipe);
        throw;
    }
    pclose(pipe);
    return result;
}

/* get MacAddress */
std::string HandleSystem::getMacAddress(std::string interface) {
    std::string mac;
    std::ifstream macAddress(interface);

    getline(macAddress, mac);
    macAddress.close();

    return mac;
}

/* System device authenticate generate code */
std::string HandleSystem::authenticate() {
    std::ofstream stream;
    cJSON *j_content;
    std::string code_challenge, code_verifier, re_success;

    code_verifier = HandleData::Instance()->random_string(12);
    code_challenge = HandleData::Instance()->gen_SHA256(code_verifier);

    /* save sha256 code to info.config */
    char *json_file = (char*) DEVICE_INFO_FILE;
    j_content = HandleData::Instance()->file_to_json(json_file);
    cJSON_AddStringToObject(j_content,"sha256_code", (char*)code_verifier.c_str());
    char *rendered=cJSON_Print(j_content);
    
    stream.open((char*)INFO_FILE);
    if(!stream)
        LOGD("Opening file failed");
    // use operator<< for clarity
    stream << rendered << std::endl;
    // test if write was succesfull - not *really* necessary
    if(!stream)
        LOGD("Write failed");
    
    return code_challenge;
}
