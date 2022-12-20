# API 
# GET http://10.10.10.254:8080/wifi-connect-request?ssid={{ssid}}&psk={{psk}}

- Error with wrong API name: ex /wifi_connect_re
405 Method Not Allowed
{
    "status": "ERROR",
    "noti": "API is not existed"
}

- Error with wrong key ssid:
405 Method Not Allowed
{
    "status": "ERROR",
    "noti": "Wifi connects fail"
}

- Error with wrong key psk:
406 Not Acceptable
{
    "status": "ERROR",
    "noti": "Wifi connects with wrong key"
}

- Error with cannot connect wifi
407 Proxy Authentication Required
{
    "status": "ERROR",
    "noti": "Wifi connects fail"
}

- Success
200 OK
{
    "status": "SUCCESS",
    "noti": "Wifi connected"
}

# GET /wifi_scanner ex GET http://10.10.10.254:8080/wifi-scanner
Response:
200 OK
{
    "cell_0": {
        "essid": "VinBigdata-Support",
        "quality": "55/70  ",
        "signal_level": "-55 dBm",
        "last_beacon": " 20ms ago"
    },
    "cell_1": {
        "essid": "VinBigdata-Staff",
        "quality": "56/70  ",
        "signal_level": "-54 dBm",
        "last_beacon": " 20ms ago"
    },
    "cell_2": {
        "essid": "VinBigdata-Guest",
        "quality": "53/70  ",
        "signal_level": "-57 dBm",
        "last_beacon": " 20ms ago"
    },
    "cell_3": {
        "essid": "TP-Link_0FDE",
        "quality": "50/70  ",
        "signal_level": "-60 dBm",
        "last_beacon": " 20ms ago"
    },
    "cell_4": {
        "essid": "Earth 616",
        "quality": "45/70  ",
        "signal_level": "-65 dBm",
        "last_beacon": " 20ms ago"
    }
}

# GET /metadata_request ex GET http://10.10.10.254:8080/metadata-request
Response:
200 OK
{
    "product_id": "VinBigdata-Speaker-123456",
    "mac_address": "b8:27:eb:8f:41:e6",
    "device_serial_number": "036000291452",
    "code_challenge": "T0HiVJtkKoX6"
}

# GET /ipaddress-request ex GET http://10.10.10.254:8080/ipaddress-request
Response:
200 OK
{
    "ip_address": "172.16.2.49"
}

400 Bad Request
{
    "status": "FAIL",
    "noti": "No wifi connected, cannot get ip addess"
}
