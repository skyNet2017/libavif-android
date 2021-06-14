package com.nmmedit.libavif;

import androidx.appcompat.app.AppCompatActivity;

import android.graphics.drawable.Drawable;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.ImageView;

import com.bumptech.glide.Glide;
import com.bumptech.glide.RequestBuilder;

import org.devio.takephoto.wrap.TakeOnePhotoListener;
import org.devio.takephoto.wrap.TakePhotoUtil;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;

import libavif.AvifEncoder;

public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        try {
            final ImageView avifImage = findViewById(R.id.avif_img);
            final byte[] bytes = inputStreamToBytes(getAssets().open("test.avif"));
            Glide.with(this)
                    .load(bytes)
                    .into(avifImage);

            final ImageView avifsImage = findViewById(R.id.avifs_img);
            final byte[] bytes2 = inputStreamToBytes(getAssets().open("test.avifs"));
            Glide.with(this)
                    .load(bytes2)
                    .into(avifsImage);
        } catch (IOException e) {

        }
    }

    private byte[] inputStreamToBytes(InputStream is) {
        ByteArrayOutputStream buffer = new ByteArrayOutputStream(64 * 1024);
        try {
            int nRead;
            byte[] data = new byte[16 * 1024];
            while ((nRead = is.read(data)) != -1) {
                buffer.write(data, 0, nRead);
            }
            buffer.close();
        } catch (IOException e) {
            return null;
        }
        return buffer.toByteArray();
    }

    public void select(View view) {
        TakePhotoUtil.startPickOne(this, true, new TakeOnePhotoListener() {
            @Override
            public void onSuccess(String path) {
                Log.w("onsuccess","path:"+path);
                File file = new File(path);
                File file1 = new File(file.getParentFile(),file.getName()+".avif");
                try {
                  boolean success =   AvifEncoder.encodeToAvif(path,file1.getAbsolutePath());
                    Log.w("result","success:"+success);
                }catch (Throwable throwable){
                    throwable.printStackTrace();
                }
                Log.w("result",file1.length()/1024+"k,"+file1.getAbsolutePath());



            }

            @Override
            public void onFail(String path, String msg) {

            }

            @Override
            public void onCancel() {

            }
        });

    }
}