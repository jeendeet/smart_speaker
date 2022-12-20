#ifndef __HTTPRESPONSE__
#define __HTTPRESPONSE__

/* Define https response */

class HttpResponse
{
    public:
        static HttpResponse* Instance();
        char* create_response(char* status, char* content);
    private:
        static HttpResponse* _instance;
        HttpResponse();
        ~HttpResponse();

};

#endif /* __HTTPRESPONSE__ */
