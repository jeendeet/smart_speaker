//
// Created by vnbk on 18/07/2018.
//

#ifndef OTA_V1_FILEWRAPER_H
#define OTA_V1_FILEWRAPER_H

#include <memory>
#include <stdio.h>
#include <vector>

namespace ota {
using namespace std;
class FileWraper {
public:
    static FILE* open(const std::string& nameFile, const std::string& mode);
    static int close(FILE* file);
    static bool extract(const std::string& nameFile, const std::string& pathToDirExtract);
    static std::string compress(const std::string& nameFile, const std::string& pathToDirCompress, const std::string& nameFileCompress);

    static bool isValid(const std::string& nameFile, const std::string& param);
    static bool executed(const std::string& path, const std::string& nameFile);
    static bool copy(const std::string& src, const std::string& dest);
    static bool isDirectoryEmpty(const std::string& path);
    static std::string findFileInDirectory(const std::string& pathDirectory);
    static void listFileInDirectory(const std::string& pathDirectory, std::vector<std::string>& listFile);
    static bool remove(const std::string& nameFile);
    static bool removeAll(const std::string& path);
    static bool removeExcept(const std::string& nameFile);
    static bool move(const std::string& src, const std::string& dest);

    static bool isFileEmpty(const std::string& nameFile);
};
}

#endif //OTA_V1_FILEWRAPER_H
