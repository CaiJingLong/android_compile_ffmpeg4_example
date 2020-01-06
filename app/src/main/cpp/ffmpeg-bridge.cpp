#include <jni.h>
#include <string>
#include <android/log.h>

#ifdef __cplusplus
#if      __cplusplus
extern "C" {
#endif
#endif
#include <libavutil/avutil.h>
#include <libavformat/avformat.h>
#ifdef __cplusplus
#if      __cplusplus
}
#endif
#endif

extern "C" void logToAndroid(void *ptr, int level, const char *fmt, va_list vl) {
    __android_log_vprint(ANDROID_LOG_INFO, "ffmpeg", fmt, vl);
}

extern "C" JNIEXPORT jstring JNICALL
Java_top_kikt_ffmpeg_1sdl_1example_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
}

extern "C"
JNIEXPORT void JNICALL
Java_top_kikt_ffmpeg_1sdl_1example_FFmpeg_dumpInfo(JNIEnv *env, jclass clazz, jstring url) {
    av_log_set_callback(logToAndroid);
    AVFormatContext *context = nullptr;

    const char *cUrl = (*env).GetStringUTFChars(url, nullptr);

    avformat_open_input(&context, cUrl, nullptr, nullptr);

    if (context) {
        avformat_close_input(&context);
    }
}