/*****************************************************************************
**
**  Name:           app_handle.c
**
**  Description:    Bluetooth BLE Handle application
**
**  Copyright (c) 2022, Vinbigdata., All Rights Reserved.
**  Proprietary and confidential.
**
*****************************************************************************/
#include "app_utils.h"
#include "app_handle.h"
#include "app_curl_request.h"
#include <mosquitto.h>

/*
 * Global Variables
 */
/* commands */
#define CMD_BAV_CP_WIFI_CONNECT_REQUEST     0x01
#define CMD_BAV_CP_WIFI_SCANNER             0x02
#define CMD_BAV_CP_METADATA_REQUEST         0x03
#define CMD_BAV_CP_IPADRESS_REQUEST         0x04
#define CMD_BAV_CP_AUTHORIZATION_REQUEST    0x05

#define CMD_RES_DATA_MAX                    20
#define STR_DATA_MAX                        50
#define STR_API_MAX                         150
#define STR_AU_CODE_MAX                     400

/* mqtt define */
#define AUTHORIZATION_TOPIC                 "metadata/authorization"
#define IP_ADDRESS_MQTT                     "localhost"
#define MOSQUITTO_PORT                      1883

UINT16 g_metadata_request = 0, g_ipaddress_request = 0, g_wifi_scanner = 0, g_wifi_connect_request = 0;
char *g_metadata_res, *g_ipaddress_res, *g_wifi_scanner_res, *g_wifi_connect_res;
char api_connect_wifi[STR_API_MAX], ssid[STR_DATA_MAX], psk[STR_DATA_MAX];
char authorization_code[STR_AU_CODE_MAX];

