/*****************************************************************************
 **
 **  Name:           app_json.c
 **
 **  Description:    Json functions support bluetooth
 **
 **  Copyright (c) 2022, VinBigdata., All Rights Reserved.
 **  VinBigData Core. Proprietary and confidential.
 **
 *****************************************************************************/
#include "app_json.h"
#include <iostream>
#include <fstream>
#include <sstream>

#define BLUETOOTH_STATE_FILE                        "/tmp/blue_state.json"

/*******************************************************************************
 **
 ** Function        file_to_json
 **
 ** Description     Read a file, parse, render back, etc.
 **
 ** Parameters      ...
 **
 ** Returns         None
 **
 *******************************************************************************/
cJSON* app_file_to_json(char *filename)
{
    FILE *f;long len;char *data;
    cJSON *content;
    
    f=fopen(filename,"rb");fseek(f,0,SEEK_END);len=ftell(f);fseek(f,0,SEEK_SET);
    data=(char*)malloc(len+1);fread(data,1,len,f);fclose(f);
    content=cJSON_Parse(data);
    free(data);
    return content;
}

/*******************************************************************************
 **
 ** Function        app_update_json_item
 **
 ** Description     update json item
 **
 ** Parameters      ...
 **
 ** Returns         None
 **
 *******************************************************************************/
void app_update_json_item(const char *key_string, const char *value) 
{
	std::ofstream stream;
    cJSON *j_content;
    cJSON *newitem = cJSON_CreateString(value);

    char *json_file = (char*) BLUETOOTH_STATE_FILE;
    j_content = app_file_to_json(json_file);
    cJSON_ReplaceItemInObject(j_content, key_string, newitem);
    char *rendered=cJSON_Print(j_content);
    
    stream.open((char*)BLUETOOTH_STATE_FILE);
    if(!stream)
        APP_DEBUG0("Opening file failed");
    // use operator<< for clarity
    stream << rendered << std::endl;
}

/*******************************************************************************
 **
 ** Function        app_get_json_item
 **
 ** Description     get json item
 **
 ** Parameters      ...
 **
 ** Returns         None
 **
 *******************************************************************************/
char* app_get_json_item(const char *key_string) 
{
    cJSON *j_content;

    char *json_file = (char*) BLUETOOTH_STATE_FILE;
    j_content = app_file_to_json(json_file);
    return cJSON_GetObjectItem(j_content, key_string)->valuestring;;
}

