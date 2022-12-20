//
// Created by vnbk on 20/08/2018.
//

#include <fstream>
#include <sstream>
#include <time.h>
#include <random>

#include "../include/OtaClient.h"

const static std::chrono::seconds TIMEOUT_DOWNLOADING_STATE{1800};
const static std::chrono::seconds TIMEOUT_UPLOADING_STATE{1800};

const static std::chrono::seconds TIMEOUT_CONNECT_TO_BROKER{5};
const static std::chrono::seconds TIMEOUT_SUBSCRIBE_TO_BROKER{5};

const static std::string PATH_DOWNLOAD_DEFAULT =    "/tmp/"; //
const static std::string PATH_EXECUTED_DEFAULT =    "/root/"; //
const static std::string PATH_BACKUP_DEFAULT =      "/opt/";  //

/*const static std::string PATH_DOWNLOAD_DEFAULT =    "/media/vnbk/Data/Projects/HC/ota/ota-hc-v1/Test/pathDownload/"; //"/tmp/"; //
const static std::string PATH_EXECUTED_DEFAULT =     "/media/vnbk/Data/Projects/HC/ota/ota-hc-v1/Test/pathExecuted/"; //"/root/"; //
const static std::string PATH_BACKUP_DEFAULT =      "/media/vnbk/Data/Projects/HC/ota/ota-hc-v1/Test/pathBackup/"; //"/opt/";  //*/

const static std::string LOCAL_APPINIT_CONTROL_TOPIC = "/local/milo/appInit/control";
const static std::string LOCAL_APPINIT_STATUS_TOPIC = "/local/milo/appInit/status";
const static std::string LOCAL_OTACLIENT_STATUS_TOPIC = "/local/milo/otaClient/status";

const static std::string CLOUD_CONTROL_TOPIC = "/cloud/milo/control";

const static std::string CMD_REQUEST_CHECK_UPDATE_OTA = "requestCheckUpdateOta";


