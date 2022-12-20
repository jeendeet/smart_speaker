//
// Created by vnbk on 08/07/2018.
//

#ifndef SESSIONOBSERVERINTERFACE_H
#define SESSIONOBSERVERINTERFACE_H


namespace ota{
class SessionObserverInterface{
public:
	/*virtual bool onDownloading(ota::ErrorCode errorCode) = 0;
	virtual bool onUploading(ota::ErrorCode errorCode, const std::string& type = "", const std::string& version = "") = 0;*/
	virtual void notifyFromSession() = 0;
};
}

#endif //SESSIONOBSERVERINTERFACE_H