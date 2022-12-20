//
// Created by ducnd on 19/10/2021.
//

#ifndef WAKEUP_UTILS_H
#define WAKEUP_UTILS_H

#ifdef ANDROID
#include <jni.h>
#endif

#include <string>

#ifdef ANDROID
std::string jstring2string(JNIEnv *env, jstring jStr);
#endif

#endif //WAKEUP_UTILS_H