namespace ota{

std::shared_ptr<ota::OtaClient> OtaClient::create(const std::string& pathToVersionInfo, const std::string& pathToConfig) {
    std::shared_ptr<ota::OtaClient> otaClient(new OtaClient());
    if(otaClient && otaClient->initial(pathToVersionInfo, pathToConfig)){
        std::cout << " OtaClient::create:: Created otaClient successfully " << std::endl;
        return otaClient;
    }
    std::cout << "ERROR::OtaClient::create:: Do not create otaclient " << std::endl;
    return nullptr;
}

OtaClient::~OtaClient() {
    reset();
}

void OtaClient::onMqttConnect(std::string clientId) {
    std::cout << "OtaClient::onMqttConnect:: ClientId " << clientId.c_str() << std::endl;
    std::lock_guard<std::mutex> lock(m_lock);
    m_trigger.notify_all();
}

void OtaClient::onMqttSubscribe(std::string clientId, int mid, int qos_count, const int *granted_qos) {
    std::cout << "OtaClient::onMqttSubscribe:: ClientId " << clientId.c_str() << std::endl;
    std::lock_guard<std::mutex> lock(m_lock);
    m_trigger.notify_all();
}

void OtaClient::onMqttPublish(std::string clientId, int messageId) {
    std::cout << "OtaClient::onMqttPublish:: ClientId " << clientId.c_str() << std::endl;
}

void OtaClient::onMqttMessage(std::string clientId, const struct mosquitto_message *message) {
    if(clientId.empty()){
        std::cout << "ERROR::OtaClient::onMqttMessge:: Client Id Invalid " << std::endl;
        return;
    }
    if(!message->payload && !message->payloadlen){
        std::cout << "ERROR::OtaClient::onMqttMessage:: Message Payload is NULL " << std::endl;
        return;
    }
    std::cout << "OtaClient::onMqttMessge:: Receive message " << message->topic << " : " << (const char*)message->payload << std::endl;
    if(ota::OtaClientState::IDLE != m_state){
        std::cout << "ERROR::OtaClient::onMqttMessage:: Program is uploading, do not receive other message " << std::endl;
        return;
    }

    std::lock_guard<std::mutex> lock(m_lock);
    rapidjson::Document document;
    if (document.Parse((const char *) message->payload).HasParseError()) {
        std::cout << "ERROR::OtaClient::onMqttMessage:: Message is not format " << std::endl;
        return;
    }
    if(!document.HasMember("cmd")){
        std::cout << "ERROR::OtaClient::onMqttMessage:: Message have not CMD field " << std::endl;
        return;
    }
    auto cmd = document.FindMember("cmd");
    if(CMD_REQUEST_CHECK_UPDATE_OTA == cmd->value.GetString()){
        std::cout << "INFO::OtaClient::onMqttMessage:: Request check Ota version" << std::endl;
        m_trigger.notify_all();
    }
}

void OtaClient::onMessage(void *payload, int16_t len) {
    if(!payload || len <= 0){
        std::cout << "[ERROR]::OtaClient::onMessage:: Message is null" << std::endl;
        return;
    }
    if(ota::OtaClientState::IDLE != m_state){
        std::cout << "ERROR::OtaClient::onMessage:: Program is uploading, do not receive other message " << std::endl;
        return;
    }

    rapidjson::Document document;
    if (document.Parse((const char *)payload).HasParseError()) {
        std::cout << "ERROR::OtaClient::onMessage:: Message is not format " << std::endl;
        return;
    }
    auto status = document.FindMember("success");
    if (!status->value.IsBool() || !status->value.GetBool()) {
        m_state = ota::OtaClientState::IDLE;
        std::cout << "OtaClient::onMessage:: No new version " << std::endl;
        return;
    }
    if(!document.HasMember("data")){
        std::cout << "[ERROR]::OtaClient::onMessage:: Ota Packet invalid " << std::endl;
        return;
    }
    auto data = document.FindMember("data");
    if(!data->value.IsObject()){
        std::cout << "[ERROR]::OtaClient::onMessage:: data of packet is not object " << std::endl;
        return;
    }

    if (!data->value.HasMember("checksum") || !data->value.HasMember("link") || !data->value.HasMember("version_name") || !data->value.HasMember("download_id") ||
        !data->value.HasMember("version_number") || !data->value.HasMember("version_min_id")) {
        std::cout << "ERROR::OtaClient::onMessage:: Ota packet invalid " << std::endl;
        return;
    }
    auto versionMin = data->value.FindMember("version_min_id");
    auto checksum = data->value.FindMember("checksum");
    auto link = data->value.FindMember("link");
    auto versionNumber = data->value.FindMember("version_number");
    auto versionName = data->value.FindMember("version_name");
    auto downloadId = data->value.FindMember("download_id");
    if (!checksum->value.IsString() || !link->value.IsString() || !versionName->value.IsString() || !versionMin->value.IsInt() ||
        !versionNumber->value.IsInt() || !downloadId->value.IsInt()) {
        std::cout << "ERROR::OtaClient::onMessage:: Ota packet invalid " << std::endl;
        return;
    }
    if(m_versionInfo.m_versionNumber < versionMin->value.GetInt()){
        std::cout << "ERROR::OtaClient::onMessage:: Version is lower minimum version" << std::endl;
        return;
    }

    ota::Session::InfoSession infoSession;
    infoSession.m_versionNumber = (int16_t)versionNumber->value.GetInt();
    infoSession.m_versionMin = (int16_t)versionMin->value.GetInt();
    infoSession.m_checksum = checksum->value.GetString();
    infoSession.m_link = link->value.GetString();
    infoSession.m_versionName = versionName->value.GetString();
    infoSession.m_downloadId = (int16_t)downloadId->value.GetInt();

    m_session = std::make_shared<ota::Session>(infoSession, PATH_DOWNLOAD_DEFAULT, shared_from_this());
    m_state = ota::OtaClientState::PREPARE_DOWNLOAD;
}

void OtaClient::notifyFromSession() {
    std::cout << "OtaClient::notifyFromSession:: Notify from session " << std::endl;
    std::lock_guard<std::mutex> lock(m_lock);
    m_trigger.notify_all();
}

void OtaClient::run() {
    while (!m_isStopping){
        switch (m_state){
            case ota::OtaClientState::INITIAL:
                processInitialState();
                break;
            case ota::OtaClientState::CONNECTING:
                processConnectingState();
                break;
            case ota::OtaClientState::IDLE:
                processIdleState();
                break;
            case ota::OtaClientState::PREPARE_DOWNLOAD:
                processPrepareDownloadState();
                break;
            case ota::OtaClientState::DOWNLOADING:
                processDownloadingState();
                break;
            case ota::OtaClientState::PREPARE_UPLOAD:
                processPrepareUploadState();
                break;
            case ota::OtaClientState::UPLOADING:
                processUploadingState();
                break;
        }
    }
}

bool OtaClient::initial(const std::string& pathToVersionInfo, const std::string& pathToConfig) {
    if(pathToVersionInfo.empty()  || pathToConfig.empty()){
        std::cout << "ERROR::OtaClient:: Path to config file is empty " << std::endl;
        return false;
    }
    m_otaPathConfig.m_pathVersionInfo = pathToVersionInfo;
    m_otaPathConfig.m_pathSystemConfig = pathToConfig;

    rapidjson::Document document;
    if(!checkVersionInfo(document)){
        std::cout << "ERROR::OtaClient::initial:: Config file invalid " << std::endl;
        return false;
    }

    m_versionInfo.m_versionName = document.FindMember("version_name")->value.GetString();
    m_versionInfo.m_versionNumber = (int16_t)document.FindMember("version_number")->value.GetInt();
    m_versionInfo.m_versionMin = (int16_t)document.FindMember("version_min")->value.GetInt();

    if(document.FindMember("updating")->value.GetBool()){
        std::cout << "OtaClient::initial:: Backup old version " << std::endl;
        backup();
    }
    if(document.FindMember("updated")->value.GetBool()){
        std::cout << "OtaClient::initial:: Update new version successfully" << std::endl;
        m_versionInfo.m_oldUpdateInfo.m_updated = true;
        if(document.HasMember("download_id")){
            m_versionInfo.m_oldUpdateInfo.m_downloadId = (std::int16_t)document.FindMember("download_id")->value.GetInt();
        }
    }

    document.RemoveAllMembers();
    if(!checkSystemConfigInfo(document)){
        std::cout << "ERROR::OtaClient::initial:: System config file invalid " << std::endl;
        return false;
    }
    m_apiInfo.m_urlBase = document.FindMember("url_base")->value.GetString();
    m_apiInfo.m_apiKey = document.FindMember("api_key")->value.GetString();
    m_apiInfo.m_pathUrlOtaUpdateStatus = document.FindMember("url_ota_update_status")->value.GetString();
    m_apiInfo.m_pathUrlOtaCheckUpdate = document.FindMember("url_ota_check_update")->value.GetString();
    m_apiInfo.m_requestCycleTime = (std::int16_t)document.FindMember("time_request")->value.GetInt();
    m_apiInfo.m_maxRandomTime = (std::int16_t)document.FindMember("random_time")->value.GetInt();

    m_httpPost.reset(new HttpPost("", shared_from_this()));
    m_httpPost->addHeader("X-Lumi-Api-Key", m_apiInfo.m_apiKey);

    m_state = ota::OtaClientState::INITIAL;
    m_session = nullptr;
    m_isStopping = false;

    return true;
}

void OtaClient::processInitialState() {
    std::cout << "OtaClient::processInitialState:: Program is initial state " << std::endl;
    reset();
    if(!initialMqttLocal() || !setupMqttLocal()){
        std::cout << "ERROR::OtaClient::onInitalState:: Do not initial Mqtt Client " << std::endl;
        reset();
        return;
    }
    std::ostringstream payload;
    payload << R"({"cmd":"startSystem", "info":{"version_name":")" << m_versionInfo.m_versionName;
    payload << R"(","version_number":)" << m_versionInfo.m_versionNumber;
    payload << R"(,"version_min":)" << m_versionInfo.m_versionMin;
    payload << R"(,"updateSuccess":)";
    if(m_versionInfo.m_oldUpdateInfo.m_updated){
        payload << R"(true)";
    } else
        payload << R"(false)";
    payload << R"(}})";

    std::cout << "OtaClient::processInitialState:: Content of message sent to AppInit: " << payload.str().c_str() << std::endl;

    m_mqttClient->publish(nullptr, (LOCAL_APPINIT_CONTROL_TOPIC).c_str(),
                          payload.str().length(), payload.str().c_str(), 0, true);

    m_state = ota::OtaClientState::CONNECTING;
}

