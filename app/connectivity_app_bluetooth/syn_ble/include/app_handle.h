/*****************************************************************************
**
**  Name:           app_handle.h
**
**  Description:    Bluetooth BLE include file
**
**  Copyright (c) 2022, VinBigData., All Rights Reserved.
**
*****************************************************************************/
#ifndef APP_BLE_HANDLE_CONNECTED_APP_H
#define APP_BLE_HANDLE_CONNECTED_APP_H

#include "bsa_api.h"
#include "app_ble.h"

/*******************************************************************************
**
** Function         app_ble_connected_app_api_registration
**
** Description      CONNECTED APP Server callback
**
** Returns          void
**
*******************************************************************************/
void app_ble_connected_app_api_registration (tBSA_BLE_MSG *p_data);

/*******************************************************************************
**
** Function         app_ble_connected_app_api_command
**
** Description      CONNECTED APP Server API Command
**
** Returns          UINT8 array
**
*******************************************************************************/
UINT8* app_ble_connected_app_api_command (UINT16 handle);

/*******************************************************************************
**
** Function         app_ble_chopN
**
** Description      CONNECTED APP Server API Command
**
** Returns          void
**
*******************************************************************************/
size_t app_ble_chopN(char *str, size_t n);

/*******************************************************************************
**
** Function         app_ble_chopN
**
** Description      CONNECTED APP Publish message mqtt
**
** Returns          void
**
*******************************************************************************/
BOOLEAN app_ble_publish_message_mqtt(const char *topic, char *payload);

#endif