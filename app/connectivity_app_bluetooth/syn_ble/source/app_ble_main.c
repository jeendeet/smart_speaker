/*****************************************************************************
**
**  Name:           app_ble_main.c
**
**  Description:    Bluetooth BLE Main application
**
**  Copyright (c) 2015-2016, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/

#include <stdlib.h>
#include <unistd.h>
#include "app_ble.h"
#include "app_utils.h"
#include "app_disc.h"
#include "app_mgt.h"
#include "app_dm.h"
#include "app_ble_server.h"
/*
 * Defines
 */
UINT8 wifi_connect_request[16]=         {0xe1, 0x0c, 0x20, 0x84, 0x87, 0x86, 0x40, 0xba, 0xab, 0x96, 0x99, 0xb9, 0x1a, 0xc9, 0x81, 0xd0};
UINT8 wifi_scanner[16]=                 {0xe2, 0x0c, 0x20, 0x84, 0x87, 0x86, 0x40, 0xba, 0xab, 0x96, 0x99, 0xb9, 0x1a, 0xc9, 0x81, 0xd0};
UINT8 metadata_request[16]=             {0xe3, 0x0c, 0x20, 0x84, 0x87, 0x86, 0x40, 0xba, 0xab, 0x96, 0x99, 0xb9, 0x1a, 0xc9, 0x81, 0xd0};
UINT8 ipaddress_request[16]=            {0xe4, 0x0c, 0x20, 0x84, 0x87, 0x86, 0x40, 0xba, 0xab, 0x96, 0x99, 0xb9, 0x1a, 0xc9, 0x81, 0xd0};
UINT8 authorization_request[16]=        {0xe5, 0x0c, 0x20, 0x84, 0x87, 0x86, 0x40, 0xba, 0xab, 0x96, 0x99, 0xb9, 0x1a, 0xc9, 0x81, 0xd0};

/*******************************************************************************
 **
 ** Function         app_ble_run
 **
 ** Description      Handle the BLE run
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_ble_run(void)
{
    tBSA_DM_BLE_CONN_PARAM conn_param;
    UINT8 connected_app_bdaddr[6] = {0xA0, 0xA0, 0xA0, 0xA0, 0xA0, 0xA0};
    bdcpy(conn_param.bd_addr,connected_app_bdaddr);
    conn_param.min_conn_int = 300;
    conn_param.max_conn_int = 400;
    conn_param.slave_latency = 1;
    conn_param.supervision_tout = 3;

    app_ble_connected_start_connected_uid_adv();

    app_ble_server_register_connected_app(app_ble_server_profile_cback);
    GKI_delay(1000);
    //app_ble_server_register(APP_BLE_MAIN_INVALID_UUID, NULL);
    app_ble_connected_app_create_service();
    GKI_delay(1000);

    app_ble_connected_add_char(
        wifi_connect_request,
        BSA_GATT_PERM_READ | BSA_GATT_PERM_WRITE,
        BSA_GATT_CHAR_PROP_BIT_READ | BSA_GATT_CHAR_PROP_BIT_WRITE
    );
    /* attr_id:0x3e 62 */
    GKI_delay(1000);

    app_ble_connected_add_char(
        wifi_scanner,
        BSA_GATT_PERM_READ | BSA_GATT_PERM_WRITE,
        BSA_GATT_CHAR_PROP_BIT_READ | BSA_GATT_CHAR_PROP_BIT_WRITE
    );
    /* attr_id:0x40 64 */
    GKI_delay(1000);

    app_ble_connected_add_char(
        metadata_request,
        BSA_GATT_PERM_READ | BSA_GATT_PERM_WRITE,
        BSA_GATT_CHAR_PROP_BIT_READ | BSA_GATT_CHAR_PROP_BIT_WRITE
    );
    /* attr_id:0x42 66 */
    GKI_delay(1000);

    app_ble_connected_add_char(
        ipaddress_request,
        BSA_GATT_PERM_READ | BSA_GATT_PERM_WRITE,
        BSA_GATT_CHAR_PROP_BIT_READ | BSA_GATT_CHAR_PROP_BIT_WRITE
    );
        /* attr_id:0x44 68 */
    GKI_delay(1000);

    app_ble_connected_add_char(
        authorization_request,
        BSA_GATT_PERM_READ | BSA_GATT_PERM_WRITE,
        BSA_GATT_CHAR_PROP_BIT_READ | BSA_GATT_CHAR_PROP_BIT_WRITE
    );
    /* attr_id:0x44 68 */
    GKI_delay(1000);

    app_dm_set_ble_conn_param(&conn_param);
    GKI_delay(1000);

    /* start service */
    app_ble_connected_start_service();
    GKI_delay(1000);

    /* Set visisble and connectable */
    app_dm_set_ble_visibility(TRUE, TRUE);

    app_ble_server_display();

    do {
        sleep(10);
    }
    while(1);

}


/*******************************************************************************
 **
 ** Function         app_ble_mgt_callback
 **
 ** Description      This callback function is called in case of server
 **                  disconnection (e.g. server crashes)
 **
 ** Parameters
 **
 ** Returns          BOOLEAN
 **
 *******************************************************************************/
BOOLEAN app_ble_mgt_callback(tBSA_MGT_EVT event, tBSA_MGT_MSG *p_data)
{
    switch(event)
    {
    case BSA_MGT_STATUS_EVT:
        APP_DEBUG0("BSA_MGT_STATUS_EVT");
        if (p_data->status.enable)
        {
            APP_DEBUG0("Bluetooth restarted => re-initialize the application");
            app_ble_start();
        }
        break;

    case BSA_MGT_DISCONNECT_EVT:
        APP_DEBUG1("BSA_MGT_DISCONNECT_EVT reason:%d", p_data->disconnect.reason);
        /* Connection with the Server lost => Just exit the application */
        exit(-1);
        break;

    default:
        break;
    }
    return FALSE;
}

/*******************************************************************************
 **
 ** Function        main
 **
 ** Description     This is the main function
 **
 ** Parameters      Program's arguments
 **
 ** Returns         status
 **
 *******************************************************************************/
int main(int argc, char **argv)
{
    int status;

    /* Initialize BLE application */
    status = app_ble_init();
    if (status < 0)
    {
        APP_ERROR0("Couldn't Initialize BLE app, exiting");
        exit(-1);
    }

    /* Open connection to BSA Server */
    app_mgt_init();
    if (app_mgt_open(NULL, app_ble_mgt_callback) < 0)
    {
        APP_ERROR0("Unable to connect to server");
        return -1;
    }

    /* Start BLE application */
    status = app_ble_start();
    if (status < 0)
    {
        APP_ERROR0("Couldn't Start BLE app, exiting");
        return -1;
    }

    /* The main BLE loop */
    app_ble_run();

    /* Exit BLE mode */
    app_ble_exit();

    /* Close BSA Connection before exiting (to release resources) */
    app_mgt_close();

    exit(0);
}

