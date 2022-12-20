#include "defs.h"
#ifdef USE_CIPHER

#include "cipher.h"
#include <botan/hex.h>
#include <fstream>
#include <iostream>
#include <sstream>

AESDecoder::AESDecoder() {
    std::vector<uint8_t> key = Botan::hex_decode("abcd02030405060bcad90A42310D0E0F123412131415161718191A1B1C1D1E1F");
    this->cipher = Botan::BlockCipher::create("AES-256");
    this->cipher->set_key(key);
}

AESDecoder::~AESDecoder() {

}

std::string AESDecoder::decodeToString(std::string file) {
    std::vector<uint8_t> bytes = decodeToBytes(file);
    std::string result(bytes.begin(), bytes.end());

    return result;
}

std::vector<uint8_t> AESDecoder::decodeToBytes(std::string file) {
    std::vector<uint8_t> result;
    char buffer[DE_BLOCK_SIZE];

    // Open the file to decrypt
    std::ifstream infile;
    infile.open(file, std::ios::binary | std::ios::in);

    while(!infile.eof()) {
        // Read the block
        infile.read(buffer, DE_BLOCK_SIZE);

        if(infile.gcount() != DE_BLOCK_SIZE) {
            break;
        }

        // Decrypt data
        std::vector<uint8_t> block(buffer, buffer + DE_BLOCK_SIZE);
        std::vector<uint8_t> out(DE_BLOCK_SIZE);
        this->cipher->decrypt(block, out);

        // Get padding length
        uint8_t padding_length = out[out.size() - 1];

        // Append data to result
        result.insert(result.end(), out.begin(), out.end() - 1 - padding_length);
    }
    infile.close();

    return result;
}

#endif