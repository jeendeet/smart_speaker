//
//  bundle.h
//  WakeupWord
//
//  Created by Nhu Dinh Duc on 25/05/2022.
//

#ifndef bundle_h
#define bundle_h

#include "defs.h"
#ifdef USE_CIPHER
#ifdef USE_CONFIG

#include "cipher.h"
#include "config_reader.h"

class AESBundle {
public:
    AESBundle();
    AESBundle(std::string infoFile, std::string dataFile);
    virtual ~AESBundle();
    
    virtual std::vector<uint8_t>    readBytes(std::string key);
    virtual std::string             readString(std::string key);
    
    virtual void                    open(std::string infoFile, std::string dataFile);

    virtual void init();
    virtual void onRegisterConfig() = 0;
    virtual void onReadedConfig() = 0;
protected:
    std::string                                     infoFile;
    std::string                                     dataFile;
    std::map<std::string, std::pair<int, int>>      info;
    AESDecoder                                      decoder;
    config::AESParseOptions                         parseOption;
};

#endif /* USE_CONFIG */
#endif /* USE_CIPHER */

#endif /* bundle_h */
