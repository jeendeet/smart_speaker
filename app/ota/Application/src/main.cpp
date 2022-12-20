//
// Created by vnbk on 31/07/2018.
//
#include "OtaClient.h"

int main(int argc, char** argv) {
    if(argc <= 3) {
        std::shared_ptr<ota::OtaClient> client = ota::OtaClient::create(argv[1], argv[2]);
        if (!client) {
            printf("Error: Main: Donot create Client\n");
            return EXIT_FAILURE;
        }
        //client->setState(ota::OtaClientState::PREPARE_UPLOAD);
        client->run();
    }
    return EXIT_SUCCESS;
}