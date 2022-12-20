//
// Created by vnbk on 26/10/2018.
//

#ifndef OTA_HC_V1_HTTPPOST_H
#define OTA_HC_V1_HTTPPOST_H

#include <memory>
#include <string>
#include <curl/curl.h>

#include "MessageConsumerInterface.h"

namespace ota {

class HttpPost {
public:
    HttpPost(const std::string& pathToCA, std::shared_ptr<ota::MessageConsumerInterface> messageConsumer);
    ~HttpPost();

    void addHeader(const std::string& name, const std::string& value);
    void removeHeader(const std::string& name, const std::string& value);

    bool postToServer(const std::string& url, const std::string& data);

    static size_t headerCallback(char *buffer, size_t size, size_t nitems, void *userdata);
    static size_t writeCallback(char *buffer, size_t size,size_t nitems, void *userdata);
private:
    bool setOpt();

    std::shared_ptr<ota::MessageConsumerInterface> m_messageConsumer;

    std::string m_pathToCaFile;
    struct curl_slist* m_headerList;
    CURL* m_handle;
};
}

#endif //OTA_HC_V1_HTTPPOST_H
