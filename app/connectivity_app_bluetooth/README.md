--------------------------------------------------------------------
Bluetooth Low Energy API / Service (Read and Write)
--------------------------------------------------------------------

# Build
make

# Run server
LD_LIBRARY_PATH={{path_library}} ./connected_app_ble

# Connect BLE step (hex code)
1. Connect to advertisement: 
UUID: 0xAAAA
Data: 0x00DD 0x010203010506070809000 0xA0B0C0D0E0F00

Advertisement includes 4 services for 4 API with UUIDs
Execute read/write to service to take information and make commands

2. Read/Write to service:

--------------------------------------------------------------------
`Wifi connect request:` `Service UUID: 0xd081c91ab99996abba40868784200ce1`
Add ssid:               `Write`
                        byte1: 0x01             (BYTE) 
                        byte2: 0x01             (BYTE)
                        byte3: 0x12             (BYTE)
                        text:  Vinbigdata-guest (TEXT)
                        (max size 15 / if max size ssid > 15, please separate ssid and add manytime)

Remove ssid:            `Write`
                        byte1: 0x01             (BYTE) 
                        byte2: 0x01             (BYTE)
                        byte3: 0x11             (BYTE)

Add psk:                `Write`
                        byte1: 0x01             (BYTE) 
                        byte2: 0x01             (BYTE)
                        byte3: 0x14             (BYTE)
                        text:  123456a@         (TEXT)
                        (max size 15 / if max size psk > 15, please separate psk and add manytime)
                
Remove psk:             `Write`
                        byte1: 0x01             (BYTE) 
                        byte2: 0x01             (BYTE)
                        byte3: 0x13             (BYTE)

Request connect wifi:   `Write`
                        byte1: 0x01             (BYTE) 
                        byte2: 0x01             (BYTE)
                        byte3: 0x10             (BYTE)

