/*****************************************************************************
 **
 **  Name:           app_json.h
 **
 **  Description:    Json functions support bluetooth
 **
 **  Copyright (c) 2022, VinBigdata., All Rights Reserved.
 **  VinBigData Core. Proprietary and confidential.
 **
 *****************************************************************************/

/* idempotency */
#ifndef APP_JSON_H
#define APP_JSON_H

#include <stdio.h>
#include <stdlib.h>

#include "cJSON.h"
#include "app_utils.h"

/*******************************************************************************
 **
 ** Function        app_update_json_item
 **
 ** Description     Connect to MQTT service
 **
 ** Parameters      ...
 **
 ** Returns         None
 **
 *******************************************************************************/
void app_update_json_item(const char *key_string, const char *value);

/*******************************************************************************
 **
 ** Function        app_get_json_item
 **
 ** Description     Connect to MQTT service
 **
 ** Parameters      ...
 **
 ** Returns         None
 **
 *******************************************************************************/
char* app_get_json_item(const char *key_string);

#endif /* APP_JSON_H */
