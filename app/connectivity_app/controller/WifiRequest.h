#ifndef __WIFIREQUEST__
#define __WIFIREQUEST__

/* Define Wifi Request */

class WifiRequest
{
    public:
        static WifiRequest* Instance();
        std::string wifi_connect_request(char* ssid, char* psk);
        std::string wifi_scanner(bool &_result);
        char *wifi_checking(std::string text);
    private:
        static WifiRequest* _instance;
        WifiRequest();
        ~WifiRequest();
};

#endif /* __WIFIREQUEST__ */