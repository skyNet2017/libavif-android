
//
// Created by mao on 21-4-27.
//

#include <jni.h>
#include <stdio.h>
#include "memory.h"
#include <android/log.h>
#include "avif/avif.h"

#include "util.h"
#define LOG_TAG "Luban"

#define LOG_I(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOG_E(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

static jint
Java_libavif_AvifImage_getWidth0(JNIEnv *env, jclass clazz, jlong n_image) {
    avifImage *image = jlong_to_ptr(n_image);
    return image->width;
}

static jint
Java_libavif_AvifImage_getHeight0(JNIEnv *env, jclass clazz, jlong n_image) {
    avifImage *image = jlong_to_ptr(n_image);
    return image->height;
}

static jint
Java_libavif_AvifImage_getDepth0(JNIEnv *env, jclass clazz, jlong n_image) {
    avifImage *image = jlong_to_ptr(n_image);
    return image->depth;
}

static jint
Java_libavif_AvifImage_getYuvFormat0(JNIEnv *env, jclass clazz, jlong n_image) {
    avifImage *image = jlong_to_ptr(n_image);
    return image->yuvFormat;
}

static jint
Java_libavif_AvifImage_getYuvRange0(JNIEnv *env, jclass clazz, jlong n_image) {
    avifImage *image = jlong_to_ptr(n_image);
    return image->yuvRange;
}


static JNINativeMethod methods[] = {
        {"getWidth0",     "(J)I", (void *) Java_libavif_AvifImage_getWidth0},

        {"getHeight0",    "(J)I", (void *) Java_libavif_AvifImage_getHeight0},

        {"getDepth0",     "(J)I", (void *) Java_libavif_AvifImage_getDepth0},

        {"getYuvFormat0", "(J)I", (void *) Java_libavif_AvifImage_getYuvFormat0},

        {"getYuvRange0",  "(J)I", (void *) Java_libavif_AvifImage_getYuvRange0},

};


#define NELEM(x) ((int) (sizeof(x) / sizeof((x)[0])))

jboolean registerImageNativeMethods(JNIEnv *env) {
    jclass clazz = (*env)->FindClass(env, "libavif/AvifImage");
    if (clazz == NULL) {
        return JNI_FALSE;
    }
    if ((*env)->RegisterNatives(env, clazz, methods, NELEM(methods)) < 0) {
        return JNI_FALSE;
    }

    return JNI_TRUE;
}

