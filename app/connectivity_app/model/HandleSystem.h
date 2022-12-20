#ifndef __HANDLESYSTEM__
#define __HANDLESYSTEM__

#include <iostream>
/* Define Handle system */

class HandleSystem
{
    public:
        static HandleSystem* Instance();
        std::string exec(const char* cmd);
        std::string getMacAddress(std::string interface);
        std::string authenticate();
    private:
        static HandleSystem* _instance;
        HandleSystem();
        ~HandleSystem();

};

#endif /* __HANDLESYSTEM__ */