void OtaClient::processConnectingState() {
    std::cout << "OtaClient::onSetupState:: Program is setup state " << std::endl;
    HTTPPing ping(m_apiInfo.m_urlBase);
    if(!ping.ping()){
        std::cout << "[NOTIFY]::OtaClient::processConnectingState:: Internet loss" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));
        return;
    }
    m_versionInfo.m_mac = getMacAddress();
    if(m_versionInfo.m_mac.empty()){
        std::cout << "[NOTIFY]::OtaClient::processConnectingState:: Can not get mac device" << std::endl;
        return;
    }

    m_state = ota::OtaClientState::IDLE;
}

void OtaClient::processIdleState() {
    std::cout << "OtaClient::processIdleState:: Program is IDLE state " << std::endl;
    if(m_versionInfo.m_oldUpdateInfo.m_updated){
        notifyToServer(m_versionInfo.m_oldUpdateInfo.m_downloadId, ota::OtaMessageStatus::STT_SUCCESSFULLY, ota::ErrorCode::ERR_SUCCESSFULL, "Update new firmware successfully");
        updateVersionInfo("updated", false);
        m_versionInfo.m_oldUpdateInfo.m_updated =false;
        checkNewVersion();
        return;
    }
    checkNewVersion();
    if(m_state == ota::OtaClientState::PREPARE_DOWNLOAD && m_session != nullptr){
        return;
    }
    int timeRand = std::rand() % m_apiInfo.m_maxRandomTime + 1;
    std::unique_lock<std::mutex> lock(m_lock);
    if (std::cv_status::timeout == m_trigger.wait_for(lock, std::chrono::minutes(m_apiInfo.m_requestCycleTime + timeRand))) {
        std::cout << "OtaClient::processIdleState:: Timeout Idle state " << std::endl;
    }
}

