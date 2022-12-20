/*****************************************************************************
**
**  Name:           app_handle.c
**
**  Description:    Bluetooth BLE Curl request
**
**  Copyright (c) 2022, Vinbigdata., All Rights Reserved.
**  Proprietary and confidential.
**
*****************************************************************************/

#include "app_curl_request.h"
#include "app_utils.h"

/*******************************************************************************
**
** Function         app_ble_init_string
**
** Description      CONNECTED APP
**
** Returns          void
**
*******************************************************************************/
void app_ble_init_string(struct _responseString *s) 
{
    s->len = 0;
    s->ptr = malloc(s->len+1);
    if (s->ptr == NULL) {
        fprintf(stderr, "malloc() failed\n");
        exit(EXIT_FAILURE);
    }
    s->ptr[0] = '\0';
}

/*******************************************************************************
**
** Function         app_ble_writefunc
**
** Description      CONNECTED APP
**
** Returns          UINT8 array
**
*******************************************************************************/
size_t app_ble_writefunc(void *ptr, size_t size, size_t nmemb, struct _responseString *s)
{
    size_t new_len = s->len + size*nmemb;
    s->ptr = realloc(s->ptr, new_len+1);
    if (s->ptr == NULL) {
        fprintf(stderr, "realloc() failed\n");
        exit(EXIT_FAILURE);
    }
    memcpy(s->ptr+s->len, ptr, size*nmemb);
    s->ptr[new_len] = '\0';
    s->len = new_len;

    return size*nmemb;
}

/*******************************************************************************
**
** Function         app_ble_itoa
**
** Description      CONNECTED APP covert in to string in char*
**
** Returns          UINT8 array
**
*******************************************************************************/
char* app_ble_itoa(int value, char* result, int base) 
{
    // check that the base if valid
    if (base < 2 || base > 36) { *result = '\0'; return result; }

    char* ptr = result, *ptr1 = result, tmp_char;
    int tmp_value;

    do {
        tmp_value = value;
        value /= base;
        *ptr++ = "zyxwvutsrqponmlkjihgfedcba9876543210123456789abcdefghijklmnopqrstuvwxyz" [35 + (tmp_value - value * base)];
    } while ( value );

    // Apply negative sign
    if (tmp_value < 0) *ptr++ = '-';
    *ptr-- = '\0';
    while(ptr1 < ptr) {
        tmp_char = *ptr;
        *ptr--= *ptr1;
        *ptr1++ = tmp_char;
    }
    return result;
}

/*******************************************************************************
**
** Function         app_ble_curl_request
**
** Description      CONNECTED APP
**
** Returns          UINT8 array
**
*******************************************************************************/
struct _responseString app_ble_curl_request(char *api, long *http_code)
{
    CURL *curl;
    CURLcode res;
    struct _responseString s;
    char* url_request;
    char snum[5];

    APP_DEBUG1("curl request handle: %s", api);
    curl = curl_easy_init();
    if(curl) {
        APP_DEBUG0("make request success");
        app_ble_init_string(&s);
        /* make space for the new string (should check the return value ...) */
        url_request = malloc(HTTP_CONNECTED_APP_SERVER_REQUEST_SIZE_MAX);
        /* copy name into the new var */
        strcpy(url_request, (char *)HTTP_CONNECTED_APP_SERVER_REQUEST);
        strcat(url_request, api); /* add the extension */

        curl_easy_setopt(curl, CURLOPT_URL, url_request);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, app_ble_writefunc);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &s);
        res = curl_easy_perform(curl);
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, http_code);
        free(url_request);
        /* always cleanup */
        curl_easy_cleanup(curl);
    }
    return s;
} 
