//
// Created by zhougang on 2019/12/2.
//

#include <jni.h>
#include <stdbool.h>
#include <jerror.h>
#include "glzencode.h"
#include "zlib.h"
#include "android/log.h"
#include "android/bitmap.h"
#include "jpeglib.h"

#define MJPEG_INITIAL_BUFFER_SIZE (32 * 1024)
static char* Path = NULL;
typedef struct VideoBuffer VideoBuffer;
struct VideoBuffer {
    uint8_t *data;
    uint32_t size;
    void (*free)(VideoBuffer *buffer);
};
typedef struct MJpegVideoBuffer {
    VideoBuffer base;
    size_t maxsize;
} MJpegVideoBuffer;

typedef struct {
    struct jpeg_destination_mgr pub; /* public fields */

    unsigned char ** outbuffer;      /* target buffer */
    size_t * outsize;
    uint8_t * buffer;                /* start of buffer */
    size_t bufsize;
} mem_destination_mgr;

static void mjpeg_video_buffer_free(VideoBuffer *video_buffer)
{
    MJpegVideoBuffer *buffer = (MJpegVideoBuffer*)video_buffer;
    free(buffer->base.data);
    free(buffer);
}

static void init_mem_destination(j_compress_ptr cinfo)
{
}

static boolean empty_mem_output_buffer(j_compress_ptr cinfo)
{
    size_t nextsize;
    uint8_t * nextbuffer;
    mem_destination_mgr *dest = (mem_destination_mgr *) cinfo->dest;

    /* Try to allocate new buffer with double size */
    nextsize = dest->bufsize * 2;
    nextbuffer = realloc(dest->buffer, nextsize);

    if (nextbuffer == NULL)
        ERREXIT1(cinfo, JERR_OUT_OF_MEMORY, 10);

    dest->pub.next_output_byte = nextbuffer + dest->bufsize;
    dest->pub.free_in_buffer = dest->bufsize;

    dest->buffer = nextbuffer;
    dest->bufsize = nextsize;

    return TRUE;
}

static void Dump_Jpg(char* buffer, int size){
    static FILE *g_fp_264 = NULL;
    if(g_fp_264==NULL) {
        g_fp_264 = fopen(Path, "wb");

    }
    if(g_fp_264!=NULL){
        fwrite(buffer,size,1,g_fp_264);
    } else {
        __android_log_write(6, "zhou", "open file failed!");
    }

}

static void term_mem_destination(j_compress_ptr cinfo)
{
    mem_destination_mgr *dest = (mem_destination_mgr *) cinfo->dest;

    *dest->outbuffer = dest->buffer;
    *dest->outsize = dest->bufsize;
    Dump_Jpg(dest->buffer, dest->bufsize);
    __android_log_print(6, "zhou","out size %d !", dest->bufsize);
}

static void zg_jpg_mem_dest(j_compress_ptr cinfo,
                            unsigned char ** outbuffer, size_t * outsize)
{
        mem_destination_mgr *dest;

        if (outbuffer == NULL || *outbuffer == NULL ||
            outsize == NULL || *outsize == 0) { /* sanity check */
            ERREXIT(cinfo, JERR_BUFFER_SIZE);
        }

        if (cinfo->dest == NULL) { /* first time for this JPEG object? */
            cinfo->dest = malloc(sizeof(mem_destination_mgr));
        }

        dest = (mem_destination_mgr *) cinfo->dest;
        dest->pub.init_destination = init_mem_destination;
        dest->pub.empty_output_buffer = empty_mem_output_buffer;
        dest->pub.term_destination = term_mem_destination;
        dest->outbuffer = outbuffer;
        dest->outsize = outsize;

        dest->pub.next_output_byte = dest->buffer = *outbuffer;
        dest->pub.free_in_buffer = dest->bufsize = *outsize;
}
static MJpegVideoBuffer* create_mjpeg_video_buffer(void)
{
    MJpegVideoBuffer *buffer = malloc(sizeof(MJpegVideoBuffer));
    buffer->base.free = mjpeg_video_buffer_free;
    buffer->maxsize = MJPEG_INITIAL_BUFFER_SIZE;
    buffer->base.data = malloc(buffer->maxsize);
    if (!buffer->base.data) {
        free(buffer);
        buffer = NULL;
    }
    return buffer;
}

