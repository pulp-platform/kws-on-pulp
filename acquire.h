#ifndef __ACQUIRE_H__
#define __ACQUIRE_H__

#include "globals.h"

// Configure PDM RX interface
int configure_pdm();

// Configure PMSIS SFU transfer
void handle_out_transfer_end(void *arg);

// Set up microphone for recording
void microphone_setup();

void wav_to_array(char* wavfile, MFCC_IN_TYPE* buffer, int noise, int save);

#endif /* ACQUIRE_H */