JNIEXPORT jint JNICALL
Java_libavif_AvifEncoder_encode(JNIEnv *env, jobject thiz,  jbyteArray bytes,jstring file_path) {
    avifBool encodeYUVDirectly = AVIF_FALSE;

    const char * outputFilename = (*env)->GetStringUTFChars(env,file_path,JNI_FALSE);

    int returnCode = 1;
    avifEncoder * encoder = NULL;
    avifRWData avifOutput = AVIF_DATA_EMPTY;
    avifRGBImage rgb;
    memset(&rgb, 0, sizeof(rgb));

    avifImage * image = avifImageCreate(128, 128, 8, AVIF_PIXEL_FORMAT_YUV444); // these values dictate what goes into the final AVIF
    // Configure image here: (see avif/avif.h)
    // * colorPrimaries
    // * transferCharacteristics
    // * matrixCoefficients
    // * avifImageSetProfileICC()
    // * avifImageSetMetadataExif()
    // * avifImageSetMetadataXMP()
    // * yuvRange
    // * alphaRange
    // * alphaPremultiplied
    // * transforms (transformFlags, pasp, clap, irot, imir)
    //avifImageSetMetadataExif(image,)

    if (encodeYUVDirectly) {
        // If you have YUV(A) data you want to encode, use this path
        LOG_I("Encoding raw YUVA data\n");

        avifImageAllocatePlanes(image, AVIF_PLANES_YUV | AVIF_PLANES_A);

        // Fill your YUV(A) data here
        memset(image->yuvPlanes[AVIF_CHAN_Y], 255, image->yuvRowBytes[AVIF_CHAN_Y] * image->height);
        memset(image->yuvPlanes[AVIF_CHAN_U], 128, image->yuvRowBytes[AVIF_CHAN_U] * image->height);
        memset(image->yuvPlanes[AVIF_CHAN_V], 128, image->yuvRowBytes[AVIF_CHAN_V] * image->height);
        memset(image->alphaPlane, 255, image->alphaRowBytes * image->height);
    } else {
        // If you have RGB(A) data you want to encode, use this path
        LOG_I("Encoding from converted RGBA\n");

        avifRGBImageSetDefaults(&rgb, image);
        // Override RGB(A)->YUV(A) defaults here: depth, format, chromaUpsampling, ignoreAlpha, alphaPremultiplied, libYUVUsage, etc

        // Alternative: set rgb.pixels and rgb.rowBytes yourself, which should match your chosen rgb.format
        // Be sure to use uint16_t* instead of uint8_t* for rgb.pixels/rgb.rowBytes if (rgb.depth > 8)
        avifRGBImageAllocatePixels(&rgb);

        // Fill your RGB(A) data here
        memset(rgb.pixels, 255, rgb.rowBytes * image->height);

        avifResult convertResult = avifImageRGBToYUV(image, &rgb);
        if (convertResult != AVIF_RESULT_OK) {
            LOG_I( "Failed to convert to YUV(A): %s\n", avifResultToString(convertResult));
            goto cleanup;
        }
    }

    encoder = avifEncoderCreate();
    // Configure your encoder here (see avif/avif.h):
    // * maxThreads
    // * minQuantizer
    // * maxQuantizer
    // * minQuantizerAlpha
    // * maxQuantizerAlpha
    // * tileRowsLog2
    // * tileColsLog2
    // * speed
    // * keyframeInterval
    // * timescale

    // Call avifEncoderAddImage() for each image in your sequence
    // Only set AVIF_ADD_IMAGE_FLAG_SINGLE if you're not encoding a sequence
    // Use avifEncoderAddImageGrid() instead with an array of avifImage* to make a grid image
    avifResult addImageResult = avifEncoderAddImage(encoder, image, 1, AVIF_ADD_IMAGE_FLAG_SINGLE);
    if (addImageResult != AVIF_RESULT_OK) {
        LOG_I( "Failed to add image to encoder: %s\n", avifResultToString(addImageResult));
        goto cleanup;
    }

    avifResult finishResult = avifEncoderFinish(encoder, &avifOutput);
    if (finishResult != AVIF_RESULT_OK) {
        LOG_I( "Failed to finish encode: %s\n", avifResultToString(finishResult));
        goto cleanup;
    }

    LOG_I("Encode success: %zu total bytes\n", avifOutput.size);

    FILE * f = fopen(outputFilename, "wb");
    size_t bytesWritten = fwrite(avifOutput.data, 1, avifOutput.size, f);
    fclose(f);
    if (bytesWritten != avifOutput.size) {
        LOG_I( "Failed to write %zu bytes\n", avifOutput.size);
        goto cleanup;
    }
    LOG_I("Wrote: %s\n", outputFilename);

    returnCode = 0;
    cleanup:
    if (image) {
        avifImageDestroy(image);
    }
    if (encoder) {
        avifEncoderDestroy(encoder);
    }
    avifRWDataFree(&avifOutput);
    avifRGBImageFreePixels(&rgb); // Only use in conjunction with avifRGBImageAllocatePixels()
    return returnCode;

}