void OtaClient::processPrepareDownloadState() {
    std::cout << "OtaClient::processPrepareDownloadState:: Program is preparing download " << std::endl;
    if(!m_session){
        resetSession();
        m_state = ota::OtaClientState::IDLE;
        std::cout << "ERROR::OtaClient::onPrepareDownloadState:: Download session is NULL " << std::endl;
        return;
    }
    //TODO: pub start download message to cloud;
    notifyToServer(m_session->m_infoSession.m_downloadId, ota::OtaMessageStatus::STT_DOWNLOAD, ota::ErrorCode::ERR_SUCCESSFULL, "Start Download new firmware");

    m_session->start();
    m_state = ota::OtaClientState::DOWNLOADING;
}

void OtaClient::processDownloadingState() {
    std::cout << "OtaClient::processPrepareDownloadState:: Program is downloading!!! " << std::endl;
    if(!m_session){
        resetSession();
        m_state = ota::OtaClientState::IDLE;
        std::cout << "ERROR::OtaClient::onDownloadingState:: The session download is NULL " << std::endl;
        return;
    }
    std::unique_lock<std::mutex> lock(m_lock);
    ota::ErrorCode errorCode;
    if(m_trigger.wait_for(lock, TIMEOUT_DOWNLOADING_STATE) == std::cv_status::timeout){
        errorCode = ota::ErrorCode::ERR_TIMEOUT;
        std::cout << "ERROR::OtaClient::onDownloadingState:: Download fail " << std::endl;
    }else{
        errorCode = m_session->getErrorCode();
    }

    if(ota::ErrorCode::ERR_SUCCESSFULL == errorCode){
        m_state = ota::OtaClientState::PREPARE_UPLOAD;
        std::cout << "OtaClient::processDownloadingState:: Download new version successfully " << std::endl;
    }else{
        notifyToServer(m_session->m_infoSession.m_downloadId, ota::OtaMessageStatus::STT_ERROR, errorCode, "Download fail");
        resetSession();
        m_state = ota::OtaClientState::IDLE;
        std::cout << "ERROR::OtaClient::processDownloadingState:: Download fail " << std::endl;
    }
}

void OtaClient::processPrepareUploadState() {
    std::cout << "OtaClient::processPrepareDownloadState:: Program is preparing Upload " << std::endl;
    if(!m_session){
        resetSession();
        m_state = ota::OtaClientState::IDLE;
        std::cout << "ERROR::OtaClient::processPrepareUploadState:: The session download is NULL " << std::endl;
        return;
    }
    if (!saveCurrentVersion()) {
        notifyToServer(m_session->m_infoSession.m_downloadId, ota::OtaMessageStatus::STT_ERROR, ota::ErrorCode::ERR_INTERNAL, "Can not save current version");
        resetSession();
        m_state = ota::OtaClientState::IDLE;
        std::cout << "ERROR::OtaClient::processPrepareUploadState:: Do not save current version " << std::endl;
        return;
    }
    if (m_mqttClient) {
        std::ostringstream payload;
        payload << R"({"cmd":"resInfo", "info":"systemUpdating"})";

        m_mqttClient->publish(nullptr, LOCAL_OTACLIENT_STATUS_TOPIC.c_str(),
                              payload.str().length(), payload.str().c_str());
        std::cout << "OtaClient::processPrepareUploadState:: Send to AppInit " << std::endl;
    }else{
        reset();
        std::cout << "ERROR::OtaClient::processDownloading::: mqttClient not exist " << std::endl;
        return;
    }
    updateVersionInfo("updating", true);
    m_state = ota::OtaClientState::UPLOADING;
    std::cout << "OtaClient::processPrepareDownloadState:: The prepare upload is completed" << std::endl;
}

