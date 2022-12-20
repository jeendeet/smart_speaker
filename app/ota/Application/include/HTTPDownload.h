//
// Created by vnbk on 05/07/2018.
//

#ifndef EXAMPLECMAKE_HTTPDOWNLOAD_H
#define EXAMPLECMAKE_HTTPDOWNLOAD_H

#include <curl/curl.h>

#include "FileWraper.h"

namespace ota {

class HTTPDownload {
public:
	HTTPDownload(const char* url);
	~HTTPDownload();

	bool download(FILE* file);
private:
	bool setOptions();
	CURL* m_handle;
	std::string m_url;
};
}

#endif //EXAMPLECMAKE_HTTPDOWNLOAD_H
