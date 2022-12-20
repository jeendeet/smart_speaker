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
#include "app_mqtt.h"
#include "app_avk.h"
#include "app_utils.h"
#include "app_xml_utils.h"

#define SUBSCRIBE_ID                        "subscribe-app-bluetooth-audio"
#define PUBLIC_ID                           "publisher-app-bluetooth-audio"
#define MQTT_BROKER_IP_ADDRESS              "localhost"
#define MQTT_BLE_TOPIC                      "bluetooth/audio-control"
#define MOSQUITTO_PORT                      1883

#define APP_AVK_MENU_PLAY_START             "START-STREAM-PLAY"
#define APP_AVK_MENU_PLAY_STOP              "STOP-STREAM-PLAY"
#define APP_AVK_MENU_PLAY_START_FOCUS       "START-STREAM-FOCUS"
#define APP_AVK_MENU_PLAY_STOP_FOCUS        "STOP-STREAM-FOCUS"
#define APP_AVK_MENU_PLAY_PAUSE             "PAUSE-STREAM-PLAY"
#define APP_AVK_MENU_PLAY_NEXT_TRACK        "PLAY-NEXT-TRACK"
#define APP_AVK_MENU_PLAY_PREVIOUS_TRACK    "PLAY-PREVIOUS-TRACK"
#define APP_AVK_MENU_PAIR_TO_DEVICE         "PAIR-TO-DEVICE"
#define APP_AVK_MENU_UNPAIR_TO_DEVICE       "UNPAIR-TO-DEVICE"
#define APP_AVK_TURN_OFF_VOLUME             "TURN-OFF-VOLUME"
#define APP_AVK_TURN_ON_VOLUME              "TURN-ON-VOLUME"

