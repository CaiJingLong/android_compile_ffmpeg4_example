package top.kikt.ffmpeg_sdl_example;

public class FFmpeg {

    static {
        System.loadLibrary("avcodec");
        System.loadLibrary("avfilter");
        System.loadLibrary("avformat");
        System.loadLibrary("avutil");
        System.loadLibrary("swresample");
        System.loadLibrary("swscale");
        System.loadLibrary("ffmpeg-bridge");
    }

    public static native void dumpInfo(String url);

}
