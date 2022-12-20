//
// Created by vnbk on 05/07/2018.
//

#include "../include/HTTPDownload.h"

namespace ota {
HTTPDownload::HTTPDownload(const char* url) {
	if(!url){
		return ;
	}
	m_handle = curl_easy_init();
	m_url = url;
}

HTTPDownload::~HTTPDownload() {
	curl_easy_cleanup(m_handle);
}

bool HTTPDownload::download(FILE* file) {
	if(!file){
		printf("Error: HTTPDownload: download: File is null\n");
		return false;
	}
	if(curl_easy_setopt(m_handle, CURLOPT_URL, m_url.c_str()) != CURLE_OK){
		printf("Error: HTTPDownload: download: url invalid\n");
		return false;
	}
	if(!setOptions()){
		printf("Error: HTTPDownload: download: Options false\n");
		return false;
	}

	if(curl_easy_setopt(m_handle, CURLOPT_WRITEDATA, (void*)file) != CURLE_OK){
		printf("Error: HTTPDownload: download: File is not open\n");
		return false;
	}

	auto result = curl_easy_perform(m_handle);

	if(result != CURLE_OK){
		printf("Error: HTTPDownload: download: Don't download file\n");
		return false;
	}
	return true;
}

bool HTTPDownload::setOptions() {
	curl_easy_setopt(m_handle, CURLOPT_CUSTOMREQUEST, "GET");
	curl_easy_setopt(m_handle, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(m_handle, CURLOPT_HEADER, 0);
	return true;
}

}