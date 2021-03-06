#include <jni.h>
#include <string>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing/render_face_detections.h>
#include <dlib/image_processing.h>
#include <dlib/image_transforms.h>
#include <dlib/image_io.h>
#include <dlib/opencv.h>
#include <iostream>
#include <android/log.h>
//#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc_c.h>


using namespace dlib;
using namespace cv;

// This value is 2 ^ 18 - 1, and is used to clamp the RGB values before their
// ranges
// are normalized to eight bits.
static const int kMaxChannelValue = 262143;

/*
 *  카메라 프리뷰에서 캡처한 YUV 420 형식의 데이터를
 *  OpenCV 이미지 처리가 가능한 RGB 형식의 데이터로 바꾸는 메서드
 * */
void ConvertYUV420ToARGB8888(uint8 *, uint8 *string1, uint8 *string2, uint32 *pInt, jint width,
                             jint height, jint stride, jint row_stride, jint pixel_stride);

uint32 YUV2RGB(int nY, int nU, int nV) {
    nY -= 16;
    nU -= 128;
    nV -= 128;
    if (nY < 0)
        nY = 0;

    // This is the floating point equivalent. We do the conversion in integer
    // because some Android devices do not have floating point in hardware.
    // nR = (int)(1.164 * nY + 2.018 * nU);
    // nG = (int)(1.164 * nY - 0.813 * nV - 0.391 * nU);
    // nB = (int)(1.164 * nY + 1.596 * nV);

    int nR = (int)(1192 * nY + 1634 * nV);
    int nG = (int)(1192 * nY - 833 * nV - 400 * nU);
    int nB = (int)(1192 * nY + 2066 * nU);

    nR = MIN(kMaxChannelValue, MAX(0, nR));
    nG = MIN(kMaxChannelValue, MAX(0, nG));
    nB = MIN(kMaxChannelValue, MAX(0, nB));

    nR = (nR >> 10) & 0xff;
    nG = (nG >> 10) & 0xff;
    nB = (nB >> 10) & 0xff;

    return 0xff000000 | (nR << 16) | (nG << 8) | nB;
}


void ConvertYUV420ToARGB(uint8 * const yData, uint8* const uData,
                             uint8* const vData, uint32* const output,
                             const int width,
                             const int height, const int y_row_stride, const int uv_row_stride, const int uv_pixel_stride) {

    uint32 *out = output;

    for (int y = 0; y < height; y++) {
        const uint8 *pY = yData + y_row_stride * y;

        const int uv_row_start = uv_row_stride * (y >> 1);
        const uint8 *pU = uData + uv_row_start;
        const uint8 *pV = vData + uv_row_start;

        for (int x = 0; x < width; x++) {
            const int uv_offset = (x >> 1) * uv_pixel_stride;
            *out++ = YUV2RGB(pY[x], pU[uv_offset], pV[uv_offset]);
        }

    }
}

    extern "C"
    JNIEXPORT void JNICALL
    Java_com_example_jwh4807_newtask_ImageUtils_convertYUV420ToARGB8888(JNIEnv *env, jclass clazz,
                                                                        jbyteArray y, jbyteArray u,
                                                                        jbyteArray v,
                                                                        jintArray output,
                                                                        jint width,
                                                                        jint height,
                                                                        jint y_row_stride,
                                                                        jint uv_row_stride,
                                                                        jint uv_pixel_stride,
                                                                        jboolean halfSize) {
        jboolean inputCopy = JNI_FALSE;
        jbyte *const y_buff = env->GetByteArrayElements(y, &inputCopy);
        jboolean outputCopy = JNI_FALSE;
        jint *const o = env->GetIntArrayElements(output, &outputCopy);

        if (halfSize) {
//        ConvertYUV420SPToARGB8888HalfSize(reinterpret_cast<uint8*>(y_buff),
//                                          reinterpret_cast<uint32*>(o), width,
//                                          height);
        } else {
            jbyte *const u_buff = env->GetByteArrayElements(u, &inputCopy);
            jbyte *const v_buff = env->GetByteArrayElements(v, &inputCopy);

            ConvertYUV420ToARGB(
                    reinterpret_cast<uint8 *>(y_buff), reinterpret_cast<uint8 *>(u_buff),
                    reinterpret_cast<uint8 *>(v_buff), reinterpret_cast<uint32 *>(o), width,
                    height, y_row_stride, uv_row_stride, uv_pixel_stride);

            env->ReleaseByteArrayElements(u, u_buff, JNI_ABORT);
            env->ReleaseByteArrayElements(v, v_buff, JNI_ABORT);
        }

        env->ReleaseByteArrayElements(y, y_buff, JNI_ABORT);
        env->ReleaseIntArrayElements(output, o, 0);
    }