#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include <android/log.h>
#include "client/linux/handler/exception_handler.h"
#include "client/linux/handler/minidump_descriptor.h"

#define TAG_BREAKPADUTILS "breakpadutils"
#define PROJECT_TAG "SEMM"
#define DEFAULT_CRASH_DIR "/storage/emulated/0/sangfor/" //崩溃时dmp文件的默认存储目录

#define LOGI(tag, fmt, ...) __android_log_print(ANDROID_LOG_INFO, (tag), (fmt), ## __VA_ARGS__)
#define LOGD(tag, fmt, ...) __android_log_print(ANDROID_LOG_DEBUG, (tag), (fmt), ## __VA_ARGS__)
#define LOGW(tag, fmt, ...) __android_log_print(ANDROID_LOG_WARN, (tag), (fmt), ## __VA_ARGS__)
#define LOGE(tag, fmt, ...) __android_log_print(ANDROID_LOG_ERROR, (tag), (fmt), ## __VA_ARGS__)

namespace {

    static bool DumpCallback(const google_breakpad::MinidumpDescriptor &descriptor,
                             void *context, bool succeeded) {
        LOGI(TAG_BREAKPADUTILS, "Dump successed:%d path: %s\n", succeeded, descriptor.path());
        return succeeded;
    }

}  // namespace

//google_breakpad::MinidumpDescriptor descriptor("/storage/emulated/0/sangfor/");
//google_breakpad::ExceptionHandler eh(descriptor, NULL, DumpCallback,
//                                     NULL, true, -1);
//改成指针是为了能够控制初始化的时间，比如延迟初始化等等
google_breakpad::MinidumpDescriptor *g_pDescriptor = NULL;
google_breakpad::ExceptionHandler *g_pEH = NULL;
#ifdef __cplusplus
extern "C" {
#endif
JNIEXPORT void JNICALL
Java_cn_onlinecache_breakpad_NativeBreakpad_nativeInit(JNIEnv *env, jclass type, jstring path) {
    LOGI(TAG_BREAKPADUTILS, "init ");
    if (!path) {
        LOGE(TAG_BREAKPADUTILS, "log path is null.");
        return;
    }
    //设置奔溃日志存放的地方,如果不设置默认为sd卡根目录下breakpad目录，产生的文件名为crash-process-time.dmp
    const char *pathName = env->GetStringUTFChars(path, JNI_FALSE);
    if(pathName == nullptr){
        LOGE(TAG_BREAKPADUTILS,"path GetStringUTFChars ret null.");
        return;
    }
    
    if (g_pDescriptor == NULL) {
        g_pDescriptor = new google_breakpad::MinidumpDescriptor(pathName);
    }
    
    LOGI(TAG_BREAKPADUTILS, "setMinidumpDescriptor");
    if (g_pEH == NULL) {
        g_pEH = new google_breakpad::ExceptionHandler(*g_pDescriptor, NULL,
                                                      DumpCallback, NULL, true, -1);
    } else {
        g_pEH->set_minidump_descriptor(*g_pDescriptor);
    }
    out:
    env->ReleaseStringUTFChars(path, pathName);
}

#ifdef __cplusplus
}
#endif