#ifndef __MFCCKERNEL_H__
#define __MFCCKERNEL_H__

#include "AutoTilerLibTypes.h"
#include "MfccBasicKernels.h"
#include "CmplxFunctions.h"
#include "PreProcessing.h"
// #include "DSP_Lib.h"
#define _L1_Memory_SIZE 25300
#define _L2_Memory_SIZE 0
extern char *L1_Memory; /* Size given for generation: 50784 bytes, used: 25300 bytes */
extern char *L2_Memory; /* Size used for generation: 0 bytes */
extern void Tensorflow_MFCC(
		short int * __restrict__ In,
		short int * __restrict__ Out,
		short int * __restrict__ Twiddles_fft_int,
		short int * __restrict__ Twiddles_rfft,
		short int * SwapTable_fft,
		short int * __restrict__ WinTable,
		fbank_type_t * Mel_FilterBank,
		short int * __restrict__ Mel_Coeffs,
		int Norm,
		short int * __restrict__ DCT_Coeff);
#endif