Read response:          `Read` (20 bytes for each times, read manytime until response empty)
                        (Response in hex code so we have to convert to string https://onlinestringtools.com/convert-hexadecimal-to-string)

--------------------------------------------------------------------
`wifi-scanner` `Service UUID: 0xd081c91ab99996abba40868784200ce2`
Scanner wifi:           `Write`
                        byte1: 0x01             (BYTE)
                        byte2: 0x02             (BYTE)

Read response:          `Read` (20 bytes for each times, read manytime until response empty)
                        (Response in hex code so we have to convert to string https://onlinestringtools.com/convert-hexadecimal-to-string)

--------------------------------------------------------------------
`metadata-request` `Service UUID: 0xd081c91ab99996abba40868784200ce3`
Metadata request:       `Write`
                        byte1: 0x01             (BYTE)
                        byte2: 0x03             (BYTE)
                        
Read response:          `Read` (20 bytes for each times, read manytime until response empty)
                        (Response in hex code so we have to convert to string https://onlinestringtools.com/convert-hexadecimal-to-string)

--------------------------------------------------------------------
`ipaddress-request` `Service UUID: 0xd081c91ab99996abba40868784200ce4`
Ipadress request:       `Write`
                        byte1: 0x01             (BYTE)
                        byte2: 0x04             (BYTE)
                        
Read response:          `Read` (20 bytes for each times, read manytime until response empty)
                        (Response in hex code so we have to convert to string https://onlinestringtools.com/convert-hexadecimal-to-string)

--------------------------------------------------------------------
`authorization-request` `Service UUID: 0xd081c91ab99996abba40868784200ce5`
Send authorization :    `Write`
                        byte1: 0x01             (BYTE)
                        byte2: 0x05             (BYTE)
                        byte3: 0x10             (BYTE)

Remove authorization :  `Write`
                        byte1: 0x01             (BYTE)
                        byte2: 0x05             (BYTE)
                        byte3: 0x11             (BYTE)

Authorization request:  `Write`
                        byte1: 0x01             (BYTE)
                        byte2: 0x05             (BYTE)
                        byte3: 0x12             (BYTE)
                        text:  json_code        (TEXT)
                        (max size 15 / if max size json_code > 15, please separate json_code and insert manytime)
json_code example:
{
    "authorization_code":"Z0FBQUFBQmlsWjNydHNqbUlYV1FnT0NoMzhidml6YjJKVWxoQm9LNW9aZlNpajJHeHcwN1NiNDNNdDR4ZFl0dnU0MjVqcl9MMEphekZZUkM5VnpQcDZ4WHRqc09SQnJ3aHZ2TVBURjBqYnBlZU5ZZW5MZDRxV1BKRjk2TkwtNHJpelBhUmVlRXAtNFRYTVFiZUhocGI3bk5ONndkTUVNZGtBPT0",
    "profile_id": 1652868930922904300
}
`Send code to mosquitto broker -> avs service`
Send authorization code to mqtt with topic: metadata/authorization
Mosquitto broker localhost port 1883 

--------------------------------------------------------------------
`Example:`
Response in hex:
7B2270726F647563745F6964223A202256696E426967646174612D537065616B65722D313233343536222C6D61635F61646472657373223A202232303A35303A61373A30643A3330222C226465766963655F73657269616C5F6E756D626572223A2022303336303030323931343532222C22636F64655F6368616C6C656E6765223A20223370416D64343761456F3955227D

Convert to strings:
{"product_id": "VinBigdata-Speaker-123456",mac_address": "20:50:a7:0d:30","device_serial_number": "036000291452","code_challenge": "3pAmd47aEo9U"}

--------------------------------------------------------------------
`Logs example`

# connect
BSA_BLE_SE_OPEN_EVT status:0
conn_id:0x4
DEBUG: app_read_xml_remote_devices: read(./bt_devices.xml): OK
Added trusted services
DEBUG: app_write_xml_remote_devices: write(./bt_devices.xml): OK

# metadata-request
DEBUG: app_ble_server_profile_cback: app_ble_server_profile_cback event = 17 
BSA_BLE_SE_WRITE_EVT status:0
BSA_trace 30@ 01/01 02h:00m:25s:775ms: Write value:
BSA_trace 31@ 01/01 02h:00m:25s:775ms:     0000: 01 03                                             ..              
Write value:����, len:2
BSA_BLE_SE_WRITE_EVT trans_id:1, conn_id:4, handle:46, is_prep:0, offset:0
DEBUG: app_ble_connected_app_api_registration: registration CMD_BAV_CP_METADATA_REQUEST handle: 46
DEBUG: app_ble_curl_request: curl request handle: metadata-request
DEBUG: app_ble_curl_request: make request success
DEBUG: app_ble_connected_app_api_registration: response {"product_id": "VinBigdata-Speaker-123456","mac_address": "20:50:e7:a7:0d:30","device_serial_number": "036000291452","code_challenge": "TSZZY3JMCEqE"}
DEBUG: app_ble_connected_app_api_registration: http_code 200
BSA_trace 32@ 01/01 02h:00m:25s:794ms: BSA_BleSeSendRspInit
BSA_BLE_SE_WRITE_EVT: send_server_resp.conn_id:4, send_server_resp.trans_id:1
BSA_BLE_SE_WRITE_EVT: send_server_resp.status:0,send_server_resp.auth_req:0
BSA_BLE_SE_WRITE_EVT: send_server_resp.handle:46, send_server_resp.offset:0, send_server_resp.len:4
BSA_trace 33@ 01/01 02h:00m:25s:794ms: BSA_BleSeSendRsp

# ipaddress-request
DEBUG: app_ble_server_profile_cback: app_ble_server_profile_cback event = 17 
BSA_BLE_SE_WRITE_EVT status:0
BSA_trace 34@ 01/01 02h:02m:39s:472ms: Write value:
BSA_trace 35@ 01/01 02h:02m:39s:472ms:     0000: 01 04                                             ..              
Write value:, len:2
BSA_BLE_SE_WRITE_EVT trans_id:2, conn_id:4, handle:48, is_prep:0, offset:0
DEBUG: app_ble_connected_app_api_registration: registration CMD_BAV_CP_IPADRESS_REQUEST handle: 48
DEBUG: app_ble_curl_request: curl request handle: ipaddress-request
DEBUG: app_ble_curl_request: make request success
DEBUG: app_ble_connected_app_api_registration: response {"ip_address": "172.16.2.49"}
DEBUG: app_ble_connected_app_api_registration: http_code 200
BSA_trace 36@ 01/01 02h:02m:39s:487ms: BSA_BleSeSendRspInit
BSA_BLE_SE_WRITE_EVT: send_server_resp.conn_id:4, send_server_resp.trans_id:2
BSA_BLE_SE_WRITE_EVT: send_server_resp.status:0,send_server_resp.auth_req:0
BSA_BLE_SE_WRITE_EVT: send_server_resp.handle:48, send_server_resp.offset:0, send_server_resp.len:4
BSA_trace 37@ 01/01 02h:02m:39s:487ms: BSA_BleSeSendRsp

# wifi-scanner
DEBUG: app_ble_server_profile_cback: app_ble_server_profile_cback event = 17 
BSA_BLE_SE_WRITE_EVT status:0
BSA_trace 38@ 01/01 02h:03m:39s:638ms: Write value:
BSA_trace 39@ 01/01 02h:03m:39s:638ms:     0000: 01 02                                             ..              
Write value:, len:2
BSA_BLE_SE_WRITE_EVT trans_id:3, conn_id:4, handle:44, is_prep:0, offset:0
DEBUG: app_ble_connected_app_api_registration: registration CMD_BAV_CP_WIFI_SCANNER handle: 44
DEBUG: app_ble_curl_request: curl request handle: wifi-scanner
DEBUG: app_ble_curl_request: make request success
DEBUG: app_ble_connected_app_api_registration: response {"cell_0":{"essid": "VinBigdata-Guest"
                    ,"quality": "5/5  ","signal_level": "-51 dBm"},"cell_1":{"essid": "VinBigdata-Support"
                    ,"quality": "5/5  ","signal_level": "-49 dBm"},"cell_2":{"essid": "VinBigdata-Staff"
                    ,"quality": "5/5  ","signal_level": "-49 dBm"},"cell_3":{"essid": "T5-1102"
                    ,"quality": "1/5  ","signal_level": "-86 dBm"},"cell_4":{"essid": "TP-Link_0FDE"
                    ,"quality": "5/5  ","signal_level": "-55 dBm"},"cell_5":{"essid": "TAN THANH PHAT"
                    ,"quality": "4/5  ","signal_level": "-66 dBm"}} 
DEBUG: app_ble_connected_app_api_registration: http_code 200
BSA_trace 40@ 01/01 02h:03m:45s:525ms: BSA_BleSeSendRspInit
BSA_BLE_SE_WRITE_EVT: send_server_resp.conn_id:4, send_server_resp.trans_id:3
BSA_BLE_SE_WRITE_EVT: send_server_resp.status:0,send_server_resp.auth_req:0
BSA_BLE_SE_WRITE_EVT: send_server_resp.handle:44, send_server_resp.offset:0, send_server_resp.len:4
BSA_trace 41@ 01/01 02h:03m:45s:525ms: BSA_BleSeSendRsp

# add ssid
DEBUG: app_ble_server_profile_cback: app_ble_server_profile_cback event = 17 
BSA_BLE_SE_WRITE_EVT status:0
BSA_trace 42@ 01/01 02h:04m:50s:064ms: Write value:
BSA_trace 43@ 01/01 02h:04m:50s:064ms:     0000: 01 01 12 56 69 6e 42 69 67 64 61 74 61 2d 47 75   ...VinBigdata-Gu
BSA_trace 44@ 01/01 02h:04m:50s:064ms:     0010: 65 73 74                                          est             
Write value:VinBigdata-Guest, len:19
BSA_BLE_SE_WRITE_EVT trans_id:4, conn_id:4, handle:42, is_prep:0, offset:0
DEBUG: app_ble_connected_app_api_registration: insert ssid: 42
DEBUG: app_ble_connected_app_api_registration: ssid_insert: VinBigdata-Guest
BSA_trace 45@ 01/01 02h:04m:50s:064ms: BSA_BleSeSendRspInit
BSA_BLE_SE_WRITE_EVT: send_server_resp.conn_id:4, send_server_resp.trans_id:4
BSA_BLE_SE_WRITE_EVT: send_server_resp.status:0,send_server_resp.auth_req:0
BSA_BLE_SE_WRITE_EVT: send_server_resp.handle:42, send_server_resp.offset:0, send_server_resp.len:4
BSA_trace 46@ 01/01 02h:04m:50s:064ms: BSA_BleSeSendRsp

# add psk
DEBUG: app_ble_server_profile_cback: app_ble_server_profile_cback event = 17 
BSA_BLE_SE_WRITE_EVT status:0
BSA_trace 47@ 01/01 02h:05m:26s:604ms: Write value:
BSA_trace 48@ 01/01 02h:05m:26s:604ms:     0000: 01 01 14 31 32 33 34 35 36 61 40                  ...123456a@     
Write value:123456a@, len:11
BSA_BLE_SE_WRITE_EVT trans_id:5, conn_id:4, handle:42, is_prep:0, offset:0
DEBUG: app_ble_connected_app_api_registration: insert psk: 42
DEBUG: app_ble_connected_app_api_registration: psk_insert: 123456a@
BSA_trace 49@ 01/01 02h:05m:26s:604ms: BSA_BleSeSendRspInit
BSA_BLE_SE_WRITE_EVT: send_server_resp.conn_id:4, send_server_resp.trans_id:5
BSA_BLE_SE_WRITE_EVT: send_server_resp.status:0,send_server_resp.auth_req:0
BSA_BLE_SE_WRITE_EVT: send_server_resp.handle:42, send_server_resp.offset:0, send_server_resp.len:4
BSA_trace 50@ 01/01 02h:05m:26s:604ms: BSA_BleSeSendRsp

# wifi connection
DEBUG: app_ble_server_profile_cback: app_ble_server_profile_cback event = 17 
BSA_BLE_SE_WRITE_EVT status:0
BSA_trace 51@ 01/01 02h:05m:57s:745ms: Write value:
BSA_trace 52@ 01/01 02h:05m:57s:745ms:     0000: 01 01 10                                          ...             
Write value:, len:3
BSA_BLE_SE_WRITE_EVT trans_id:6, conn_id:4, handle:42, is_prep:0, offset:0
DEBUG: app_ble_connected_app_api_registration: registration CMD_BAV_CP_WIFI_CONNECT_REQUEST handle: 42
DEBUG: app_ble_curl_request: curl request handle: wifi-connect-request?ssid=VinBigdata-Guest&psk=123456a@
DEBUG: app_ble_curl_request: make request success
DEBUG: app_ble_connected_app_api_registration: response: {"status": "SUCCESS", "noti":"Wifi connected"}
DEBUG: app_ble_connected_app_api_registration: http_code: 200
DEBUG: app_ble_connected_app_api_registration: api_connect_wifi: wifi-connect-request?ssid=VinBigdata-Guest&psk=123456a@
BSA_trace 53@ 01/01 02h:06m:07s:871ms: BSA_BleSeSendRspInit
BSA_BLE_SE_WRITE_EVT: send_server_resp.conn_id:4, send_server_resp.trans_id:6
BSA_BLE_SE_WRITE_EVT: send_server_resp.status:0,send_server_resp.auth_req:0
BSA_BLE_SE_WRITE_EVT: send_server_resp.handle:42, send_server_resp.offset:0, send_server_resp.len:4
BSA_trace 54@ 01/01 02h:06m:07s:871ms: BSA_BleSeSendRsp
