#ifndef __METADATAREQUEST__
#define __METADATAREQUEST__

/* Define Meta Request */
#include <iostream>

class MetaDataRequest
{
    public:
        static MetaDataRequest* Instance();
        std::string metadata_request();
        std::string ipaddress_request(bool &_result);
    private:
        static MetaDataRequest* _instance;
        MetaDataRequest();
        ~MetaDataRequest();
};

#endif /* __METADATAREQUEST__ */