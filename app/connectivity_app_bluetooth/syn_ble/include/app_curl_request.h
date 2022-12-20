/*****************************************************************************
**
**  Name:           app_curl_request.h
**
**  Description:    Bluetooth BLE include file for curl request
**
**  Copyright (c) 2022, VinBigData, All Rights Reserved.
**
*****************************************************************************/
#ifndef APP_BLE_CURL_REQUEST_CONNECTED_APP_H
#define APP_BLE_CURL_REQUEST_CONNECTED_APP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#define  HTTP_CONNECTED_APP_SERVER_REQUEST          "http://localhost:8080/"
#define  HTTP_CONNECTED_APP_SERVER_REQUEST_SIZE_MAX 100
struct _responseString {
    char *ptr;
    size_t len;
};

/*******************************************************************************
**
** Function         app_ble_init_string
**
** Description      CONNECTED APP
**
** Returns          void
**
*******************************************************************************/
void app_ble_init_string(struct _responseString *s);

/*******************************************************************************
**
** Function         app_ble_writefunc
**
** Description      CONNECTED APP
**
** Returns          UINT8 array
**
*******************************************************************************/
size_t app_ble_writefunc(void *ptr, size_t size, size_t nmemb, struct _responseString *s);

/*******************************************************************************
**
** Function         app_ble_curl_request
**
** Description      CONNECTED APP
**
** Returns          UINT8 array
**
*******************************************************************************/
struct _responseString app_ble_curl_request(char *api, long *http_code);

char* app_ble_itoa(int value, char* result, int base);


#endif /* APP_BLE_CURL_REQUEST_CONNECTED_APP_H */