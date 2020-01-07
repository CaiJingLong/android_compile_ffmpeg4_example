package top.kikt.ffmpeg_sdl_example;

public class FFmpeg {

    static {
        System.loadLibrary("ffmpeg-bridge");
    }

    public static native void dumpInfo(String url);

    public static native void getFirstImage(String url, String dst);

}
