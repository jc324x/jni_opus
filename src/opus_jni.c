#include <jni.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "opus_jni.h"
#include "opus.h"

static const char *JNIT_CLASS = "interop/PrintLine";

char logMsg[255];
OpusDecoder *dec;

opus_int32 SAMPLING_RATE;
int CHANNELS;
int FRAME_SIZE;

static jboolean opus_init_decoder (JNIEnv *env, jobject obj, jint samplingRate, jint numberOfChannels, jint frameSize){
    FRAME_SIZE = frameSize;
    SAMPLING_RATE = samplingRate;
    CHANNELS = numberOfChannels;

    int size;
    int error;

    size = opus_decoder_get_size(CHANNELS);
    dec = malloc(size);
    error = opus_decoder_init(dec, SAMPLING_RATE, CHANNELS);

    return error;
}

static jint opus_decode_bytes (JNIEnv *env, jobject obj, jbyteArray in, jshortArray out){
    jint inputArraySize = (*env)->GetArrayLength(env, in);
    jint outputArraySize = (*env)->GetArrayLength(env, out);

    jbyte* encodedData = (*env)->GetByteArrayElements(env, in, 0);
    opus_int16 *data = (opus_int16*)calloc(outputArraySize,sizeof(opus_int16));

    int decodedDataArraySize = opus_decode(dec, encodedData, inputArraySize, data, FRAME_SIZE, 0);

    if (decodedDataArraySize >=0)
    {
        if (decodedDataArraySize <= outputArraySize)
        {
            (*env)->SetShortArrayRegion(env,out,0,decodedDataArraySize,data);
        }
        else
        {
            return -1;
        }
    }

    (*env)->ReleaseByteArrayElements(env,in,encodedData,JNI_ABORT);

    return decodedDataArraySize;
}

static jboolean opus_release_decoder (JNIEnv *env, jobject obj){
    free(dec);
    return 1;
}


static JNINativeMethod funcs[] = {
        { "opus_init_decoder", "(III)Z", (void *)&opus_init_decoder },
        { "opus_decode_bytes", "([B[S)I", (void *)&opus_decode_bytes },
        { "opus_release_decoder", "()Z", (void *)&opus_release_decoder }
};

#define CURRENT_JNI JNI_VERSION_1_6

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved)
{
    JNIEnv *env;
    jclass  cls;
    jint    res;

    (void)reserved;

    if ((*vm)->GetEnv(vm, (void **)&env, CURRENT_JNI) != JNI_OK)
        return -1;

    cls = (*env)->FindClass(env, JNIT_CLASS);
    if (cls == NULL)
        return -1;

    res = (*env)->RegisterNatives(env, cls, funcs, sizeof(funcs)/sizeof(*funcs));
    if (res != 0)
        return -1;

    return CURRENT_JNI;
}

JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *vm, void *reserved)
{
JNIEnv *env;
jclass  cls;

(void)reserved;

if ((*vm)->GetEnv(vm, (void **)&env, CURRENT_JNI) != JNI_OK)
return;

cls = (*env)->FindClass(env, JNIT_CLASS);
if (cls == NULL)
return;

(*env)->UnregisterNatives(env, cls);
}