JNIEXPORT jint JNICALL
Java_libavif_AvifEncoder_encode2(JNIEnv *env, jclass clazz, jbyteArray bitmap_bytes_rgba,
                                 jint width, jint height, jlong row_bytes, jstring out_path) {
    avifBool encodeYUVDirectly = AVIF_FALSE;

    const char * outputFilename = (*env)->GetStringUTFChars(env,out_path,JNI_FALSE);

    int returnCode = 1;
    avifEncoder * encoder = NULL;
    avifRWData avifOutput = AVIF_DATA_EMPTY;
    avifRGBImage rgb ;
    memset(&rgb, 0, sizeof(rgb));

    avifImage * image = avifImageCreate(width, height, 8, AVIF_PIXEL_FORMAT_YUV444); // these values dictate what goes into the final AVIF
    // Configure image here: (see avif/avif.h)
    // * colorPrimaries
    // * transferCharacteristics
    // * matrixCoefficients
    // * avifImageSetProfileICC()
    // * avifImageSetMetadataExif()
    // * avifImageSetMetadataXMP()
    // * yuvRange
    // * alphaRange
    // * alphaPremultiplied
    // * transforms (transformFlags, pasp, clap, irot, imir)
    //avifImageSetMetadataExif(image,)

    if (encodeYUVDirectly) {
        // If you have YUV(A) data you want to encode, use this path
        LOG_I("Encoding raw YUVA data\n");

        avifImageAllocatePlanes(image, AVIF_PLANES_YUV | AVIF_PLANES_A);

        // Fill your YUV(A) data here
        memset(image->yuvPlanes[AVIF_CHAN_Y], 255, image->yuvRowBytes[AVIF_CHAN_Y] * image->height);
        memset(image->yuvPlanes[AVIF_CHAN_U], 128, image->yuvRowBytes[AVIF_CHAN_U] * image->height);
        memset(image->yuvPlanes[AVIF_CHAN_V], 128, image->yuvRowBytes[AVIF_CHAN_V] * image->height);
        memset(image->alphaPlane, 255, image->alphaRowBytes * image->height);
    } else {
        // If you have RGB(A) data you want to encode, use this path
        LOG_I("Encoding from converted RGBA\n");

        avifRGBImageSetDefaults(&rgb, image);
        // Override RGB(A)->YUV(A) defaults here: depth, format, chromaUpsampling, ignoreAlpha, alphaPremultiplied, libYUVUsage, etc

        // Alternative: set rgb.pixels and rgb.rowBytes yourself, which should match your chosen rgb.format
        // Be sure to use uint16_t* instead of uint8_t* for rgb.pixels/rgb.rowBytes if (rgb.depth > 8)
        avifRGBImageAllocatePixels(&rgb);

        // Fill your RGB(A) data here
        rgb.pixels = (*env)->GetByteArrayElements(env,bitmap_bytes_rgba, NULL);;
        rgb.rowBytes = row_bytes;
        rgb.width = width;
        rgb.height = height;

        //memset(rgb.pixels, 255, rgb.rowBytes * image->height);

        avifResult convertResult = avifImageRGBToYUV(image, &rgb);
        if (convertResult != AVIF_RESULT_OK) {
            LOG_I( "Failed to convert to YUV(A): %s\n", avifResultToString(convertResult));
            goto cleanup;
        }
    }

    encoder = avifEncoderCreate();
    // Configure your encoder here (see avif/avif.h):
    // * maxThreads
    // * minQuantizer
    // * maxQuantizer
    // * minQuantizerAlpha
    // * maxQuantizerAlpha
    // * tileRowsLog2
    // * tileColsLog2
    // * speed
    // * keyframeInterval
    // * timescale

    // Call avifEncoderAddImage() for each image in your sequence
    // Only set AVIF_ADD_IMAGE_FLAG_SINGLE if you're not encoding a sequence
    // Use avifEncoderAddImageGrid() instead with an array of avifImage* to make a grid image
    avifResult addImageResult = avifEncoderAddImage(encoder, image, 1, AVIF_ADD_IMAGE_FLAG_SINGLE);
    if (addImageResult != AVIF_RESULT_OK) {
        LOG_I( "Failed to add image to encoder: %s\n", avifResultToString(addImageResult));
        goto cleanup;
    }

    avifResult finishResult = avifEncoderFinish(encoder, &avifOutput);
    if (finishResult != AVIF_RESULT_OK) {
        LOG_I( "Failed to finish encode: %s\n", avifResultToString(finishResult));
        goto cleanup;
    }

    LOG_I("Encode success: %zu total bytes\n", avifOutput.size);

    FILE * f = fopen(outputFilename, "wb");
    size_t bytesWritten = fwrite(avifOutput.data, 1, avifOutput.size, f);
    fclose(f);
    if (bytesWritten != avifOutput.size) {
        LOG_I( "Failed to write %zu bytes\n", avifOutput.size);
        goto cleanup;
    }
    LOG_I("Wrote: %s\n", outputFilename);

    returnCode = 0;
    cleanup:
    if (image) {
        avifImageDestroy(image);
    }
    if (encoder) {
        avifEncoderDestroy(encoder);
    }
    avifRWDataFree(&avifOutput);
    avifRGBImageFreePixels(&rgb); // Only use in conjunction with avifRGBImageAllocatePixels()
    return returnCode;
}