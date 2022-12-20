#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "cJSON/cJSON.h"
#include "HttpResponse.h"
#include "HandleData.h"

#include "../config/define.h"
#include "../config/log.h"

// Definition
HttpResponse::HttpResponse(){}
HttpResponse::~HttpResponse(){}
HttpResponse* HttpResponse::_instance = 0;
HttpResponse* HttpResponse::Instance() {
   if(_instance == 0) {
       _instance = new HttpResponse();
   }
   return _instance;
}

/* Create response for http requests */
/* Restful API format */
/*
    "Server: {}\n"
    "Last-Modified: {}\n"
    "ETag: {}\n"
    "Content-Type: {}\n"
    "Content-Length: {}\n"
    "Accept-Ranges: {}\n"
    "Connection: {}\n"
    "\n"
    "{message}"
*/
char* HttpResponse::create_response(char* status, char* content)
{
    LOGI("HttpResponse::create_response: begin");
    char *reply = (char *)malloc(strlen(status)*10);
    strcpy(reply, status);

    /* generate current time*/
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    char s[64];
    strftime(s, sizeof(s), "Date: %c\n", tm);
    strcat(reply, s);

    /* create API format */
    cJSON *j_content;
    char *json_file = (char*) JSON_FILE;
    j_content = HandleData::Instance()->file_to_json(json_file);
    cJSON *http_response = cJSON_GetObjectItem(j_content, "http_response");

    std::string sub_string = \
        "Server: " +\
        HandleData::Instance()->convert_to_string(
            cJSON_GetObjectItem(http_response, "server_name")->valuestring) + "\n" +\
        "Last-Modified: " +\
        HandleData::Instance()->convert_to_string(
            cJSON_GetObjectItem(http_response, "last_modified_server")->valuestring) + "\n" +\
        "ETag: " +\
        HandleData::Instance()->convert_to_string(
            cJSON_GetObjectItem(http_response, "e_tag")->valuestring) + "\n" +\
        "Content-Type: " +\
        HandleData::Instance()->convert_to_string(
            cJSON_GetObjectItem(http_response, "content_type")->valuestring) + "\n" +\
        "Content-Length: " + std::to_string(strlen(content)) + "\n" +\
        "Accept-Ranges: " +\
        HandleData::Instance()->convert_to_string(
            cJSON_GetObjectItem(http_response, "accept_ranges")->valuestring) + "\n" +\
        "Connection: " +\
        HandleData::Instance()->convert_to_string(
            cJSON_GetObjectItem(http_response, "connection")->valuestring) + "\n" +\
        "\n";

    strcat(reply, sub_string.c_str());
    strcat(reply, content);
    LOGI("HttpResponse::create_response: end");
    return reply;
};
