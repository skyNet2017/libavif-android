package libavif;

import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.util.Log;

import java.io.File;
import java.nio.ByteBuffer;

public class AvifEncoder {
    static {
        try {
            System.loadLibrary("avif-jni");
        }catch (Throwable throwable){
            throwable.printStackTrace();
        }

    }

    public static boolean encodeToAvif(String  inputPath,String outputPath){
        //Bitmap像素点的色彩通道排列顺序是RGBA
        Bitmap bitmap = BitmapFactory.decodeFile(inputPath);
        byte[] bytes = bitmap2RGBA(bitmap);
        int code =  encode2(bytes,bitmap.getWidth(),bitmap.getHeight(),bitmap.getRowBytes(),outputPath);
        Log.w("result","code:"+code);
        return code==0;


    }

    public static native int encode(byte[] bitmapBytesRGBA,String outPath);

    public static native int encode2(byte[] bitmapBytesRGBA,int width,int height,long rowBytes,String outPath);

    public static byte[] bitmap2RGB(Bitmap bitmap) {
        int bytes = bitmap.getByteCount();  //返回可用于储存此位图像素的最小字节数

        ByteBuffer buffer = ByteBuffer.allocate(bytes); //  使用allocate()静态方法创建字节缓冲区
        bitmap.copyPixelsToBuffer(buffer); // 将位图的像素复制到指定的缓冲区

        byte[] rgba = buffer.array();
        byte[] pixels = new byte[(rgba.length / 4) * 3];

        int count = rgba.length / 4;

        //Bitmap像素点的色彩通道排列顺序是RGBA
        for (int i = 0; i < count; i++) {

            pixels[i * 3] = rgba[i * 4];        //R
            pixels[i * 3 + 1] = rgba[i * 4 + 1];    //G
            pixels[i * 3 + 2] = rgba[i * 4 + 2];       //B

        }

        return pixels;
    }
    public static byte[] bitmap2RGBA(Bitmap bitmap) {
        int bytes = bitmap.getByteCount();  //返回可用于储存此位图像素的最小字节数

        ByteBuffer buffer = ByteBuffer.allocate(bytes); //  使用allocate()静态方法创建字节缓冲区
        bitmap.copyPixelsToBuffer(buffer); // 将位图的像素复制到指定的缓冲区

        byte[] rgba = buffer.array();


        return rgba;
    }

}
