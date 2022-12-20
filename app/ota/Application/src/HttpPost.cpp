//
// Created by vnbk on 26/10/2018.
//

#include <iostream>
#include <vector>
#include "HttpPost.h"

namespace ota{

HttpPost::HttpPost(const std::string& pathToCA, std::shared_ptr<ota::MessageConsumerInterface> messageConsumer) {
    m_handle = curl_easy_init();
    if(!m_handle){
        std::cout << "[ERROR]::HttpPost::Can not create curl handle" << std::endl;
        return ;
    }
    std::cout << "[NOTIFY]::HttpPost::Create curl handle successfully" << std::endl;
    m_headerList = nullptr;
    m_pathToCaFile = pathToCA;
    if(!setOpt()){
        std::cout << "[ERROR]::HttpPost::Can not set option default http Post " << std::endl;
        return;
    }
    if(!messageConsumer){
        std::cout << "[ERROR]::HttpPost::Message Consumer not exist " << std::endl;
        return;
    }
    m_messageConsumer = messageConsumer;
}

HttpPost::~HttpPost() {
    if(m_handle){
        curl_easy_cleanup(m_handle);
        m_handle = nullptr;
    }
    if(m_headerList){
        curl_slist_free_all(m_headerList);
        m_headerList = nullptr;
    }
}

void HttpPost::addHeader(const std::string &name, const std::string &value) {
    std::string header = name + ": " + value;
    m_headerList = curl_slist_append(m_headerList, header.c_str());
    curl_easy_setopt(m_handle, CURLOPT_HTTPHEADER, m_headerList);
}

void HttpPost::removeHeader(const std::string &name, const std::string &value) {

}

bool HttpPost::postToServer(const std::string &url, const std::string &data) {
    if(CURLE_OK != curl_easy_setopt(m_handle, CURLOPT_URL, url.c_str())){
        std::cout << "[ERROR]::HttpPost::postToServer:: can not assign url in curl " << std::endl;
        return false;
    }
    if(CURLE_OK != curl_easy_setopt(m_handle, CURLOPT_POSTFIELDS, data.c_str())){
        std::cout << "[ERROR]::HttpPost::postToServer:: can not assign payload in curl " << std::endl;
        return false;
    }

    if(CURLE_OK != curl_easy_perform(m_handle)){
        std::cout << "[ERROR]::HttpPost::postToServer:: Can not post the data to Server" << std::endl;
        return false;
    }
    return true;
}

size_t HttpPost::headerCallback(char *buffer, size_t size, size_t nitems, void *userdata){
    //std::cout << "Header of Http post: " << buffer << std::endl;
    return size*nitems;
}

size_t HttpPost::writeCallback(char *buffer, size_t size, size_t nitems, void *userdata){
    if(!userdata){
        std::cout << "[ERROR]::HttpPost::writeCallback::User is Null" << std::endl;
        return 0;
    }

    std::cout << "HttpPost::writeCallback::Data: Before " << buffer << std::endl;

    std::vector<char> data;
    int count = 0;
    for(int index = 0; index < size*nitems; index++){
        if(buffer[index] == '{'){
            count++;
        }
        if(count){
            data.push_back(buffer[index]);
        }
        if(buffer[index] == '}'){
            count--;
            if(!count)
                break;
        }
    }
    data.push_back('\0');
    std::cout << "HttpPost::writeCallback::Data: " << data.data() << std::endl;
    auto post = static_cast<HttpPost*>(userdata);
    post->m_messageConsumer->onMessage(data.data(), (int16_t)data.size());
    data.clear();
    return size*nitems;
}

bool HttpPost::setOpt() {
    m_headerList = curl_slist_append(m_headerList, "Content-Type: application/json");
    m_headerList = curl_slist_append(m_headerList, "Accept: application/json");
    m_headerList = curl_slist_append(m_headerList, "X-Lumi-Language: en-us");
    //m_headerList = curl_slist_append(m_headerList, "X-Lumi-Api-Key: Y537Z9L6IU67JVOVF5CP");
    if(curl_easy_setopt(m_handle, CURLOPT_HTTPHEADER, m_headerList) != CURLE_OK){
        std::cout << "[ERROR]::HttpPost::setOpt:: set header data error" << std::endl;
        return false;
    }

    if(curl_easy_setopt(m_handle, CURLOPT_HEADERDATA, this) != CURLE_OK){
        std::cout << "[ERROR]::HttpPost::setOpt:: set option header data error" << std::endl;
        return false;
    }
    if(curl_easy_setopt(m_handle, CURLOPT_HEADERFUNCTION, headerCallback) != CURLE_OK){
        std::cout << "[ERROR]::HttpPost::setOpt:: set option header function callback error" << std::endl;
        return false;
    }
    if(curl_easy_setopt(m_handle, CURLOPT_WRITEDATA, this) != CURLE_OK){
        std::cout << "[ERROR]::HttpPost::setOpt:: set option write data error" << std::endl;
        return false;
    }
    if(curl_easy_setopt(m_handle, CURLOPT_WRITEFUNCTION, writeCallback) != CURLE_OK){
        std::cout << "[ERROR]::HttpPost::setOpt:: set option write function callback error" << std::endl;
        return false;
    }

    if(CURLE_OK != curl_easy_setopt(m_handle, CURLOPT_SSLVERSION, CURL_SSLVERSION_MAX_TLSv1_2)){
        std::cout << "[ERROR]::HttpPost::setOpt:: set option SSL Version error" << std::endl;
        return false;
    }
    if(CURLE_OK != curl_easy_setopt(m_handle, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1)){
        std::cout << "[ERROR]::HttpPost::setOpt:: set option Http version error" << std::endl;
        return false;
    }
    if(CURLE_OK != curl_easy_setopt(m_handle, CURLOPT_USE_SSL, CURLUSESSL_ALL)){
        std::cout << "[ERROR]::HttpPost::setOpt:: set option use SSL error" << std::endl;
        return false;
    }
    if(CURLE_OK != curl_easy_setopt(m_handle, CURLOPT_SSL_VERIFYPEER, 0L)){
        std::cout << "[ERROR]::HttpPost::setOpt:: set option SSL verify peer error" << std::endl;
        return false;
    }
    if(CURLE_OK != curl_easy_setopt(m_handle, CURLOPT_SSL_VERIFYHOST, 0L)){
        std::cout << "[ERROR]::HttpPost::setOpt:: set option SSL verify host error" << std::endl;
        return false;
    }

    /*Set CA path SSL*/
    /*if(!curl_easy_setopt(m_handle, CURLOPT_CAPATH, m_pathToCaFile)){
        std::cout << "[ERROR]::HttpPost::setOpt:: path to CA file invalid " << std::endl;
        return false;
    }*/

    return true;
}

}