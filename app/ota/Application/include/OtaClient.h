//
// Created by vnbk on 20/08/2018.
//

#ifndef OTA_V1_OTACLIENT_H
#define OTA_V1_OTACLIENT_H

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>

#include "MQTTClientObserverInterface.h"
#include "MQTTClient.h"
#include "Session.h"
#include "SessionObserverInterface.h"
#include "HTTPPing.h"
#include "../../libs/rapidjson/rapidjson-1.1.0/include/rapidjson/document.h"
#include "../../libs/rapidjson/rapidjson-1.1.0/include/rapidjson/error/en.h"
#include "../../libs/rapidjson/rapidjson-1.1.0/include/rapidjson/stringbuffer.h"
#include "../../libs/rapidjson/rapidjson-1.1.0/include/rapidjson/writer.h"
#include "OtaDefines.h"
#include "HttpPost.h"
#include "MessageConsumerInterface.h"

namespace ota {
using namespace std;

struct OtaUpdateInfo{
    bool m_updated;
    std::int16_t m_downloadId;

    OtaUpdateInfo() = default;
};

struct OtaVersionInfo{
    std::string m_mac;
    std::string m_versionName;
    std::int16_t m_versionNumber;
    std::int16_t m_versionMin;

    OtaUpdateInfo m_oldUpdateInfo;

    OtaVersionInfo() = default;
};

struct OtaApiInfo{
    std::string m_urlBase;
    std::string m_apiKey;

    std::string m_pathUrlOtaCheckUpdate;
    std::string m_pathUrlOtaUpdateStatus;

    std::int16_t m_requestCycleTime;
    std::int16_t m_maxRandomTime;

    OtaApiInfo() = default;
};

struct OtaPathConfig{
    std::string m_pathVersionInfo;
    std::string m_pathSystemConfig;

    OtaPathConfig() = default;
};

class OtaClient :   public ota::MQTTClientObserverInterface ,
                    public ota::MessageConsumerInterface,
                    public ota::SessionObserverInterface,
                    public std::enable_shared_from_this<OtaClient>{
public:
    static std::shared_ptr<ota::OtaClient> create(const std::string& pathToVersionInfo, const std::string& pathToConfig);
    ~OtaClient();

    void onMqttConnect(std::string clientId) override;
    ///
    void onMqttMessage(std::string clientId, const struct mosquitto_message *message)override;
    ///
    void onMqttSubscribe(std::string clientId, int mid, int qos_count, const int *granted_qos) override;

    void onMqttPublish(std::string clientId, int messageId) override;

    void onMessage(void* data, int16_t len) override ;

    void notifyFromSession() ;

    void run();
private:

    OtaClient() = default;
    bool initial(const std::string& pathToVersionInfo, const std::string& pathToConfig);

    void processInitialState();
    void processConnectingState();
    void processPrepareDownloadState();
    void processDownloadingState();
    void processPrepareUploadState();
    void processUploadingState();
    void processIdleState();

    void backup();
    void checkNewVersion();
    bool saveCurrentVersion();
    void notifyToServer(const std::int16_t downloadId, ota::OtaMessageStatus status, ota::ErrorCode code, const std::string& errorMessage);

    bool checkConfigFile(rapidjson::Document& document);
    bool checkVersionInfo(rapidjson::Document& document);
    bool checkSystemConfigInfo(rapidjson::Document& document);

    void updateVersionInfo(const std::string& name, bool value);
    void updateVersionInfo(const std::string& versionName, const std::int16_t versionNumber, const std::int16_t versionMin, const std::int16_t downloadId, const bool updating, const bool updated);

    bool initialMqttLocal();
    bool setupMqttLocal();
    void mqttLocalLoop();

    void reset();
    void resetSession();
    std::string getMacAddress();

    bool m_isStopping;
    ota::OtaClientState m_state;

    ota::OtaVersionInfo m_versionInfo;
    ota::OtaApiInfo m_apiInfo;
    ota::OtaPathConfig m_otaPathConfig;

    std::shared_ptr<ota::Session> m_session;

    std::mutex m_lock;
    std::condition_variable m_trigger;

    std::thread m_mqttLocalThread;
    bool m_isMqttClientStopping;
    std::shared_ptr<ota::MQTTClient> m_mqttClient;

    std::unique_ptr<ota::HttpPost> m_httpPost;
};
}

#endif //OTA_V1_OTACLIENT_H
