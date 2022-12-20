//
// Created by vnbk on 18/07/2018.
//

#include <openssl/md5.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>

#include "FileWraper.h"

namespace ota{
FILE* FileWraper::open(const std::string& nameFile, const std::string &mode) {
    if(nameFile.empty() || mode.empty())
        return nullptr;
    return fopen(nameFile.c_str(), mode.c_str());
}

int FileWraper::close(FILE *file) {
    return fclose(file);
}

bool FileWraper::extract(const std::string &nameFile, const std::string &pathToDirExtract) {
    std::string cmd = "unzip -o " + nameFile + " -d " + pathToDirExtract;
    int status = system(cmd.c_str());
    if(WIFEXITED(status)){
        if(WEXITSTATUS(status) == 0){
            printf("File: Extract file successfully\n");
            return true;
        }
    }
    return false;
}

/*@nameFile: Ten file muon nen(bao gom ca duong dan den file)
 * @pathFileCompress: Duong dan den thu muc chua file duoc nen
 * @nameFileCompress: Ten file duoc nen (khong bao gom duong dan)
 * @format: Dinh dang file nen, mac dinh .zip*/

std::string FileWraper::compress(const std::string &nameFile,const std::string& pathFileCompress, const std::string& nameFileCompress) {
    std::string result = "";
    char pwd[PATH_MAX];
    getcwd(pwd, PATH_MAX);
    chdir(pathFileCompress.c_str());
    std::string cmd = "zip -r " + nameFile + ".zip" + " " + nameFileCompress;
    int status = system(cmd.c_str());
    if(WIFEXITED(status)){
        if(WEXITSTATUS(status) == 0){
            printf("File: Compress file successfully\n");
            result = nameFile + ".zip";
        }
    }
    chdir(pwd);

    return result;
}

bool FileWraper::isValid(const std::string &pathToFile, const std::string &checksum) {
    std::ifstream file(pathToFile, std::ifstream::binary);
    MD5_CTX md5Context;
    MD5_Init(&md5Context);
    char buf[1024 * 16];
    while (file.good()) {
        file.read(buf, sizeof(buf));
        MD5_Update(&md5Context, buf, file.gcount());
    }
    unsigned char result[MD5_DIGEST_LENGTH];
    MD5_Final(result, &md5Context);
    std::ostringstream convert;
    for (int i=0; i < MD5_DIGEST_LENGTH; i++) {
        convert << hex << setw(2) << setfill('0') << (int) result[i];
    }
    printf("FileWraper::isValid:: Checksum value calculate = %s\n", convert.str().c_str());
    if(!checksum.compare(convert.str().c_str())){
        printf("Checksum value of file = %s\n", convert.str().c_str());
        return true;
    }
    printf("Error: File: calculateChecksum: Checksum false\n");
    return false;
}

bool FileWraper::executed(const std::string& path, const std::string &nameFile) {
    bool result = false;
    std::ifstream updaterFile((path + nameFile).c_str());
    if(!updaterFile.good()){
        printf("Error: File: isValid: Updater.sh file not exist\n");
        return false;
    }
    if(!chmod((path + nameFile).c_str(), S_IRWXU|S_IRWXG)) {
        char pwd[PATH_MAX];
        getcwd(pwd, PATH_MAX);
        chdir(path.c_str());
        int status = system((path + nameFile).c_str());
        if (WIFEXITED(status)) {
            if (WEXITSTATUS(status) == 0) {
                printf("File: Executed file successfully!!!!\n");
                result = true;
            }
        }
        chdir(pwd);
    }
    return result;
}

bool FileWraper::copy(const std::string &src, const std::string &dest) {
    std::string cmd = "cp -f -R " + src + " " + dest;
    int status = system(cmd.c_str());
    if(WIFEXITED(status)){
        if(WEXITSTATUS(status) == 0){
            printf("File: copy file successfully\n");
            return true;
        }
    }
    return false;
}

bool FileWraper::isDirectoryEmpty(const std::string &path) {
    struct dirent *ent;
    int ret = 1;

    DIR *d = opendir(path.c_str());
    if (!d) {
        return 1;
    }

    while ((ent = readdir(d))) {
        if (!strcmp(ent->d_name, ".") || !(strcmp(ent->d_name, "..")))
            continue;
        ret = 0;
        break;
    }
    closedir(d);
    return ret;
}

std::string FileWraper::findFileInDirectory(const std::string &pathDirectory) {
    struct dirent *ent;
    std::string name = "";

    DIR *d = opendir(pathDirectory.c_str());
    if (!d) {
        return name;
    }

    while ((ent = readdir(d))) {
        if (!strcmp(ent->d_name, ".") || !(strcmp(ent->d_name, "..")))
            continue;
        name = ent->d_name;
        break;
    }
    closedir(d);
    return name;
}

void FileWraper::listFileInDirectory(const std::string &pathDirectory, std::vector<std::string> &listFile) {
    struct dirent *ent;

    DIR *d = opendir(pathDirectory.c_str());
    if (!d) {
        return;
    }

    while ((ent = readdir(d))) {
        if (!strcmp(ent->d_name, ".") || !(strcmp(ent->d_name, "..")))
            continue;
       listFile.push_back(ent->d_name);
    }
    closedir(d);
}

bool FileWraper::remove(const std::string &nameFile) {
    if(nameFile.empty())
        return false;
    std::string cmd = "rm -rf " + nameFile;
    int status = system(cmd.c_str());
    if(WIFEXITED(status)){
        if(WEXITSTATUS(status) == 0){
            printf("File: remove file successfully\n");
            return true;
        }
    }
    return false;
}

bool FileWraper::removeAll(const std::string &path) {
    if(path.empty())
        return false;
    std::string cmd = "rm -rf " + path + "*";
    int status = system(cmd.c_str());
    if(WIFEXITED(status)){
        if(WEXITSTATUS(status) == 0){
            printf("File: remove file successfully\n");
            return true;
        }
    }
    return false;
}

bool FileWraper::removeExcept(const std::string &nameFile) {
    bool result = false;
    if(nameFile.empty())
        return false;
    std::string cmd = "rm -rf !(" + nameFile + ")";
    int status = system(cmd.c_str());
    if(WIFEXITED(status)){
        if(WEXITSTATUS(status) == 0){
            printf("File: remove file successfully\n");
            result = true;
        }
    }
    return result;
}

bool FileWraper::move(const std::string &src, const std::string &dest) {
    if(src.empty() || dest.empty())
        return false;
    std::string cmd = "mv -v " + src + " " + dest;
    int status = system(cmd.c_str());
    if(WIFEXITED(status)){
        if(WEXITSTATUS(status) == 0){
            printf("File: remove file successfully\n");
            return true;
        }
    }
    return false;
}

bool FileWraper::isFileEmpty(const std::string &nameFile) {
    std::ifstream file((nameFile).c_str());
    return file.peek() == std::ifstream::traits_type::eof();
}

}