void OtaClient::processUploadingState() {
    std::cout << "OtaClient::processPrepareDownloadState:: Program is upload state " << std::endl;
    if(!m_session){
        resetSession();
        m_state = ota::OtaClientState::IDLE;
        std::cout << "ERROR::OtaClient::processUploadingState:: The session download is NULL " << std::endl;
        return;
    }
    ota::ErrorCode errorCode;
    m_session->notify(true);
    std::unique_lock<std::mutex> lock(m_lock);
    if (std::cv_status::timeout == m_trigger.wait_for(lock, TIMEOUT_UPLOADING_STATE)) {
        errorCode = ota::ErrorCode::ERR_TIMEOUT;
        std::cout << "ERROR::OtaClient::processUploadingState:: Uploading is timeout " << std::endl;
    } else {
        errorCode = m_session->getErrorCode();
    }
    lock.unlock();
    if (ota::ErrorCode::ERR_SUCCESSFULL == errorCode) {
        updateVersionInfo(m_session->m_infoSession.m_versionName, m_session->m_infoSession.m_versionNumber, m_session->m_infoSession.m_versionMin,
                          m_session->m_infoSession.m_downloadId, false, true);
        //notifyToServer(m_session->m_infoSession.m_downloadId, ota::OtaMessageStatus::STT_SUCCESSFULLY, errorCode, "Update new firmware successfully");
        //std::this_thread::sleep_for(std::chrono::seconds(2));
        //m_state = ota::OtaClientState::IDLE;
        std::cout << "OtaClient::processUploadingState:: Update new firmware is successfully, System reboot !!! " << std::endl;
        system("reboot");
    }else{
        notifyToServer(m_session->m_infoSession.m_downloadId, ota::OtaMessageStatus::STT_ERROR, errorCode, "Upload fail");
        resetSession();
        updateVersionInfo("updating", false);
        m_state = ota::OtaClientState::IDLE;
    }
}

std::string OtaClient::getMacAddress() {
    std::string mac;
    std::ifstream macAddress("/sys/class/net/wlan0/address");

    getline(macAddress, mac);
    macAddress.close();

    return mac;

//   return "70:2c:1f:32:da:66";
}

void OtaClient::checkNewVersion() {
    if (!m_mqttClient || m_versionInfo.m_versionName.empty()) {
        reset();
        std::cout << "ERROR::OtaClient::checkNewVersion:: Mqtt Client is NULL or The version parameter is empty " << std::endl;
        return;
    }
    std::ostringstream payload;
    payload << R"({"mac":")" <<   m_versionInfo.m_mac;
    payload << R"(", "version_number":)" << m_versionInfo.m_versionNumber;
    payload << R"(,"version_name":")" << m_versionInfo.m_versionName.c_str();
    payload << R"(","version_min_id":)" << m_versionInfo.m_versionMin;
    payload << R"(, "info_firmware":""})";

    std::cout << "Message: " << payload.str().c_str() << std::endl;

    m_httpPost->postToServer(m_apiInfo.m_urlBase + m_apiInfo.m_pathUrlOtaCheckUpdate, payload.str().c_str());
}

