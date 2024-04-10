#ifndef _LOCALUTIL_H_
#define _LOCALUTIL_H_

#include "Gap.h"
#include "application.h"

#define WAV_HEADER_SIZE 44 // bytes


static PI_L2 uint8_t header_buffer[WAV_HEADER_SIZE];
static struct pi_device fs_wav;
static void *wavfile;


int predict_float (void * array, int n_classes);

int predict (void * l2_buffer, int n_classes);

void dump_wav_open(char *filename, int width, int sampling_rate, int nb_channels, int size);

void dump_wav_write(void *data, int size);

void dump_wav_close();

// void dump_data_write(char *filename, void *data, int size);


#endif /* LOCALUTIL_H */
