//
// Created by vnbk on 13/07/2018.
//

#ifndef OTA_V1_HTTPPING_H
#define OTA_V1_HTTPPING_H

#include <memory>
#include <curl/curl.h>

namespace ota {
using namespace std;
class HTTPPing {
public:
	HTTPPing(const std::string& url);
	~HTTPPing();

	bool ping();

private:
	CURL* m_handle;
};
}

#endif //OTA_V1_HTTPPING_H
