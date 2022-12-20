//
// Created by vnbk on 24/10/2018.
//

#ifndef OTA_HC_V1_OTADEFINES_H
#define OTA_HC_V1_OTADEFINES_H

namespace ota {

enum class OtaClientState{
    INITIAL,
    CONNECTING,
    IDLE,
    PREPARE_DOWNLOAD,
    DOWNLOADING,
    PREPARE_UPLOAD,
    UPLOADING
};

enum class ErrorCode {
    ERR_SUCCESSFULL = 0,
    ERR_FILE_INVALID = 1,
    ERR_FORMAT_FILE = 2,
    ERR_DOWNLOAD_FAIL = 3,
    ERR_TIMEOUT = 4,
    ERR_INTERNAL = 5
};

enum class OtaMessageStatus{
    STT_DOWNLOAD = 1,
    STT_SUCCESSFULLY = 2,
    STT_ERROR = 3
};

}

#endif //OTA_HC_V1_OTADEFINES_H
