#include <string>
#include <aubio/aubio.h>
#include <cstdio>

#include "common/defs.h"
#include "main_process.h"
#include "common/utils.h"
#include "common/log.h"

#ifdef ANDROID
#include <jni.h>
#endif

int log_level = 4;
// Main object to process
E2EMainProcess* e2eMainProcess = nullptr;

extern "C" {
    void setLogLevel(int level) {
        log_level = level;
    }

    void initSiriFirst(const char* path) {
        LOGD("Siri path : %s", path);

        std::string strPath(path);
        e2eMainProcess = new E2EMainProcess(strPath);
    }

    void initSiriSecond(const char* path) {
        LOGD("CRA path : %s", path);

        std::string strPath(path);
        e2eMainProcess->initVerifier(strPath);
    }

    bool acceptWaveform(const char *data, int len) {
//        LOGD("Len : %d\n", len);
        short wave[len / 2];
        for (int i = 0; i < len / 2; i++) {
            wave[i] = *(((short *)data) + i);
//            LOGD("data[%i] = %d", i, wave[i]);
        }

        auto input = std::tuple<short*, size_t>(wave, len / 2);
        bool result = e2eMainProcess->acceptWaveform(input);

        return result;
    }

    void acceptWaveformForContext(const char *data, int len) {
//        LOGD("Len : %d\n", len);
        short wave[len / 2];
        for (int i = 0; i < len / 2; i++) {
            wave[i] = *(((short *)data) + i);
//            LOGD("data[%i] = %d", i, wave[i]);
        }


        auto input = std::tuple<short*, size_t>(wave, len / 2);
        e2eMainProcess->acceptWaveformForContext(input);
    }

    void setVerifyThreshold(float threshold) {
        e2eMainProcess->setVerifyThreshold(threshold);
    }

    void setThreshold(float threshold) {
        e2eMainProcess->setThreshold(threshold);
    }

    void setVadThreshold(float threshold) {
        e2eMainProcess->setVADThreshold(threshold);
    }

    void setVadEnable(bool enable) {
        e2eMainProcess->setVADEnable(enable);
    }

    void setAutoGain(bool use) {
        e2eMainProcess->setAutoGain(use);
    }

    void setAutoGainTarget(float target) {
        e2eMainProcess->setAutoGainTarget(target);
    }

    void release() {
        delete e2eMainProcess;
        e2eMainProcess = nullptr;
    }
}



#ifdef ANDROID

extern "C"
JNIEXPORT void JNICALL
Java_com_vinbdi_wakeup_WakeupLog_setLogLevel(JNIEnv *env, jclass clazz, jint level) {
    setLogLevel(level);
}

/**************************************************************************************************/
/******************************************* E2E BASED ********************************************/
/**************************************************************************************************/

