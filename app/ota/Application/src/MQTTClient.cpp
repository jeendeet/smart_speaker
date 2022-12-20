//
// Created by phuclm on 4/28/18.
//
#include <string>
#include <iostream>

#include "../include/MQTTClient.h"

namespace ota {

static const std::string USER_NAME{"thuannq"};

static const std::string PASS_WORD{"vnbk"};

std::shared_ptr<MQTTClient> MQTTClient::create(
    std::shared_ptr<MQTTClientObserverInterface> observer,
    const std::string& clientId,
    const std::string& host,
    const std::int16_t port,
	const std::string& username,
	const std::string& password) {
    if (!observer) {
        return nullptr;
    }

    auto mqttClient = std::shared_ptr<MQTTClient>(new MQTTClient(clientId, host, port, username, password, observer));

    if (!mqttClient->initialize()) {
        return nullptr;
    }

    return mqttClient;
}

MQTTClient::MQTTClient(const std::string& clientId,
                       const std::string& host,
                       const std::int16_t port,
                       const std::string& username,
                       const std::string& password,
                       const std::shared_ptr<MQTTClientObserverInterface>& observer) :
        mosqpp::mosquittopp{clientId.c_str()},
        m_cliendId{clientId},
        m_host{host},
        m_port{port},
        m_username{username},
        m_password{password},
        m_observer{observer},
        m_isConnected{false}{
}

std::string MQTTClient::getClientId() {
    return m_cliendId;
}

bool MQTTClient::isConnected() {
    return m_isConnected;
}


void MQTTClient::on_connect(int rc) {
    if(!rc){
        m_isConnected = true;
        m_observer->onMqttConnect(m_cliendId);
    }
}

void MQTTClient::on_subscribe(int messageId, int qos_count, const int *granted_qos) {
    m_observer->onMqttSubscribe(m_cliendId, messageId, qos_count, granted_qos);
}

void MQTTClient::on_message(const struct mosquitto_message *message) {
    m_observer->onMqttMessage(m_cliendId, message);
}

void MQTTClient::on_publish(int messageId) {
    m_observer->onMqttPublish(m_cliendId, messageId);
}

void MQTTClient::on_error() {
    std::cout << "[ERROR_MOSQUITO]:: " << std::endl;
}

void MQTTClient::on_log(int level, const char *str) {
   // std::cout << "[LOG_MOSQUITO]:: Level: " << level << " ; " << str << std::endl;
}

bool MQTTClient::initialize() {
    mosqpp::lib_init();

    int keepAlive = 60;
    username_pw_set(m_username.c_str(), m_password.c_str());
    connect(m_host.c_str(), m_port, keepAlive);

    return true;
}

}