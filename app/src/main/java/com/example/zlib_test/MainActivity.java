package com.example.zlib_test;

import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.os.Environment;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.TypedValue;
public class MainActivity extends AppCompatActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        System.loadLibrary("glz-debug");
        setContentView(R.layout.activity_main);

        Bitmap bitmap1 = getBitmap(this, R.drawable.before);
//        Bitmap bitmap2 = getBitmap(this, R.drawable.after2);
//        Bitmap bitmap3 = getBitmap(this, R.drawable.diff);
//        Bitmap desktop = getBitmap(this, R.drawable.desktop);
//        Bitmap word = getBitmap(this, R.drawable.word);
//        Bitmap webpage = getBitmap(this, R.drawable.webpage);
        Bitmap video = getBitmap(this, R.drawable.video);
//        Bitmap puredesktop = getBitmap(this, R.drawable.puredesktop);
        String path = Environment.getExternalStorageDirectory().getAbsolutePath();
        path = path + "/a.jpg";

        //EncodeImage(bitmap1,video, bitmap1.getWidth(), bitmap1.getHeight(), path);
        JpegEncode(bitmap1, video, bitmap1.getWidth(), bitmap1.getHeight(), path);

    }
    public static native void EncodeImage(Bitmap bitmap1, Bitmap bitmap2, int width, int height, String path);
    public static native void JpegEncode(Bitmap bitmap1, Bitmap bitmap2, int width, int height, String path);

    public static Bitmap getBitmap(Context context, int resId) {
        BitmapFactory.Options options = new BitmapFactory.Options();
        TypedValue value = new TypedValue();
        context.getResources().openRawResource(resId, value);
        options.inTargetDensity = value.density;
        options.inScaled=false;//不缩放
        return BitmapFactory.decodeResource(context.getResources(), resId, options)
                .copy(Bitmap.Config.ARGB_8888, true);
    }

}