void OtaClient::notifyToServer(const std::int16_t downloadId, ota::OtaMessageStatus status, ota::ErrorCode code, const std::string& errorMessage) {
    if(!m_httpPost && m_versionInfo.m_mac.empty()){
        reset();
        std::cout << "ERROR::OtaClient::notifyToServer:: Mqtt Client or Version or App type is empty " << std::endl;
        return;
    }
    std::ostringstream payload;
    payload << R"({"mac":")" << m_versionInfo.m_mac;
    payload << R"(","download_id":)"  << downloadId;

    time_t rawTime;
    struct tm * timeInfo;
    time(&rawTime);
    timeInfo = localtime(&rawTime);
    std::string dateTime = std::to_string(1900 + timeInfo->tm_year) + "-";
    if(timeInfo->tm_mon < 9)
        dateTime += "0";
    dateTime += std::to_string(1 + timeInfo->tm_mon) + "-";
    if(timeInfo->tm_mday < 10)
        dateTime += "0";
    dateTime += std::to_string(timeInfo->tm_mday) + "T";
    if(timeInfo->tm_hour < 10)
        dateTime += "0";
    dateTime += std::to_string(timeInfo->tm_hour) + ":";
    if(timeInfo->tm_min < 10)
        dateTime += "0";
    dateTime += std::to_string(timeInfo->tm_min) + ":";
    if(timeInfo->tm_sec < 10)
        dateTime += "0";
    dateTime += std::to_string(timeInfo->tm_sec) + ".0";

    payload << R"(,"datetime":")"  << dateTime;
    payload << R"(","status":)" << (int)status;
    payload << R"(,"errorCode":)" << (int)code;
    payload << R"(,"errorMessage":")" << errorMessage;
    payload << R"("})";

    std::cout << "Message send to Cloud: " << payload.str().c_str() << std::endl;

    m_httpPost->postToServer(m_apiInfo.m_urlBase + m_apiInfo.m_pathUrlOtaUpdateStatus, payload.str().c_str());
}

bool OtaClient::saveCurrentVersion() {
    std::cout << "OtaClient::saveCurrentVersion:: Saving current version......................." << std::endl;
    std::string nameFileBackup = "firmware_" + std::to_string(m_versionInfo.m_versionNumber);

    std::string oldFirmwareName = ota::FileWraper::compress(PATH_EXECUTED_DEFAULT + nameFileBackup, PATH_EXECUTED_DEFAULT, "milo");
    if(oldFirmwareName.empty()){
        std::cout << "ERROR::OtaClient::saveCurrentVersion: Can not compress current version " << std::endl;
        return  false;
    }
    if(!ota::FileWraper::removeAll(PATH_BACKUP_DEFAULT)){
        std::cout << "ERROR::OtaClient::saveCurrentVersion: Can not remove old firmware " << std::endl;
        return  false;
    }
    if(!ota::FileWraper::move(oldFirmwareName, PATH_BACKUP_DEFAULT)){
        std::cout << "ERROR::OtaClient::saveCurrentVersion: Can not copy old firmware " << std::endl;
        return  false;
    }
    std::cout << "OtaClient::saveCurrentVersion: Save current version successfully " << std::endl;
    return true;
}

void OtaClient::backup() {
    std::cout << "OtaClient::backup::the error when update new firmware, backup old version " << std::endl;
    std::vector<std::string> listFiles;
    ota::FileWraper::listFileInDirectory(PATH_BACKUP_DEFAULT, listFiles);
    for(int index = 0; index < listFiles.size(); index++){
        auto file = listFiles.at(index);
        if((file.find("firmware") != std::string::npos) && (!ota::FileWraper::isFileEmpty(PATH_BACKUP_DEFAULT + file))){
            if(ota::FileWraper::extract(PATH_BACKUP_DEFAULT + file, PATH_EXECUTED_DEFAULT)){
                std::cout << "OtaClient::backup::Backup old version successfully " << std::endl;
                system("reboot");
            }
        }
    }
    std::cout << "ERROR::OtaClient::backup:: Backup not successfully " << std::endl;
}

bool OtaClient::checkVersionInfo(rapidjson::Document &document) {
    FILE* configFile = fopen((m_otaPathConfig.m_pathVersionInfo + "ota_version_info.json").c_str(), "r");
    if(!configFile){
        std::cout << "ERROR::OtaClient::checkConfigFile:: Can not open Config file " << std::endl;
        return false;
    }
    fseek(configFile, 0L, SEEK_END);
    size_t total = ftell(configFile);
    fseek(configFile, 0L, SEEK_SET);
    char buffer[total + 1] = {0};
    size_t len = fread(buffer, 1, total, configFile);
    fclose(configFile);
    if (len <= 0) {
        std::cout << "ERROR:: OtaClient::checkConfigFile:: Config File is empty " << std::endl;
        return false;
    }

    if(document.Parse(buffer).HasParseError()){
        std::cout << "ERROR:: OtaClient::checkConfigFile:: Config File invalid " << std::endl;
        return false;
    }

    if(!document.HasMember("updating") || !document.HasMember("updated") || !document.HasMember("version_number") ||
       !document.HasMember("version_name") || !document.HasMember("version_min")){
        std::cout << "ERROR:: OtaClient::checkConfigFile:: Ota version info file not enough parameter " << std::endl;
        return false;
    }
    return true;
}