/*******************************************************************************
**
** Function         app_ble_connected_app_api_registration
**
** Description      CONNECTED APP Server API registration
**
** Returns          void
**
*******************************************************************************/
void app_ble_connected_app_api_registration (tBSA_BLE_MSG *p_data)
{
    long http_code = 0;
    struct _responseString res_s;
    switch (*(p_data->ser_write.value + 1))
    {
        case CMD_BAV_CP_WIFI_CONNECT_REQUEST:
            if (*(p_data->ser_write.value + 2) == 0x10) /* request */
            {
                APP_DEBUG1("registration CMD_BAV_CP_WIFI_CONNECT_REQUEST handle: %d", p_data->ser_write.handle);
                strcpy(api_connect_wifi, "wifi-connect-request?ssid=");
                strcat(api_connect_wifi, ssid);
                strcat(api_connect_wifi, "&psk=");
                strcat(api_connect_wifi, psk);

                g_wifi_connect_request = p_data->ser_write.handle;
                res_s = app_ble_curl_request(api_connect_wifi, &http_code);
                APP_DEBUG1("response: %s", res_s.ptr);
                APP_DEBUG1("http_code: %d", http_code);
                APP_DEBUG1("api_connect_wifi: %s", api_connect_wifi);
                g_wifi_connect_res = res_s.ptr;

                g_ipaddress_res = "";
                g_metadata_res = "";
                g_wifi_scanner_res = "";
                memset(authorization_code, 0, STR_AU_CODE_MAX);
                memset(ssid, 0, STR_DATA_MAX);
                memset(psk, 0, STR_DATA_MAX);
                memset(api_connect_wifi, 0, STR_API_MAX);
            }
            else if (*(p_data->ser_write.value + 2) == 0x11) /* delete ssid */
            {
                APP_DEBUG1("create ssid: %d", p_data->ser_write.handle);
                memset(ssid, 0, STR_DATA_MAX);
                APP_DEBUG1("ssid: %s", ssid);
            }
            else if (*(p_data->ser_write.value + 2) == 0x12) /* insert ssid */
            {
                APP_DEBUG1("insert ssid: %d", p_data->ser_write.handle);
                app_ble_chopN((char *)p_data->ser_write.value, 3);
                strcat(ssid, p_data->ser_write.value);
                APP_DEBUG1("ssid_insert: %s", ssid);
            }
            else if (*(p_data->ser_write.value + 2) == 0x13) /* delete psk */
            {
                APP_DEBUG1("create psk: %d", p_data->ser_write.handle);
                memset(psk, 0, STR_DATA_MAX);
                APP_DEBUG1("psk%s", psk);
            }
            else if (*(p_data->ser_write.value + 2) == 0x14) /* insert psk */
            {
                APP_DEBUG1("insert psk: %d", p_data->ser_write.handle);
                app_ble_chopN((char *)p_data->ser_write.value, 3);
                strcat(psk, p_data->ser_write.value);
                APP_DEBUG1("psk_insert: %s", psk);
            }
            else
            {
                g_wifi_connect_res = "regis unknown";
                APP_DEBUG0("registration unknown received");
            }
        break;

        case CMD_BAV_CP_WIFI_SCANNER:
            APP_DEBUG1("registration CMD_BAV_CP_WIFI_SCANNER handle: %d", p_data->ser_write.handle);
            g_wifi_scanner = p_data->ser_write.handle;
            res_s = app_ble_curl_request("wifi-scanner", &http_code);
            APP_DEBUG1("response %s", res_s.ptr);
            APP_DEBUG1("http_code %d", http_code);
            g_wifi_scanner_res = res_s.ptr;

            g_ipaddress_res = "";
            g_metadata_res = "";
            g_wifi_connect_res = "";
            memset(authorization_code, 0, STR_AU_CODE_MAX);
            memset(ssid, 0, STR_DATA_MAX);
            memset(psk, 0, STR_DATA_MAX);
            memset(api_connect_wifi, 0, STR_API_MAX);
        break;

        case CMD_BAV_CP_METADATA_REQUEST:
            APP_DEBUG1("registration CMD_BAV_CP_METADATA_REQUEST handle: %d", p_data->ser_write.handle);
            g_metadata_request = p_data->ser_write.handle;
            res_s = app_ble_curl_request("metadata-request", &http_code);
            APP_DEBUG1("response %s", res_s.ptr);
            APP_DEBUG1("http_code %d", http_code);
            g_metadata_res = res_s.ptr;

            g_ipaddress_res = "";
            g_wifi_scanner_res = "";
            g_wifi_connect_res = "";
            memset(authorization_code, 0, STR_AU_CODE_MAX);
            memset(ssid, 0, STR_DATA_MAX);
            memset(psk, 0, STR_DATA_MAX);
            memset(api_connect_wifi, 0, STR_API_MAX);
        break;

        case CMD_BAV_CP_IPADRESS_REQUEST:
            APP_DEBUG1("registration CMD_BAV_CP_IPADRESS_REQUEST handle: %d", p_data->ser_write.handle);
            g_ipaddress_request = p_data->ser_write.handle;
            res_s = app_ble_curl_request("ipaddress-request", &http_code);
            APP_DEBUG1("response %s", res_s.ptr);
            APP_DEBUG1("http_code %d", http_code);
            g_ipaddress_res = res_s.ptr;

            g_metadata_res = "";
            g_wifi_scanner_res = "";
            g_wifi_connect_res = "";
            memset(authorization_code, 0, STR_AU_CODE_MAX);
            memset(ssid, 0, STR_DATA_MAX);
            memset(psk, 0, STR_DATA_MAX);
            memset(api_connect_wifi, 0, STR_API_MAX);
        break;

        case CMD_BAV_CP_AUTHORIZATION_REQUEST:
            if (*(p_data->ser_write.value + 2) == 0x10) /* send authorization_code */
            {
                APP_DEBUG1("send authorization_code handle: %d", p_data->ser_write.handle);
                app_ble_publish_message_mqtt(AUTHORIZATION_TOPIC, (char*)authorization_code);

                g_ipaddress_res = "";
                g_metadata_res = "";
                g_wifi_scanner_res = "";
                g_wifi_connect_res = "";
                memset(authorization_code, 0, STR_AU_CODE_MAX);
                memset(ssid, 0, STR_DATA_MAX);
                memset(psk, 0, STR_DATA_MAX);
                memset(api_connect_wifi, 0, STR_API_MAX);
            }
            else if (*(p_data->ser_write.value + 2) == 0x11) /* delete authorization_code */
            {
                APP_DEBUG1("registration CMD_BAV_CP_AUTHORIZATION_REQUEST handle: %d", p_data->ser_write.handle);
                APP_DEBUG1("delete authorization: %d", p_data->ser_write.handle);
                
                g_ipaddress_res = "";
                g_metadata_res = "";
                g_wifi_scanner_res = "";
                g_wifi_connect_res = "";
                memset(authorization_code, 0, STR_AU_CODE_MAX);
                memset(ssid, 0, STR_DATA_MAX);
                memset(psk, 0, STR_DATA_MAX);
                memset(api_connect_wifi, 0, STR_API_MAX);
            }
            else if (*(p_data->ser_write.value + 2) == 0x12) /* insert authorization_code */
            {
                APP_DEBUG1("registration CMD_BAV_CP_AUTHORIZATION_REQUEST handle: %d", p_data->ser_write.handle);
                APP_DEBUG1("insert authorization: %d", p_data->ser_write.handle);
                app_ble_chopN((char *)p_data->ser_write.value, 3);
                strcat(authorization_code, p_data->ser_write.value);
                APP_DEBUG1("authorization_code_insert: %s", authorization_code);
            }
            else
            {
                APP_DEBUG0("registration unknown received");
            }
        break;

        default:
            g_ipaddress_res = "unknown";
            g_metadata_res = "unknown";
            g_wifi_scanner_res = "unknown";
            g_wifi_connect_res = "unknown";
            APP_DEBUG0("registration unknown received");
        break;
    }
}

