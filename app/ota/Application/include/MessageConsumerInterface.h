//
// Created by vnbk on 27/10/2018.
//

#ifndef OTA_HC_V1_MESSAGECONSUMERINTERFACE_H
#define OTA_HC_V1_MESSAGECONSUMERINTERFACE_H

namespace ota{
class MessageConsumerInterface{
public:
    virtual ~MessageConsumerInterface() = default;

    virtual void onMessage(void* data, int16_t len) = 0;
};
}

#endif //OTA_HC_V1_MESSAGECONSUMERINTERFACE_H
