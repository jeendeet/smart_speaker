//
// Created by ducnd on 27/10/2020.
//

#ifndef C_CIPHER_H
#define C_CIPHER_H

#include "defs.h"
#ifdef USE_CIPHER

#include <botan/block_cipher.h>
#include <string>
#include <memory>
#include <iostream>

#define EN_BLOCK_SIZE 31
#define DE_BLOCK_SIZE 32

class AESDecoder {
public:
    AESDecoder();
    ~AESDecoder();
    std::string             decodeToString(std::string file);
    std::vector<uint8_t>    decodeToBytes(std::string file);

private:
    std::unique_ptr<Botan::BlockCipher> cipher;
};

#endif
#endif //C_CIPHER_H
