#ifndef __MFCCKERNEL_H__
#define __MFCCKERNEL_H__

#include "AutoTilerLibTypes.h"
#include "DSP_Lib.h"
#define _L1_Memory_SIZE 54696
#define _(null)_SIZE 0
extern char *L1_Memory; /* Size given for generation: 104520 bytes, used: 54696 bytes */
extern char *(null); /* Size used for generation: 0 bytes */
extern void Tensorflow_MFCC(
		F16_DSP * __restrict__ In,
		F16_DSP * __restrict__ Out,
		F16_DSP * __restrict__ Twiddles_fft_int,
		F16_DSP * __restrict__ Twiddles_rfft,
		short int * SwapTable_fft,
		F16_DSP * __restrict__ WinTable,
		short int * Mel_FilterBank,
		F16_DSP * __restrict__ Mel_Coeffs,
		F16_DSP * __restrict__ DCT_Coeff);
#endif