bool OtaClient::checkSystemConfigInfo(rapidjson::Document &document) {
    FILE* configFile = fopen((m_otaPathConfig.m_pathSystemConfig + "ota_config.json").c_str(), "r");
    if(!configFile){
        std::cout << "ERROR::OtaClient::checkConfigFile:: Can not open Config file " << std::endl;
        return false;
    }
    fseek(configFile, 0L, SEEK_END);
    size_t total = ftell(configFile);
    fseek(configFile, 0L, SEEK_SET);
    char buffer[total + 1] = {0};
    size_t len = fread(buffer, 1, total, configFile);
    fclose(configFile);
    if (len <= 0) {
        std::cout << "ERROR:: OtaClient::checkSystemConfigInfo:: Ota config file is empty " << std::endl;
        return false;
    }

    if(document.Parse(buffer).HasParseError()){
        std::cout << "ERROR:: OtaClient::checkSystemConfigInfo:: Ota config file invalid " << std::endl;
        return false;
    }
    if(!document.HasMember("url_base") || !document.HasMember("api_key") || !document.HasMember("url_ota_check_update")
            || !document.HasMember("url_ota_update_status") || ! document.HasMember("time_request") || !document.HasMember("random_time")){
        std::cout << "ERROR:: OtaClient::checkSystemConfigInfo:: Ota config File not enough parameter " << std::endl;
        return false;
    }

    return true;
}

void OtaClient::updateVersionInfo(const std::string &name, bool value) {
    std::lock_guard<std::mutex> lock(m_lock);
    rapidjson::Document document;
    if(!checkVersionInfo(document)){
        std::cout << "ERROR::OtaClient::updateVersionInfo:: Version info file invalid " << std::endl;
        return;
    }
    if(document.HasMember(name.c_str())){
        document.EraseMember(name.c_str());
    }
    document.AddMember(rapidjson::StringRef(name.c_str()), value, document.GetAllocator());

    rapidjson::StringBuffer buf;
    buf.Clear();
    rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
    document.Accept(writer);
    std::cout << "OtaClient::updateVersionInfo:: Content of new version info file : " << buf.GetString() << std::endl;

    std::string pathToConfigFile(m_otaPathConfig.m_pathVersionInfo);
    FILE* configFile = fopen(pathToConfigFile.append("ota_version_info.json").c_str(), "w+");
    if(!configFile){
        std::cout << "ERROR::OtaClient::updateVersionInfo:: Version info  file not exist " << std::endl;
        ferror(configFile);
    }
    if(buf.GetSize() != fwrite(buf.GetString(), 1, buf.GetSize(), configFile)){
        std::cout << "ERROR::OtaClient::updateVersionInfo:: Cannot write file Version info " << std::endl;
        ferror(configFile);
    }
    fclose(configFile);
}

void OtaClient::updateVersionInfo(const std::string &versionName, const std::int16_t versionNumber, const std::int16_t versionMin,
                                  const std::int16_t downloadId, const bool updating, const bool updated) {
    std::lock_guard<std::mutex> lock(m_lock);
    rapidjson::Document document;
    if(!checkVersionInfo(document)){
        std::cout << "ERROR::OtaClient::updateVersionInfo:: Version info  file invalid " << std::endl;
        return;
    }

    document.RemoveAllMembers();
    document.AddMember("version_name", rapidjson::StringRef(versionName.c_str()), document.GetAllocator());
    document.AddMember("version_number", versionNumber, document.GetAllocator());
    document.AddMember("version_min", versionMin, document.GetAllocator());
    document.AddMember("updating", updating, document.GetAllocator());
    document.AddMember("updated", updated, document.GetAllocator());
    document.AddMember("download_id", downloadId, document.GetAllocator());

    rapidjson::StringBuffer buf;
    buf.Clear();
    rapidjson::Writer<rapidjson::StringBuffer> writer(buf);
    document.Accept(writer);
    std::cout << "OtaClient::updateVersionInfo:: Content of new version info file : " << buf.GetString() << std::endl;

    std::string pathToConfigFile(m_otaPathConfig.m_pathVersionInfo);
    FILE* configFile = fopen(pathToConfigFile.append("ota_version_info.json").c_str(), "w+");
    if(!configFile){
        std::cout << "ERROR::OtaClient::updateVersionInfo:: Version info  file not exist " << std::endl;
        ferror(configFile);
    }
    if(buf.GetSize() != fwrite(buf.GetString(), 1, buf.GetSize(), configFile)){
        std::cout << "ERROR::OtaClient::updateVersionInfo:: Cannot write file Version info " << std::endl;
        ferror(configFile);
    }
    fclose(configFile);
}

