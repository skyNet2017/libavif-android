package libavif;

public class AvifEncoder {
    static {
        System.loadLibrary("avif-jni");
    }

    public native boolean encode(String filePath,byte[] bytes);
}
