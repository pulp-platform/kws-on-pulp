#ifndef __PREPROCESS_H__
#define __PREPROCESS_H__

#include "globals.h"
#include "Gap.h"

// Initialize MFCC computation
void mfcc_kernel(void *args_mfcc);
// Set up MFCC computation
void mfcc_computation(MFCC_IN_TYPE * MfccInputSignal, OUT_TYPE * MfccOutputSignal);
void preprocess(MFCC_IN_TYPE * input_buffer, uint8_t * output_buffer, int input_src);

#endif /* PREPROCESS_H */