extern "C" JNIEXPORT void JNICALL
Java_com_vinbdi_wakeup_SiriArchWuW_initSiriFirst(
        JNIEnv* env,
        jobject /* this */,
        jstring path) {

    std::string strPath = jstring2string(env, path);
    e2eMainProcess = new E2EMainProcess(strPath);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_vinbdi_wakeup_SiriArchWuW_initSiriSecond(JNIEnv *env, jobject thiz, jstring path) {
    std::string strPath = jstring2string(env, path);
    e2eMainProcess->initVerifier(strPath);
}

extern "C" JNIEXPORT void JNICALL
Java_com_vinbdi_wakeup_SiriArchWuW_release(
        JNIEnv* env,
        jobject /* this */) {
    delete e2eMainProcess;
    e2eMainProcess = nullptr;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_vinbdi_wakeup_SiriArchWuW_acceptWaveform(
        JNIEnv* env,
        jobject /* this */,
        jshortArray data) {

    int length = env->GetArrayLength(data);
    short* wave = new short[length];
    jshort* wave_ = env->GetShortArrayElements(data, NULL);
    for (int i = 0; i < length; i++) {
        wave[i] = wave_[i];
    }

    auto input = std::tuple<short*, size_t>(wave, length);
    bool result = e2eMainProcess->acceptWaveform(input);

    env->ReleaseShortArrayElements(data, wave_, 0);

    delete[] wave;

    return result;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_vinbdi_wakeup_SiriArchWuW_setVerifyThreshold(JNIEnv *env, jobject thiz, jfloat threshold) {
    e2eMainProcess->setVerifyThreshold((float) threshold);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_vinbdi_wakeup_SiriArchWuW_setThreshold(JNIEnv *env, jobject thiz, jfloat threshold) {
    e2eMainProcess->setThreshold((float) threshold);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_vinbdi_wakeup_SiriArchWuW_acceptWaveformForContext(JNIEnv *env, jobject thiz, jshortArray data) {
    int length = env->GetArrayLength(data);
    short* wave = new short[length];
    jshort* wave_ = env->GetShortArrayElements(data, NULL);
    for (int i = 0; i < length; i++) {
        wave[i] = wave_[i];
    }

    auto input = std::tuple<short*, size_t>(wave, length);
    e2eMainProcess->acceptWaveformForContext(input);

    env->ReleaseShortArrayElements(data, wave_, 0);

    delete[] wave;
}

extern "C"
JNIEXPORT void JNICALL
Java_com_vinbdi_wakeup_SiriArchWuW_setVadThreshold(JNIEnv *env, jobject thiz, jfloat threshold) {
    e2eMainProcess->setVADThreshold(threshold);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_vinbdi_wakeup_SiriArchWuW_setVadEnable(JNIEnv *env, jobject thiz, jboolean enable) {
    e2eMainProcess->setVADEnable(enable);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_vinbdi_wakeup_SiriArchWuW_setAutoGain(JNIEnv *env, jobject thiz, jboolean use) {
    e2eMainProcess->setAutoGain(use);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_vinbdi_wakeup_SiriArchWuW_setAutoGainTarget(JNIEnv *env, jobject thiz, jfloat target) {
    e2eMainProcess->setAutoGainTarget(target);
}

/**************************************************************************************************/
/****************************************** KALDI BASED *******************************************/
/**************************************************************************************************/

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_vinbdi_wakeup_KaldiArchWuw_acceptWaveform(JNIEnv *env, jobject thiz, jshortArray data) {
#ifdef USE_KALDI
#endif
}
extern "C"
JNIEXPORT void JNICALL
Java_com_vinbdi_wakeup_KaldiArchWuw_acceptWaveformForContext(JNIEnv *env, jobject thiz, jshortArray data) {
#ifdef USE_KALDI
#endif
}
extern "C"
JNIEXPORT void JNICALL
Java_com_vinbdi_wakeup_KaldiArchWuw_initModel(JNIEnv *env, jobject thiz, jstring path) {
#ifdef USE_KALDI
#endif
}
extern "C"
JNIEXPORT void JNICALL
Java_com_vinbdi_wakeup_KaldiArchWuw_setThreshold(JNIEnv *env, jobject thiz, jfloat threshold) {
#ifdef USE_KALDI
#endif
}
extern "C"
JNIEXPORT void JNICALL
Java_com_vinbdi_wakeup_KaldiArchWuw_setVerifyThreshold(JNIEnv *env, jobject thiz, jfloat threshold) {
#ifdef USE_KALDI
#endif
}
extern "C"
JNIEXPORT void JNICALL
Java_com_vinbdi_wakeup_KaldiArchWuw_release(JNIEnv *env, jobject thiz) {
#ifdef USE_KALDI
#endif
}

extern "C"
JNIEXPORT void JNICALL
Java_com_vinbdi_wakeup_KaldiArchWuw_setVadThreshold(JNIEnv *env, jobject thiz, jfloat threshold) {
#ifdef USE_KALDI
#endif
}
extern "C"
JNIEXPORT void JNICALL
Java_com_vinbdi_wakeup_KaldiArchWuw_setVadEnable(JNIEnv *env, jobject thiz, jboolean enable) {
#ifdef USE_KALDI
#endif
}

extern "C"
JNIEXPORT void JNICALL
Java_com_vinbdi_wakeup_KaldiArchWuw_setAutoGain(JNIEnv *env, jobject thiz, jboolean use) {
#ifdef USE_KALDI
#endif
}
extern "C"
JNIEXPORT void JNICALL
Java_com_vinbdi_wakeup_KaldiArchWuw_setAutoGainTarget(JNIEnv *env, jobject thiz, jfloat target) {
#ifdef USE_KALDI
#endif
}

#endif