//
// Created by ducnd on 18/10/2021.
//

#ifndef WAKEUP_LOG_H
#define WAKEUP_LOG_H

#include <ctime>
#include <chrono>

#define LOG_DISABLE     0
#define LOG_ERROR       1
#define LOG_WARNING     2
#define LOG_DEBUG       3
#define LOG_INFO        4

extern int log_level;

inline void setLogLevel(int level) {
    log_level = level;
}

#ifdef __ANDROID__
// Log for android
#include <android/log.h>

#define TAG "WAKEUP"

#define LOGE(...) if(log_level > LOG_ERROR)     __android_log_print(ANDROID_LOG_ERROR,    TAG, __VA_ARGS__)
#define LOGW(...) if(log_level > LOG_WARNING)   __android_log_print(ANDROID_LOG_WARN,     TAG, __VA_ARGS__)
#define LOGD(...) if(log_level > LOG_DEBUG)     __android_log_print(ANDROID_LOG_DEBUG,    TAG, __VA_ARGS__)
#define LOGI(...) if(log_level > LOG_INFO)      __android_log_print(ANDROID_LOG_INFO,     TAG, __VA_ARGS__)

#else
// Log for normal application
#include <stdio.h>

#define LOGE(...) if(log_level > LOG_ERROR)     {printf(__VA_ARGS__); printf("\n");}
#define LOGW(...) if(log_level > LOG_WARNING)   {printf(__VA_ARGS__); printf("\n");}
#define LOGD(...) if(log_level > LOG_DEBUG)     {printf(__VA_ARGS__); printf("\n");}
#define LOGI(...) if(log_level > LOG_INFO)      {printf(__VA_ARGS__); printf("\n");}

#endif

#define NOW_MS() std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count()
#define NOW_US() std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count()
#define NOW_S() std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count()
#define NOW_M() std::chrono::duration_cast<std::chrono::minutes>(std::chrono::system_clock::now().time_since_epoch()).count()
#define NOW_H() std::chrono::duration_cast<std::chrono::hours>(std::chrono::system_clock::now().time_since_epoch()).count()

#endif //WAKEUP_LOG_H

