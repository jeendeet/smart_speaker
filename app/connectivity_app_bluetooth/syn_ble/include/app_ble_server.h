/*****************************************************************************
**
**  Name:           app_ble_server.h
**
**  Description:    Bluetooth BLE include file
**
**  Copyright (c) 2014, VinBigData., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef APP_BLE_SERVER_H
#define APP_BLE_SERVER_H

#include "bsa_api.h"
#include "app_ble.h"

/*******************************************************************************
 **
 ** Function        app_ble_connected_power_level_to_byte_value
 **
 ** Description     get connected power level in dbm
 **
 ** Parameters      Tx power level
 **
 ** Returns         power level
 **
 *******************************************************************************/
static UINT8 app_ble_connected_power_level_to_byte_value(UINT8 txPowerLevel);

/*******************************************************************************
 **
 ** Function        app_ble_connected_start_connected_uid_adv
 **
 ** Description     start connected UID advertisement
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_connected_start_connected_uid_adv(void);

/*******************************************************************************
 **
 ** Function        app_ble_connected_set_adv_params
 **
 ** Description     set adv params
 **
 ** Parameters      instance ID of the advertisement (0 if non multi-adv)
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_connected_set_adv_params(UINT8 inst_id);

/*******************************************************************************
 **
 ** Function         app_ble_server_find_free_server
 **
 ** Description      find free server for BLE server application
 **
 ** Parameters
 **
 ** Returns          positive number(include 0) if successful, error code otherwise
 **
 *******************************************************************************/
int app_ble_server_find_free_server(void);

/*******************************************************************************
 **
 ** Function         app_ble_server_find_free_space
 **
 ** Description      find free space for BLE server application
 **
 ** Parameters
 **
 ** Returns          positive number(include 0) if successful, error code otherwise
 **
 *******************************************************************************/
int app_ble_server_find_free_space(void);

/*******************************************************************************
 **
 ** Function         app_ble_server_display
 **
 ** Description      display BLE server
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_ble_server_display(void);

/*******************************************************************************
 **
 ** Function         app_ble_server_find_reg_pending_index
 **
 ** Description      find registration pending index
 **
 ** Parameters
 **
 ** Returns          positive number(include 0) if successful, error code otherwise
 **
 *******************************************************************************/
int app_ble_server_find_reg_pending_index(void);

/*******************************************************************************
 **
 ** Function         app_ble_server_find_index_by_interface
 **
 ** Description      find BLE server index by interface 
 **
 ** Parameters    if_num: interface number
 **
 ** Returns          positive number(include 0) if successful, error code otherwise
 **
 *******************************************************************************/
int app_ble_server_find_index_by_interface(tBSA_BLE_IF if_num);


/*
 * BLE Server functions
 */
/*******************************************************************************
 **
 ** Function        app_ble_server_register_connected_app
 **
 ** Description     Register server app
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_server_register_connected_app(tBSA_BLE_CBACK *p_cback);

/*******************************************************************************
 **
 ** Function        app_ble_server_deregister
 **
 ** Description     Deregister server app
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_server_deregister(void);

/*******************************************************************************
 **
 ** Function        app_ble_connected_app_create_service
 **
 ** Description     create service
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_connected_app_create_service(void);

/*******************************************************************************
 **
 ** Function        app_ble_connected_add_char
 **
 ** Description     Add character to service
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_connected_add_char(UINT8 *uuid_char, tBSA_BLE_PERM  perm, tBSA_BLE_CHAR_PROP prop);

/*******************************************************************************
 **
 ** Function        app_ble_connected_start_service
 **
 ** Description     Start Service
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_connected_start_service(void);

/*******************************************************************************
 **
 ** Function        app_ble_server_stop_service
 **
 ** Description     Stop Service
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_server_stop_service(void);

/*******************************************************************************
 **
 ** Function        app_ble_server_send_indication
 **
 ** Description     Send indication to client
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_server_send_indication(void);

/*******************************************************************************
**
** Function         app_ble_server_profile_cback
**
** Description      BLE Server Profile callback.
**                  
** Returns          void
**
*******************************************************************************/
void app_ble_server_profile_cback(tBSA_BLE_EVT event,  tBSA_BLE_MSG *p_data);

/*******************************************************************************
 **
 ** Function        app_ble_server_open
 **
 ** Description     This is the ble open connection to ble client
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_server_open(void);

/*******************************************************************************
 **
 ** Function        app_ble_server_close
 **
 ** Description     This is the ble close connection
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_server_close(void);

#endif
