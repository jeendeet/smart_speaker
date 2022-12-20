/*****************************************************************************
 **
 **  Name:           app_mqtt.c
 **
 **  Description:    MQTT functions support bluetooth
 **
 **  Copyright (c) 2022, VinBigdata., All Rights Reserved.
 **  VinBigData Core. Proprietary and confidential.
 **
 *****************************************************************************/

/* idempotency */
#ifndef APP_MQTT_H
#define APP_MQTT_H

#include <stdio.h>
#include <stdlib.h>
#include <mosquitto.h>

#include "bsa_api.h"
#include "bsa_avk_api.h"
#include "app_utils.h"

/*******************************************************************************
 **
 ** Function        app_audio_on_connect
 **
 ** Description     Connect to MQTT service
 **
 ** Parameters      ...
 **
 ** Returns         None
 **
 *******************************************************************************/
void app_audio_on_connect(struct mosquitto *mosq, void *obj, int rc);

/*******************************************************************************
 **
 ** Function        app_audio_on_message
 **
 ** Description     Receice message from MQTT service
 **
 ** Parameters      ...
 **
 ** Returns         None
 **
 *******************************************************************************/
void app_audio_on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg);

/*******************************************************************************
 **
 ** Function        app_audio_server_mqtt_init
 **
 ** Description     Init MQTT service
 **
 ** Parameters      ...
 **
 ** Returns         None
 **
 *******************************************************************************/
void app_audio_server_mqtt_init(struct mosquitto *m_mosq);

/*******************************************************************************
 **
 ** Function        app_audio_server_mqtt_shut_down
 **
 ** Description     Shut down from MQTT service
 **
 ** Parameters      ...
 **
 ** Returns         None
 **
 *******************************************************************************/
void app_audio_server_mqtt_shut_down(struct mosquitto *m_mosq);

/*******************************************************************************
 **
 ** Function        app_audio_client_mqtt
 **
 ** Description     MQTT clienr from MQTT service
 **
 ** Parameters      ...
 **
 ** Returns         None
 **
 *******************************************************************************/
void app_audio_client_mqtt(const char *topic, char *payload);

/*******************************************************************************
 **
 ** Function        app_audio_create_json_str
 **
 ** Description     Create json string
 **
 ** Parameters      ...
 **
 ** Returns         None
 **
 *******************************************************************************/
void app_audio_create_json_str(char* payload, char* key, char* value);

/*******************************************************************************
 **
 ** Function        app_audio_get_bluetooth_focus_avs
 **
 ** Description     Get bluetooth focus avs
 **
 ** Parameters      ...
 **
 ** Returns         bluetooth focus avs
 **
 *******************************************************************************/
BOOLEAN app_audio_get_bluetooth_focus_avs();

/*******************************************************************************
 **
 ** Function        app_audio_set_bluetooth_focus
 **
 ** Description     Set bluetooth focus
 **
 ** Parameters      ...
 **
 ** Returns         bluetooth focus
 **
 *******************************************************************************/
void app_audio_set_bluetooth_focus_avs(BOOLEAN bluetooth_focus);


#endif /* APP_MQTT_H_ */