BOOLEAN m_bluetooth_focus = FALSE;
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
void app_audio_on_connect(struct mosquitto *mosq, void *obj, int rc) 
{
	APP_DEBUG1("ID: %d", * (int *) obj);
	if(rc) {
		APP_DEBUG1("Error with result code: %d", rc);
		exit(-1);
	}
	mosquitto_subscribe(mosq, NULL, MQTT_BLE_TOPIC, 0);
}

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
void app_audio_on_message(struct mosquitto *mosq, void *obj, const struct mosquitto_message *msg) 
{
	APP_DEBUG1("New message with topic %s", msg->topic);
    APP_DEBUG1("Payload %s", (char *) msg->payload);
    int connection_index = 0;
    for (int i=0; i<APP_NUM_ELEMENTS(app_xml_remote_devices_db); i++)
    {
        if (app_xml_remote_devices_db[i].in_use != FALSE)
        {
            connection_index = i;
        }
    }
    
    tAPP_AVK_CONNECTION *connection = NULL;
    if (strcmp((char *) msg->payload, APP_AVK_MENU_PLAY_START) == 0)
    {
        /* Example to start stream play */
        APP_DEBUG0("Choose connection index");
        app_avk_display_connections();
        m_bluetooth_focus = FALSE;
        connection = app_avk_find_connection_by_index(connection_index);
        if(connection)
            app_avk_play_start(connection->rc_handle);
        else
            APP_DEBUG1("Unknown choice:%d", connection_index);
    }
    else if (strcmp((char *) msg->payload, APP_AVK_MENU_PLAY_STOP) == 0)
    {
        /* Example to stop stream play */
        APP_DEBUG0("Choose connection index");
        app_avk_display_connections();
        m_bluetooth_focus = FALSE;
        connection = app_avk_find_connection_by_index(connection_index);
        if(connection)
            app_avk_play_stop(connection->rc_handle);
        else
            APP_DEBUG1("Unknown choice:%d", connection_index);
    }
    else if (strcmp((char *) msg->payload, APP_AVK_MENU_PLAY_START_FOCUS) == 0)
    {
        /* Example to start stream play */
        APP_DEBUG0("Choose connection index");
        APP_DEBUG0("AAAAAAAAAAAAAAAAAAAAAAAAA APP_AVK_MENU_PLAY_START_FOCUS AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
        app_avk_display_connections();
        m_bluetooth_focus = TRUE;
        connection = app_avk_find_connection_by_index(connection_index);
        if(connection)
            app_avk_play_start(connection->rc_handle);
        else
            APP_DEBUG1("Unknown choice:%d", connection_index);
    }
    else if (strcmp((char *) msg->payload, APP_AVK_MENU_PLAY_STOP_FOCUS) == 0)
    {
        /* Example to stop stream play */
        APP_DEBUG0("Choose connection index");
        APP_DEBUG0("BBBBBBBBBBBBBBBBBBBBBBBBBBB APP_AVK_MENU_PLAY_STOP_FOCUS BBBBBBBBBBBBBBBBBBBBBBBBBBBB");
        app_avk_display_connections();
        m_bluetooth_focus = TRUE;
        connection = app_avk_find_connection_by_index(connection_index);
        if(connection)
            app_avk_play_stop(connection->rc_handle);
        else
            APP_DEBUG1("Unknown choice:%d", connection_index);
    }
    else if (strcmp((char *) msg->payload, APP_AVK_MENU_PLAY_PAUSE) == 0)
    {
        /* Example to pause stream play */
        APP_DEBUG0("Choose connection index");
        app_avk_display_connections();
        connection = app_avk_find_connection_by_index(connection_index);
        if(connection)
            app_avk_play_pause(connection->rc_handle);
        else
            APP_DEBUG1("Unknown choice:%d", connection_index);
    }
    else if (strcmp((char *) msg->payload, APP_AVK_MENU_PLAY_NEXT_TRACK) == 0)
    {
        /* Example to play next track */
        APP_DEBUG0("Choose connection index");
        app_avk_display_connections();
        connection = app_avk_find_connection_by_index(connection_index);
        if(connection)
            app_avk_play_next_track(connection->rc_handle);
        else
            APP_DEBUG1("Unknown choice:%d", connection_index);
    }
    else if (strcmp((char *) msg->payload, APP_AVK_MENU_PLAY_PREVIOUS_TRACK) == 0)
    {
        /* Example to play previous track */
        APP_DEBUG0("Choose connection index");
        app_avk_display_connections();
        connection = app_avk_find_connection_by_index(connection_index);
        if(connection)
            app_avk_play_previous_track(connection->rc_handle);
        else
            APP_DEBUG1("Unknown choice:%d", connection_index);
    }
    else if (strcmp((char *) msg->payload, APP_AVK_MENU_PAIR_TO_DEVICE) == 0)
    {
        /* Example to pair to device */
        APP_DEBUG0("Pair to device");
        app_avk_open();
    }
    else if (strcmp((char *) msg->payload, APP_AVK_MENU_UNPAIR_TO_DEVICE) == 0)
    {
        /* Example to unpair to device */
        APP_DEBUG0("Unpair to device");
        connection = app_avk_find_connection_by_index(connection_index);
        printf("    - bdaddr: %02x:%02x:%02x:%02x:%02x:%02x\n",
                connection->bda_connected[0], connection->bda_connected[1],
                connection->bda_connected[2], connection->bda_connected[3],
                connection->bda_connected[4], connection->bda_connected[5]);
        if(connection)
            app_avk_close(connection->bda_connected);
    }
    else if (strcmp((char *) msg->payload, APP_AVK_TURN_OFF_VOLUME) == 0)
    {
        /* Example to turn off volumn */
        APP_DEBUG0("Turn off volume");
        UINT8 volume = 0x00;
        app_avk_display_connections();
        connection = app_avk_find_connection_by_index(connection_index);
        if(connection)
            app_avk_set_abs_vol_rsp(volume, connection->rc_handle, connection->volChangeLabel);
        else
            APP_DEBUG1("Unknown choice:%d", connection_index);
    }
    else if (strcmp((char *) msg->payload, APP_AVK_TURN_ON_VOLUME) == 0)
    {
        /* Example to turn off volumn */
        APP_DEBUG0("Turn on volume");
        UINT8 volume = (UINT8)((BSA_MAX_ABS_VOLUME - BSA_MIN_ABS_VOLUME)>>1);
        app_avk_display_connections();
        connection = app_avk_find_connection_by_index(connection_index);
        if(connection)
            app_avk_set_abs_vol_rsp(volume, connection->rc_handle, connection->volChangeLabel);
        else
            APP_DEBUG1("Unknown choice:%d", connection_index);
    }
    else 
    {
        APP_DEBUG0("No key available");
    }
}

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
void app_audio_server_mqtt_init(struct mosquitto *m_mosq)
{
    /* Mosquitto init */
    int rc, id=12;
    /* Mosquitto init */
    mosquitto_lib_init();
    m_mosq = mosquitto_new(SUBSCRIBE_ID, true, &id);
    mosquitto_connect_callback_set(m_mosq, app_audio_on_connect);
    mosquitto_message_callback_set(m_mosq, app_audio_on_message);
    rc = mosquitto_connect(m_mosq, MQTT_BROKER_IP_ADDRESS, MOSQUITTO_PORT, 10);
    if(rc) {
        APP_DEBUG0("Could not connect to Broker with return code");
	}
    mosquitto_loop_start(m_mosq);
}

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
void app_audio_server_mqtt_shut_down(struct mosquitto *m_mosq)
{
    /* Mosquitto server shutdown */
    mosquitto_loop_stop(m_mosq, true);

	mosquitto_disconnect(m_mosq);
	mosquitto_destroy(m_mosq);
	mosquitto_lib_cleanup();
}

/*******************************************************************************
 **
 ** Function        app_audio_client_mqtt
 **
 ** Description     MQTT client from MQTT service
 **
 ** Parameters      ...
 **
 ** Returns         None
 **
 *******************************************************************************/
void app_audio_client_mqtt(const char *topic, char *payload)
{
    int rc;
    struct mosquitto *mosq;

    mosquitto_lib_init();

    mosq = mosquitto_new(PUBLIC_ID, true, NULL);

    rc = mosquitto_connect(mosq, MQTT_BROKER_IP_ADDRESS, 1883, 60);
    if(rc != 0){
        APP_DEBUG0("Client could not connect to broker!");
        mosquitto_destroy(mosq);
    }
    mosquitto_publish(mosq, NULL, topic, strlen(payload), payload, 0, false);
    mosquitto_disconnect(mosq);
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
}

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
void app_audio_create_json_str(char* payload, char* key, char* value)
{
    if (strlen(payload) == 0) {
        strcat(payload, (char*)"{\"");
        strcat(payload, key);
        strcat(payload, (char*)"\":\"");
        strcat(payload, value);
        strcat(payload, (char*)"\"}");
    }
    else
    {
        payload[strlen(payload)-1] = '\0';
        strcat(payload, (char*)",");
        strcat(payload, (char*)"\"");
        strcat(payload, key);
        strcat(payload, (char*)"\":\"");
        strcat(payload, value);
        strcat(payload, (char*)"\"}");
    }
}

/*******************************************************************************
 **
 ** Function        app_audio_get_bluetooth_focus
 **
 ** Description     Get bluetooth focus
 **
 ** Parameters      ...
 **
 ** Returns         bluetooth focus
 **
 *******************************************************************************/
BOOLEAN app_audio_get_bluetooth_focus_avs()
{
    return m_bluetooth_focus;
}

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
void app_audio_set_bluetooth_focus_avs(BOOLEAN bluetooth_focus)
{
    m_bluetooth_focus = bluetooth_focus;
}
