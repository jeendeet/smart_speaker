//
// Created by vnbk on 08/07/2018.
//

#ifndef OTA_V2_SESSION_H
#define OTA_V2_SESSION_H

#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <condition_variable>

#include "HTTPDownload.h"
#include "FileWraper.h"
#include "SessionObserverInterface.h"
#include "OtaDefines.h"

namespace ota {
class Session {
public:
	struct InfoSession{
		std::string m_link;
		std::string m_checksum;
        std::int16_t m_versionNumber;
        std::int16_t m_versionMin;
		std::string m_versionName;
		std::int16_t m_downloadId;

		InfoSession(){}
		InfoSession(InfoSession& info){
            m_versionNumber = info.m_versionNumber;
			m_link = info.m_link;
			m_checksum = info.m_checksum;
			m_versionName = info.m_versionName;
			m_versionMin = info.m_versionMin;
			m_downloadId = info.m_downloadId;
		}
	};
	Session(ota::Session::InfoSession& info, const std::string& path,
			std::shared_ptr<ota::SessionObserverInterface> observer);
	~Session();

	void start();
	void notify(bool isSuccessful);


	ota::ErrorCode getErrorCode();
	InfoSession m_infoSession;
private:
	void loop();

	std::shared_ptr<ota::SessionObserverInterface> m_observer;

	std::shared_ptr<ota::HTTPDownload> m_httpDownload;
	std::string m_pathToDownload;

	std::mutex m_mutex;
	std::condition_variable m_triger;
	std::thread m_thread;

	ota::ErrorCode m_errorCode;
	bool m_isSuccessfully;
};
}

#endif //OTA_V2_SESSION_H