bool OtaClient::initialMqttLocal() {
    if(m_mqttClient){
        m_isMqttClientStopping = true;
        if(m_mqttLocalThread.joinable())
            m_mqttLocalThread.join();
        m_mqttClient.reset();
    }

    m_mqttClient = ota::MQTTClient::create(shared_from_this(), "otaCommLocal" + std::to_string(std::rand()));
    if(!m_mqttClient){
        std::cout << "ERROR:: OtaClient::initMqttlocal:: Do not create Mqtt local client\n";
        return false;
    }
    m_isMqttClientStopping = false;

    std::unique_lock<std::mutex> lock(m_lock);
    m_mqttLocalThread = std::thread(&OtaClient::mqttLocalLoop, this);

    if(m_trigger.wait_for(lock, TIMEOUT_CONNECT_TO_BROKER) == std::cv_status::timeout) {
        m_mqttClient = nullptr;
        std::cout << "ERROR::OtaClient::initMqttLocal:: Timeout connected to Mqtt Local Broker\n";
        return false;
    }
    std::cout << "OtaClient::initMqttLocal:: Mqtt Local Broker connected\n";
    return true;
}

bool OtaClient::setupMqttLocal() {
    if(!m_mqttClient){
        std::cout << "ERROR::BridgeComm::setupMqttLocal:: Mqtt Client To Local is NULL " << std::endl;
        return false;
    }
    std::unique_lock<std::mutex> lock(m_lock);

    /*m_mqttClient->subscribe(nullptr, (CONTROL_TOPIC).c_str());
    if(m_trigger.wait_for(lock, TIMEOUT_SUBSCRIBE_TO_BROKER) == std::cv_status::timeout){
        m_mqttClient = nullptr;
        std::cout << "ERROR::OtaClient::setupMqttLocal:: Timeout subscribe to Mqtt Local Broker " << CONTROL_TOPIC << std::endl;
        return false;
    }
    std::cout << "OtaClient::setupMqttLocal:: subscribe to Mqtt Local Broker successfully " << CONTROL_TOPIC << std::endl;*/

    m_mqttClient->subscribe(nullptr, (CLOUD_CONTROL_TOPIC).c_str());
    if(m_trigger.wait_for(lock, TIMEOUT_SUBSCRIBE_TO_BROKER) == std::cv_status::timeout){
        m_mqttClient = nullptr;
        std::cout << "ERROR::OtaClient::setupMqttLocal:: Timeout subscribe to Mqtt Local Broker " << CLOUD_CONTROL_TOPIC << std::endl;
        return false;
    }
    std::cout << "OtaClient::setupMqttLocal:: subscribe to Mqtt Local Broker successfully " << CLOUD_CONTROL_TOPIC << std::endl;

    std::cout << "OtaClient::setupMqttLocal:: Setup Mqtt Local Client Successfully " << std::endl;
    return true;
}

void OtaClient::reset() {
    m_isMqttClientStopping = true;
    if(m_mqttLocalThread.joinable())
        m_mqttLocalThread.join();
    resetSession();
    m_state = ota::OtaClientState::INITIAL;
    std::cout << "OtaClient::reset::Reset program " << std::endl;
}

void OtaClient::resetSession(){
    if(m_session) {
        m_session->notify(false);
    }
    m_session.reset();
    std::cout << "OtaClient::resetSession:: Destroy session " << std::endl;
}

void OtaClient::mqttLocalLoop() {
    while(!m_isMqttClientStopping){
        if(m_mqttClient && m_mqttClient->loop()){
            std::lock_guard<std::mutex> lock(m_lock);
            m_isMqttClientStopping = true;
            std::cout << "ERROR::OtaClient::mqttLocalLoop:: Exit Mqtt loop " << std::endl;
        }
    }
}

}