static void jpeg_encode(char* pixels1, char* pixels2 , int width, int height, char* path) {

    int src_len = width * height * 4;
//    for (int i=0; i< src_len; i++){
//        pixels2[i] = pixels1[i]^pixels2[i];
//    }
    MJpegVideoBuffer *videoBuffer = create_mjpeg_video_buffer();
    struct jpeg_compress_struct jpeg_compress;
    struct jpeg_error_mgr jerr;
    jpeg_compress.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&jpeg_compress);
    jpeg_compress.image_width = width;
    jpeg_compress.image_height = height;
    jpeg_compress.dct_method = JDCT_FLOAT;
    //jpeg_compress.dc_huff_tbl_ptrs
    jpeg_compress.input_components = 4;
    jpeg_compress.in_color_space = JCS_EXT_RGBA;
    zg_jpg_mem_dest(&jpeg_compress, &videoBuffer->base.data, &videoBuffer->maxsize);

    jpeg_set_defaults(&jpeg_compress);
    jpeg_set_quality(&jpeg_compress, 40, true);
    jpeg_start_compress(&jpeg_compress, TRUE);
    JSAMPROW row_pointer[1];
    int row_stride;
    row_stride = jpeg_compress.image_width * 4;
    while (jpeg_compress.next_scanline < jpeg_compress.image_height){
        row_pointer[0] = &pixels2[jpeg_compress.next_scanline * row_stride];
        jpeg_write_scanlines(&jpeg_compress, row_pointer, 1);
    }

    jpeg_finish_compress(&jpeg_compress);

}
JNIEXPORT void JNICALL
Java_com_example_zlib_1test_MainActivity_EncodeImage(JNIEnv* env,
                                                     jobject obj,
                                                     jobject bitmap1,
                                                     jobject bitmap2,
                                                     jint width,
                                                     jint height,
                                                     jstring path) {
    char* pixels1;
    char* pixels2;
    Path = (*env)->GetStringUTFChars(env, path, NULL);
    __android_log_write(6, "zhou","start encode image !");
    if (AndroidBitmap_lockPixels(env, bitmap1, (void**)&pixels1) < 0) {
        return;
    }
    if (AndroidBitmap_lockPixels(env, bitmap2, (void**)&pixels2) < 0) {
        return;
    }
    int length = width * height * 4;
    int src_len = length;
//    for (int i=0; i< src_len; i++){
//        pixels2[i] = pixels1[i]^pixels2[i];
//    }
    __android_log_print(6, "zhou","encode image before size %d !", src_len);
    char *dest = malloc(length); int dest_len = length;
    compress2(dest, &dest_len, pixels2, src_len, 9);
    __android_log_print(6, "zhou","encode image after size %d !", dest_len);
    char *dest2 = malloc(length); int dest2_len = length;
    uncompress2(dest2, &dest2_len, dest, &dest_len);
    __android_log_print(6, "zhou","encode image uncompress size %d !", dest2_len);
    jpeg_encode(pixels1, dest2, width, height, Path);

    AndroidBitmap_unlockPixels(env, bitmap1);
    AndroidBitmap_unlockPixels(env, bitmap2);
}
JNIEXPORT void JNICALL
Java_com_example_zlib_1test_MainActivity_JpegEncode(JNIEnv* env,
                                                    jobject obj,
                                                    jobject bitmap1,
                                                    jobject bitmap2,
                                                    jint width,
                                                    jint height,
                                                    jstring path) {
    char* pixels1;
    char* pixels2;
    Path = (*env)->GetStringUTFChars(env, path, NULL);
    __android_log_print(6, "zhou","start jpeg encode image %s!", Path);
    if (AndroidBitmap_lockPixels(env, bitmap1, (void**)&pixels1) < 0) {
        return;
    }
    if (AndroidBitmap_lockPixels(env, bitmap2, (void**)&pixels2) < 0) {
        return;
    }
    int src_len = width * height * 4;
    char *pixels3 = malloc(width * 3 * height);
    int stride = width * 3;
//    for (int i=0; i< src_len; i++){
//        pixels2[i] = pixels1[i]^pixels2[i];
//    }
    for (int i = 0; i < height; i ++ ){
        for (int j = 0; j < width; j ++ ) {
            pixels3[i * stride + j * 3] = pixels2[i * width * 4 + j * 4];
            pixels3[i * stride + j * 3 + 1] = pixels2[i * width * 4 + j * 4 + 1];
            pixels3[i * stride + j * 3 + 2] = pixels2[i * width * 4 + j * 4 + 2];
        }
    }

    MJpegVideoBuffer *videoBuffer = create_mjpeg_video_buffer();
    struct jpeg_compress_struct jpeg_compress;
    struct jpeg_error_mgr jerr;
    jpeg_compress.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&jpeg_compress);
    jpeg_compress.image_width = width;
    jpeg_compress.image_height = height;
    jpeg_compress.input_components = 3;
    jpeg_compress.in_color_space = JCS_EXT_RGB;
    jpeg_compress.dct_method = JDCT_FLOAT;
    zg_jpg_mem_dest(&jpeg_compress, &videoBuffer->base.data, &videoBuffer->maxsize);
    jpeg_set_defaults(&jpeg_compress);
    jpeg_set_quality(&jpeg_compress, 40, true);
    jpeg_start_compress(&jpeg_compress, TRUE);
    JSAMPROW row_pointer[1];
    int row_stride;
    row_stride = jpeg_compress.image_width * 3;
    while (jpeg_compress.next_scanline < jpeg_compress.image_height){
        row_pointer[0] = &pixels3[jpeg_compress.next_scanline * row_stride];
        jpeg_write_scanlines(&jpeg_compress, row_pointer, 1);

    }

    jpeg_finish_compress(&jpeg_compress);
    AndroidBitmap_unlockPixels(env, bitmap1);
    AndroidBitmap_unlockPixels(env, bitmap2);
}
