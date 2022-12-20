//
// Created by ducnd on 04/11/2020.
//

#ifndef VOSK_ANDROID_DEMO_EXCEPTION_H
#define VOSK_ANDROID_DEMO_EXCEPTION_H
#include <string>
#include <exception>

class IOException: public std::exception {
private:
    std::string message;
public:
    IOException(const std::string& message) : message(message) {}
    ~IOException() {}
    virtual const char* what() const throw() {
        return message.c_str();
    }
};

class InferenceException: public std::exception {
private:
    std::string message;
public:
    InferenceException(const std::string& message) : message(message) {}
    ~InferenceException() {}
    virtual const char* what() const throw() {
        return message.c_str();
    }
};
#endif //VOSK_ANDROID_DEMO_EXCEPTION_H