/*******************************************************************************
**
** Function         app_ble_chopN
**
** Description      CONNECTED APP Publish message mqtt
**
** Returns          void
**
*******************************************************************************/
BOOLEAN app_ble_publish_message_mqtt(const char *topic, char *payload) {
    int rc;
	struct mosquitto* mosq;

	mosquitto_lib_init();
	mosq = mosquitto_new("publisher-connected-app", true, NULL);
	rc = mosquitto_connect(mosq, IP_ADDRESS_MQTT, MOSQUITTO_PORT, 60);
	if(rc != 0){
        APP_DEBUG1("Client could not connect to broker! Error Code %d", rc);
		mosquitto_destroy(mosq);
		return false;
	}
    APP_DEBUG0("We are now connected to the broker!");

	mosquitto_publish(mosq, NULL, topic, strlen(payload), payload, 0, false);
	mosquitto_disconnect(mosq);
	mosquitto_destroy(mosq);

	mosquitto_lib_cleanup();
    return true;
}

/*******************************************************************************
**
** Function         app_ble_chopN
**
** Description      CONNECTED APP Server API Command
**
** Returns          void
**
*******************************************************************************/
size_t app_ble_chopN(char *str, size_t n)
{
    size_t len = strlen(str);
    if (n > len)
        n = len;
    memmove(str, str+n, len - n + 1);
    return(len - n);
}

/*******************************************************************************
**
** Function         app_ble_connected_app_api_command
**
** Description      CONNECTED APP Server API Command
**
** Returns          void
**
*******************************************************************************/
UINT8* app_ble_connected_app_api_command (UINT16 handle)
{
    UINT8* result_request;
    if (handle == g_wifi_connect_request)
    {
        APP_DEBUG1("command CMD_BAV_CP_WIFI_CONNECT_REQUEST handle: %d", handle);
        char *test_string = g_wifi_connect_res;
        int len_test = CMD_RES_DATA_MAX;
        result_request = (UINT8*)malloc(len_test*sizeof(UINT8));
        for(int i=0; i<len_test; i++)
            result_request[i] = test_string[i];
        app_ble_chopN(g_wifi_connect_res, CMD_RES_DATA_MAX);
    }
    else if (handle == g_wifi_scanner)
    {
        APP_DEBUG1("command CMD_BAV_CP_WIFI_SCANNER handle: %d", handle);
        char *test_string = g_wifi_scanner_res;
        result_request = (UINT8*)malloc(CMD_RES_DATA_MAX*sizeof(UINT8));
        for(int i=0; i<CMD_RES_DATA_MAX; i++)
            result_request[i] = test_string[i];
        app_ble_chopN(g_wifi_scanner_res, CMD_RES_DATA_MAX);
    }
    else if (handle == g_metadata_request)
    {
        APP_DEBUG1("command CMD_BAV_CP_METADATA_REQUEST handle: %d", handle);
        char *test_string = g_metadata_res;
        result_request = (UINT8*)malloc(CMD_RES_DATA_MAX*sizeof(UINT8));
        for(int i=0; i<CMD_RES_DATA_MAX; i++)
            result_request[i] = test_string[i];
        app_ble_chopN(g_metadata_res, CMD_RES_DATA_MAX);
    }
    else if (handle == g_ipaddress_request)
    {
        APP_DEBUG1("command CMD_BAV_CP_IPADRESS_REQUEST handle: %d", handle);
        char *test_string = g_ipaddress_res;
        result_request = (UINT8*)malloc(CMD_RES_DATA_MAX*sizeof(UINT8));
        for(int i=0; i<CMD_RES_DATA_MAX; i++)
            result_request[i] = test_string[i];
        app_ble_chopN(g_ipaddress_res, CMD_RES_DATA_MAX);
    }
    else
    {
        char *test_string = "unknown";
        result_request = (UINT8*)malloc(CMD_RES_DATA_MAX*sizeof(UINT8));
        for(int i=0; i<CMD_RES_DATA_MAX; i++)
            result_request[i] = test_string[i];
        APP_DEBUG0("command unknown received");
    }
    return result_request;
}
