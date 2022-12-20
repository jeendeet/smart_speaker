/* Http notification */
#define HTTP_200_OK "HTTP/1.1 200 OK\n"
#define HTTP_400_ERROR "HTTP/1.1 400 Bad Request\n"
#define HTTP_407_ERROR "HTTP/1.1 407 Proxy Authentication Required\n"
#define HTTP_406_ERROR "HTTP/1.1 406 Not Acceptable\n"
#define HTTP_405_ERROR "HTTP/1.1 405 Method Not Allowed\n"
#define HTTP_500_SERVER_ERROR "HTTP/1.1 500 Internal Server Error\n"

/* notification GET /wifi_connect_request */
/* json format with space character in the end of string */
#define SSID_NOT_EXISTED "{\"status\": \"ERROR\", \"noti\":\"ssid is not existed\"}"
#define PSK_NOT_EXISTED "{\"status\": \"ERROR\", \"noti\":\"psk is not existed\"}"
#define API_NOT_EXISTED "{\"status\": \"ERROR\", \"noti\":\"API is not existed\"}"
#define WIFI_CONNECTION "{\"status\": \"SUCCESS\", \"noti\":\"Wifi connected\"}"
#define NO_WIFI_CONNECTION "{\"status\": \"FAIL\", \"noti\":\"No wifi connected, cannot get ip addess\"}"
#define WIFI_CONNECTION_FAIL "{\"status\": \"FAIL\", \"noti\":\"Wifi connects fail\"}"
#define WIFI_WRONG_KEY "{\"status\": \"FAIL\", \"noti\":\"Wifi connects with wrong key\"}"
#define WIFI_NO_INTERNET "{\"status\": \"FAIL\", \"noti\":\"Wifi connects no internet\"}"
#define WIFI_SCANNER_NOT_FOUND "{\"status\": \"FAIL\", \"noti\":\"Wifi scanner not found\"}"

/* parameter from json file */
#define JSON_FILE "./config/params.json"
#define DEVICE_INFO_FILE "./config/device_info.json"
#define INFO_FILE "./config/info.json"

/* interface ethernet */
#define INTERFACE_ETHERNET "wlan0"

/* log level */
/* LOG_LEVEL 5 : LOG_ERROR, LOG_WARNING, LOG_DEBUG, LOG_INFO */
/* LOG_LEVEL 4 : LOG_ERROR, LOG_WARNING, LOG_DEBUG*/
#define LOG_LEVEL 5

/* buffer size */
#define BUFFER_SIZE 2024
