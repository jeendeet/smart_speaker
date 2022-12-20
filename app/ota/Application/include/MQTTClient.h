//
// Created by phuclm on 4/28/18.
//

#ifndef MQTTCLIENT_H
#define MQTTCLIENT_H

#include <memory>
#include <thread>
#include <string>
#include <mosquittopp.h>

#include "MQTTClientObserverInterface.h"

namespace ota {

class MQTTClient
    : public mosqpp::mosquittopp {
public:
    ///
    static std::shared_ptr<MQTTClient> create (
        std::shared_ptr<MQTTClientObserverInterface> observer,
        const std::string& clientId,
        const std::string& host = "127.0.0.1",
        const std::int16_t port = 1883,
        const std::string& username = "vnbk",
        const std::string& password = "vnbk");
    ///
    void on_connect(int rc) override ;
    void on_message(const struct mosquitto_message *message) override ;
    void on_subscribe(int messageId, int qos_count, const int *granted_qos) override ;
    void on_publish(int messageId);

	void on_log(int level, const char *str) override ;
	void on_error() override ;

	std::string getClientId();
	bool isConnected();
private:
    ///
    MQTTClient(const std::string& clientId,
	            const std::string& host,
	            const std::int16_t port,
				const std::string& username,
				const std::string& password,
                const std::shared_ptr<MQTTClientObserverInterface>& observer);
    ///
    bool initialize();
    ///
    std::string m_cliendId;
    std::string m_host;
    std::int16_t m_port;
    std::string m_username;
    std::string m_password;

    bool m_isConnected;

    std::shared_ptr<ota::MQTTClientObserverInterface> m_observer;
};
}

#endif //ALEXACLIENTSDK_MQTTCLIENT_H
