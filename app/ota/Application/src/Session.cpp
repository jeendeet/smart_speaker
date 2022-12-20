//
// Created by vnbk on 08/07/2018.
//

#include "Session.h"


static const std::chrono::seconds TIMEOUT_WAIT_SESSION{1800};

namespace ota{
Session::Session(ota::Session::InfoSession &info, const std::string &path,
                 std::shared_ptr<ota::SessionObserverInterface> observer) : m_infoSession{info},
                                                                            m_observer{observer},
                                                                            m_isSuccessfully{false},
                                                                            m_errorCode{ota::ErrorCode::ERR_SUCCESSFULL} {
    if(path.empty() || observer == nullptr)
        return;
	m_pathToDownload = path;
}

Session::~Session() {
	if(m_thread.joinable())
		m_thread.join();
	m_observer = nullptr;
	m_httpDownload = nullptr;
}

void Session::notify(bool isSuccessful) {
	std::lock_guard<std::mutex> lock(m_mutex);
	m_isSuccessfully = isSuccessful;
	m_triger.notify_all();
}

ota::ErrorCode Session::getErrorCode() {
	return m_errorCode;
}

void Session::start(){
	m_thread = std::thread(&Session::loop, this);
}

void Session::loop() {
	std::string nameFile = m_pathToDownload + "ota-updater.zip";
    FILE* file = ota::FileWraper::open(nameFile, "w");
    if(!file){
		m_errorCode = ota::ErrorCode::ERR_FORMAT_FILE;
		m_observer->notifyFromSession();
		std::cout << "ERROR::Session::loop: File invalid " << std::endl;
		return;
    }

	m_httpDownload = std::make_shared<ota::HTTPDownload>(m_infoSession.m_link.c_str());
	std::cout << "Session::loop:: Downloading............... " << std::endl;
	auto result = m_httpDownload->download(file);
	ota::FileWraper::close(file);
	if(!result){
	    m_errorCode = ota::ErrorCode::ERR_DOWNLOAD_FAIL;
		m_observer->notifyFromSession();
        std::cout << "ERROR::Session::loop: Download fail " << std::endl;
		return;
	}
	if(!ota::FileWraper::isValid(nameFile, m_infoSession.m_checksum)){
		m_errorCode = ota::ErrorCode::ERR_FILE_INVALID;
		m_observer->notifyFromSession();
        std::cout << "ERROR::Session::loop: File incomplete " << std::endl;
		return;
	}

    std::cout << "Session::loop: Download successfully " << std::endl;
	m_observer->notifyFromSession();
	std::unique_lock<std::mutex> lock(m_mutex);
	//m_triger.wait(lock);
	if(std::cv_status::timeout == m_triger.wait_for(lock, TIMEOUT_WAIT_SESSION)){
		m_errorCode = ota::ErrorCode::ERR_INTERNAL;
		m_observer->notifyFromSession();
        std::cout << "ERROR::Session::loop: Timeout response from main thread, exit session loop " << std::endl;
		return;
	}
	lock.unlock();

	if(!m_isSuccessfully){
	    std::cout << "ERROR::Session::loop:: Upload fail, remove download file and exit loop " << std::endl;
		ota::FileWraper::remove(nameFile);
		return;
	}

    std::cout << "Session::loop:: Extracting............... " << std::endl;
	std::string pathDirExtract = m_pathToDownload + "ota-updater";
	if(!ota::FileWraper::extract(nameFile, pathDirExtract)){
		m_errorCode = ota::ErrorCode::ERR_FORMAT_FILE;
		m_observer->notifyFromSession();
		printf("Session: loop: Error: Extracting false\n");
		return;
	}

    std::cout << "Session::loop:: Uploading............... " << std::endl;
	if(!ota::FileWraper::executed(pathDirExtract,  "/updater.sh")) {
        m_errorCode = ota::ErrorCode::ERR_FORMAT_FILE;
        m_observer->notifyFromSession();
        std::cout << "ERROR::Session::loop: Uploading fail " << std::endl;
        return;
    }
	//Remove file download and extract
	ota::FileWraper::remove(nameFile);
	ota::FileWraper::remove(pathDirExtract);

    std::cout << "Session::loop:: Upload successfully, notify to Main thread and exit loop " << std::endl;
    m_observer->notifyFromSession();
}

}