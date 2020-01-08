#include <jni.h>
#include <string>
#include <android/log.h>

constexpr int mjpegDen = 1000;

#ifdef __cplusplus
#if      __cplusplus
extern "C" {
#endif
#endif
#include <libavutil/avutil.h>
#include <libavformat/avformat.h>
#include <unistd.h>

void vlog(const char *fmt, ...) {
    va_list list;
    va_start(list, fmt);
    __android_log_vprint(ANDROID_LOG_INFO, "ffmpeg", fmt, list);
    va_end(list);
}

void logToAndroid(void *ptr, int level, const char *fmt, va_list vl) {
    if (level == AV_LOG_INFO) {
        __android_log_vprint(ANDROID_LOG_INFO, "ffmpeg", fmt, vl);
    } else if (level == AV_LOG_DEBUG) {
//        __android_log_vprint(ANDROID_LOG_DEBUG, "ffmpeg", fmt, vl);
    } else if (level == AV_LOG_WARNING) {
        __android_log_vprint(ANDROID_LOG_WARN, "ffmpeg", fmt, vl);
    } else if (level == AV_LOG_ERROR) {
        __android_log_vprint(ANDROID_LOG_ERROR, "ffmpeg", fmt, vl);
    } else if (level == AV_LOG_VERBOSE) {
//        __android_log_vprint(ANDROID_LOG_VERBOSE, "ffmpeg", fmt, vl);
    }
}

JNIEXPORT void JNICALL
Java_top_kikt_ffmpeg_1sdl_1example_FFmpeg_dumpInfo(JNIEnv *env, jclass clazz, jstring url) {
    av_log_set_callback(logToAndroid);
    AVFormatContext *context = nullptr;

    const char *cUrl = env->GetStringUTFChars(url, nullptr);

    avformat_open_input(&context, cUrl, nullptr, nullptr);

    av_dump_format(context, -1, cUrl, 0);

    if (context) {
        avformat_close_input(&context);
    }
}

AVFrame *getFirstFrame(AVFormatContext *context, int videoIndex) {
    AVCodecContext *codecContext = avcodec_alloc_context3(nullptr);

    AVStream *videoStream = context->streams[videoIndex];

    AVCodec *codec = avcodec_find_decoder(videoStream->codecpar->codec_id);

    avcodec_parameters_to_context(codecContext, videoStream->codecpar);

    avcodec_open2(codecContext, codec, nullptr);

    AVFrame *frame;
    AVPacket pkt;
    int ret;

    while (true) {
        frame = av_frame_alloc();
        ret = avcodec_receive_frame(codecContext, frame);
        if (ret == AVERROR(EAGAIN)) { // 应该继续发送包, 先从format里读
            ret = av_read_frame(context, &pkt);
            if (ret < 0) { // 没有数据了, 应该退出了
                break;
            }
            if (pkt.stream_index != videoIndex) {
                av_packet_unref(&pkt);
                continue;
            }
            ret = avcodec_send_packet(codecContext, &pkt);
            if (ret == AVERROR(EAGAIN)) { //写不进去, 理论上不可能, 但是输出下日志
                av_packet_unref(&pkt);
                continue;
            } else if (ret) {
                av_packet_unref(&pkt);
                continue;
            }
            // 到这里就可以理解为发送成功了, 释放包
            av_packet_unref(&pkt);
        } else if (ret) {
            // 出错, 先输出日志
            break;
        } else {
            // 获取到了数据
            break;
        }
    }

    avcodec_close(codecContext);

    if (&pkt) {
        av_packet_unref(&pkt);
    }

    av_log(nullptr, AV_LOG_INFO, "视频帧宽高: %d x %d", frame->width, frame->height);

    return frame;
}

void convertFrameAndSaveToFile(AVFrame *frame, const char *url) {
    int ret;
    AVCodec *encoder = avcodec_find_encoder(AV_CODEC_ID_MJPEG);

    if (!encoder) {
        vlog("没有找到jpg编码器");
        return;
    }

    AVCodecContext *encoderContext = avcodec_alloc_context3(encoder);

    encoderContext->width = frame->width;
    encoderContext->height = frame->height;
    encoderContext->time_base.num = 1;
    encoderContext->time_base.den = mjpegDen;
    encoderContext->pix_fmt = AV_PIX_FMT_YUVJ420P;
    encoderContext->codec_id = encoder->id;
    encoderContext->codec_type = encoder->type;

    ret = avcodec_open2(encoderContext, encoder, nullptr);
    if (ret) {
        const char *errorMsg = av_err2str(ret);
        vlog("开启编码器失败, %s ,%d", errorMsg, ret);
        return;
    }

    AVFormatContext *outputContext = nullptr;
    AVOutputFormat *fmt = av_guess_format("mjpeg", nullptr, nullptr);
    if (!fmt) {
        vlog("没找到format");
        return;
    }

    ret = avformat_alloc_output_context2(&outputContext, fmt, nullptr, url);
    if (ret) {
        const char *errorMsg = av_err2str(ret);
        vlog("申请outputContext空间失败, %s, %d", errorMsg, ret);
        return;
    }
    avio_open(&outputContext->pb, url, AVIO_FLAG_WRITE);
    AVStream *outputStream = avformat_new_stream(outputContext, nullptr);

    outputStream->codecpar->codec_id = encoder->id;
    outputStream->codecpar->codec_type = encoder->type;
    outputStream->codecpar->width = frame->width;
    outputStream->codecpar->height = frame->height;

    avformat_write_header(outputContext, nullptr);

    avcodec_send_frame(encoderContext, frame);

    AVPacket *pkt;

    pkt = av_packet_alloc();

    avcodec_receive_packet(encoderContext, pkt);

    pkt->stream_index = outputStream->index;
    av_write_frame(outputContext, pkt);
    av_write_trailer(outputContext);

    av_packet_unref(pkt);
    avcodec_close(encoderContext);
    avio_close(outputContext->pb);
    avformat_free_context(outputContext);
}

JNIEXPORT void JNICALL
Java_top_kikt_ffmpeg_1sdl_1example_FFmpeg_getFirstImage(JNIEnv *env, jclass clazz, jstring javaUrl,
                                                        jstring dst) {
    av_log_set_callback(logToAndroid);

    AVFormatContext *context = nullptr;
    const char *url = env->GetStringUTFChars(javaUrl, nullptr);

    avformat_open_input(&context, url, nullptr, nullptr);

    int videoIndex = av_find_best_stream(context, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
    if (videoIndex == -1) {
        return;
    }

    const char *cDst = env->GetStringUTFChars(dst, nullptr);

    if (access(cDst, F_OK) != -1) {
        // 存在
        remove(cDst);
    }

    AVFrame *pFrame = getFirstFrame(context, videoIndex);

    av_log(nullptr, AV_LOG_INFO, "视频帧宽高: %d, %d", pFrame->width, pFrame->height);
//
    // 读取第一帧
    if (pFrame) {
        convertFrameAndSaveToFile(pFrame, cDst);
    }

    if (pFrame) {
        av_frame_free(&pFrame);
    }


}

#ifdef __cplusplus
#if      __cplusplus
}
#endif
#endif