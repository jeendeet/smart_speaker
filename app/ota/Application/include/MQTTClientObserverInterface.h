//
// Created by phuclm on 4/28/18.
//

#ifndef MQTTCLIENTOBSERVERINTERFACE_H
#define MQTTCLIENTOBSERVERINTERFACE_H

#include <mosquittopp.h>

namespace ota {

class MQTTClientObserverInterface {
public:
    ///
    virtual ~MQTTClientObserverInterface() = default;
    ///
    virtual void onMqttConnect(std::string clientId) = 0;
    ///
    virtual void onMqttMessage(std::string clientId, const struct mosquitto_message *message) = 0;
    ///
    virtual void onMqttSubscribe(std::string clientId, int mid, int qos_count, const int *granted_qos) = 0;

    virtual void onMqttPublish(std::string clientId, int messageId) = 0;
};

}

#endif //ALEXACLIENTSDK_MQTTCLIENTOBSERVERINTERFACE_H
