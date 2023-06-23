#ifndef _WAVUTIL_H_
#define _WAVUTIL_H_

#include "Gap.h"

#define WAV_HEADER_SIZE 44 // bytes


static PI_L2 uint8_t header_buffer[WAV_HEADER_SIZE];
static struct pi_device fs_wav;
static void *wavfile;


void dump_wav_open(char *filename, int width, int sampling_rate, int nb_channels, int size);

void dump_wav_write(void *data, int size);

void dump_wav_close();


#endif /* WAVUTIL_H */
