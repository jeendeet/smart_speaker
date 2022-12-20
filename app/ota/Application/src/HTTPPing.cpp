//
// Created by vnbk on 13/07/2018.
//

#include "../include/HTTPPing.h"

namespace ota{
HTTPPing::HTTPPing(const std::string &url) {
	if(url.empty())
		return;
	m_handle = curl_easy_init();

	curl_easy_setopt(m_handle, CURLOPT_URL, url.c_str());
	curl_easy_setopt(m_handle, CURLOPT_CUSTOMREQUEST, "GET");
	curl_easy_setopt(m_handle, CURLOPT_FOLLOWLOCATION, 1);
	curl_easy_setopt(m_handle, CURLOPT_HEADER, 0);
}

HTTPPing::~HTTPPing() {
	if(m_handle)
		curl_easy_cleanup(m_handle);
}

bool HTTPPing::ping() {
	auto result = curl_easy_perform(m_handle);
	if(result != CURLE_OK)
		return false;
	return true;
}
}