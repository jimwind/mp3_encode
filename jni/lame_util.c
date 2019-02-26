#include "lame_3.99.5_libmp3lame/lame.h"
#include "me_mi_mp3_util_LameUtil.h"
#include <stdio.h>
#include <jni.h>
#include <stdio.h>
#include <stdlib.h>
#include <jni.h>
#define LOG_TAG "LAME ENCODER"
#define LOGD(format, args...)  __android_log_print(ANDROID_LOG_DEBUG, LOG_TAG, format, ##args);
#define BUFFER_SIZE 8192
#define be_short(s) ((short) ((unsigned short) (s) << 8) | ((unsigned short) (s) >> 8))

#define INBUFSIZE 4096  
#define MP3BUFSIZE (int) (1.25 * INBUFSIZE) + 7200  

static lame_global_flags *lame = NULL;

JNIEXPORT void JNICALL Java_me_mi_mp3_util_LameUtil_init(
		JNIEnv *env, jclass cls, jint inSamplerate, jint inChannel, jint outSamplerate, jint outBitrate, jint quality) {
	if (lame != NULL) {
		lame_close(lame);
		lame = NULL;
	}
	lame = lame_init();
	lame_set_in_samplerate(lame, inSamplerate);
	lame_set_num_channels(lame, inChannel);//输入流的声道
	lame_set_out_samplerate(lame, outSamplerate);
	lame_set_brate(lame, outBitrate);
	lame_set_quality(lame, quality);
	lame_init_params(lame);
}

JNIEXPORT jint JNICALL Java_me_mi_mp3_util_LameUtil_encode(
		JNIEnv *env, jclass cls, jshortArray buffer_l, jshortArray buffer_r,
		jint samples, jbyteArray mp3buf) {
	jshort* j_buffer_l = (*env)->GetShortArrayElements(env, buffer_l, NULL);

	jshort* j_buffer_r = (*env)->GetShortArrayElements(env, buffer_r, NULL);

	const jsize mp3buf_size = (*env)->GetArrayLength(env, mp3buf);
	jbyte* j_mp3buf = (*env)->GetByteArrayElements(env, mp3buf, NULL);

	int result = lame_encode_buffer(lame, j_buffer_l, j_buffer_r,
			samples, j_mp3buf, mp3buf_size);

	(*env)->ReleaseShortArrayElements(env, buffer_l, j_buffer_l, 0);
	(*env)->ReleaseShortArrayElements(env, buffer_r, j_buffer_r, 0);
	(*env)->ReleaseByteArrayElements(env, mp3buf, j_mp3buf, 0);

	return result;
}

JNIEXPORT jint JNICALL Java_me_mi_mp3_util_LameUtil_flush(
		JNIEnv *env, jclass cls, jbyteArray mp3buf) {
	const jsize mp3buf_size = (*env)->GetArrayLength(env, mp3buf);
	jbyte* j_mp3buf = (*env)->GetByteArrayElements(env, mp3buf, NULL);

	int result = lame_encode_flush(lame, j_mp3buf, mp3buf_size);

	(*env)->ReleaseByteArrayElements(env, mp3buf, j_mp3buf, 0);

	return result;
}

JNIEXPORT void JNICALL Java_me_mi_mp3_util_LameUtil_close
(JNIEnv *env, jclass cls) {
	lame_close(lame);
	lame = NULL;
}

 
int read_samples(FILE *input_file, short *input) {
	int nb_read;
	nb_read = fread(input, 1, sizeof(short), input_file) / sizeof(short);
 
	int i = 0;
	while (i < nb_read) {
		input[i] = be_short(input[i]);
		i++;
	}
 
	return nb_read;
}
 
JNIEXPORT jint Java_me_mi_mp3_util_LameUtil_encodeFile(JNIEnv *env,
		jobject jobj, jstring in_source_path, jstring in_target_path) {
	int status = 0;  
	
	short* input_buffer;
	int input_samples;
	unsigned char* mp3_buffer;
	int mp3_bytes;
	
	const char *source_path, *target_path;
	source_path = (*env)->GetStringUTFChars(env, in_source_path, NULL);
	target_path = (*env)->GetStringUTFChars(env, in_target_path, NULL);
 
	FILE *infp, *outfp;
	infp = fopen(source_path, "rb");
	outfp = fopen(target_path, "wb");
	
	mp3_buffer = (char *)malloc((size_t)MP3BUFSIZE);
	input_buffer = (short *)malloc((size_t)INBUFSIZE * 2);
	
	
	do{
		input_samples = fread(input_buffer, sizeof(short int) * 2, (size_t)INBUFSIZE, infp);
		printf("input_samples is %d./n",input_samples);  
		mp3_bytes = lame_encode_buffer_interleaved(lame, input_buffer, input_samples, mp3_buffer, MP3BUFSIZE);
		
		if(mp3_bytes < 0){
			printf("lame_encode_buffer_interleaved returned %d/n", mp3_bytes);  
			status = -1;
			goto free_buffers;
		} else if(mp3_bytes > 0){
			fwrite(mp3_buffer, 1, mp3_bytes, outfp);
		}
	} while(input_samples == INBUFSIZE);
	
	mp3_bytes = lame_encode_flush(lame, mp3_buffer, sizeof(mp3_buffer));
	if(mp3_bytes > 0){
		printf("writing %d mp3 bytes/n", mp3_bytes);  
		fwrite(mp3_buffer, 1, mp3_bytes, outfp);
	}
	
	free_buffers:
		free(mp3_buffer);
		free(input_buffer);
		fclose(outfp);
		fclose(infp);
	close_lame:
		lame_close(lame);
		
	exit:
		return status;
}