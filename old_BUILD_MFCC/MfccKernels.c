#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#include "MfccKernels.h"
L1_CL_MEM AT_L1_POINTER L1_Memory;
L2_MEM AT_L2_POINTER (null);
static AT_HYPERFLASH_FS_T HyperFlash;
void Tensorflow_MFCC(
		F16_DSP * __restrict__ In,
		F16_DSP * __restrict__ Out,
		F16_DSP * __restrict__ Twiddles_fft_int,
		F16_DSP * __restrict__ Twiddles_rfft,
		short int * SwapTable_fft,
		F16_DSP * __restrict__ WinTable,
		short int * Mel_FilterBank,
		F16_DSP * __restrict__ Mel_Coeffs,
		F16_DSP * __restrict__ DCT_Coeff)

{
	/* Shared L1: 54696 bytes, L2 buffer: 0 bytes */
	/* Local variables used by this kernel */
	AT_L2_EVENT _DmaR_Evt1, *DmaR_Evt1 = &_DmaR_Evt1;
	AT_L2_EVENT _DmaW_Evt1, *DmaW_Evt1 = &_DmaW_Evt1;
	Windowing_T S_KerArg0, *KerArg0 = &S_KerArg0;
	RFFT_Arg_T S_KerArg1, *KerArg1 = &S_KerArg1;
	CmplxMag_T S_KerArg2, *KerArg2 = &S_KerArg2;
	MelFilterBank_T S_KerArg3, *KerArg3 = &S_KerArg3;
	MFCC_LogF_T S_KerArg4, *KerArg4 = &S_KerArg4;
	DCT_II_Arg_T S_KerArg5, *KerArg5 = &S_KerArg5;

	/* Iteration space related variables */
	int D0Ind, D0Ind_Last;
	int T0Ind, T0Ind_Last;
	/* User kernel arguments related variables */
	/*============================= Ker Arg Iter Spaces =========================================
	User Kernel Iteration Space:
		[D0 Dim: 49][Tile0 Dim: 1]
	Ker Arg: In, Tiled Space: Buffer
		Min Pipe Depth: 0, Max Pipe Depth: 0
		KerArgItSpace: 49 logical tiles, 1 physical tiles
			@ 0 (Total Size: 32768 )[D0, [48 x 2048, 2048]]
		KerArgItSpace (User Kernel Iter Order):
			[D0, [48 x 2048, 2048]]
		Tile0: [0, 32768, 32768], Tile1: [0, 32768, 32768], Tile2; [0, 32768, 32768]
	Ker Arg: Out, Tiled Space: Buffer
		Min Pipe Depth: 0, Max Pipe Depth: 0
		KerArgItSpace: 49 logical tiles, 1 physical tiles
			@ 32768 (Total Size: 3920 )[D0, [48 x 80, 80]]
		KerArgItSpace (User Kernel Iter Order):
			[D0, [48 x 80, 80]]
		Tile0: [0, 3920, 3920], Tile1: [0, 3920, 3920], Tile2; [0, 3920, 3920]
	Ker Arg: In_rfft, Tiled Space: Buffer
		Min Pipe Depth: 0, Max Pipe Depth: 0
		KerArgItSpace: 1 logical tiles, 1 physical tiles
			@ 36688 (Total Size: 2048 )[Tile0, 1:[1x1024], 2]
		KerArgItSpace (User Kernel Iter Order):
			[Tile0, 1:[1x1024], 2]
		Tile0: [0, 2048, 2048], Tile1: [0, 2048, 2048], Tile2; [0, 2048, 2048]
	Ker Arg: FFT_Out, Tiled Space: Buffer
		Min Pipe Depth: 0, Max Pipe Depth: 0
		KerArgItSpace: 1 logical tiles, 1 physical tiles
			@ 38736 (Total Size: 2052 )[Tile0, 1:[1x1026], 2]
		KerArgItSpace (User Kernel Iter Order):
			[Tile0, 1:[1x1026], 2]
		Tile0: [0, 2052, 2052], Tile1: [0, 2052, 2052], Tile2; [0, 2052, 2052]
	Ker Arg: Buff2, Tiled Space: Buffer
		Min Pipe Depth: 0, Max Pipe Depth: 0
		KerArgItSpace: 1 logical tiles, 1 physical tiles
			@ 40788 (Total Size: 4104 )[Tile0, 1:[1x1026], 4]
		KerArgItSpace (User Kernel Iter Order):
			[Tile0, 1:[1x1026], 4]
		Tile0: [0, 4104, 4104], Tile1: [0, 4104, 4104], Tile2; [0, 4104, 4104]
	Ker Arg: WinTable, Tiled Space: Buffer
		Min Pipe Depth: 0, Max Pipe Depth: 0
		KerArgItSpace: 1 logical tiles, 1 physical tiles
			@ 44892 (Total Size: 1280 )[Tile0, 1:[1x640], 2]
		KerArgItSpace (User Kernel Iter Order):
			[Tile0, 1:[1x640], 2]
		Tile0: [0, 1280, 1280], Tile1: [0, 1280, 1280], Tile2; [0, 1280, 1280]
	Ker Arg: Twiddles_fft_int, Tiled Space: Buffer
		Min Pipe Depth: 0, Max Pipe Depth: 0
		KerArgItSpace: 1 logical tiles, 1 physical tiles
			@ 46172 (Total Size: 1024 )[Tile0, 1:[1x512], 2]
		KerArgItSpace (User Kernel Iter Order):
			[Tile0, 1:[1x512], 2]
		Tile0: [0, 1024, 1024], Tile1: [0, 1024, 1024], Tile2; [0, 1024, 1024]
	Ker Arg: SwapTable_fft, Tiled Space: Buffer
		Min Pipe Depth: 0, Max Pipe Depth: 0
		KerArgItSpace: 1 logical tiles, 1 physical tiles
			@ 47196 (Total Size: 1024 )[Tile0, 1:[1x512], 2]
		KerArgItSpace (User Kernel Iter Order):
			[Tile0, 1:[1x512], 2]
		Tile0: [0, 1024, 1024], Tile1: [0, 1024, 1024], Tile2; [0, 1024, 1024]
	Ker Arg: Twiddles_rfft, Tiled Space: Buffer
		Min Pipe Depth: 0, Max Pipe Depth: 0
		KerArgItSpace: 1 logical tiles, 1 physical tiles
			@ 48220 (Total Size: 2048 )[Tile0, 1:[1x1024], 2]
		KerArgItSpace (User Kernel Iter Order):
			[Tile0, 1:[1x1024], 2]
		Tile0: [0, 2048, 2048], Tile1: [0, 2048, 2048], Tile2; [0, 2048, 2048]
	Ker Arg: Mel_FilterBank, Tiled Space: Buffer
		Min Pipe Depth: 0, Max Pipe Depth: 0
		KerArgItSpace: 1 logical tiles, 1 physical tiles
			@ 50268 (Total Size: 240 )[Tile0, 1:[1x40], 6]
		KerArgItSpace (User Kernel Iter Order):
			[Tile0, 1:[1x40], 6]
		Tile0: [0, 240, 240], Tile1: [0, 240, 240], Tile2; [0, 240, 240]
	Ker Arg: Mel_Coeffs, Tiled Space: Buffer
		Min Pipe Depth: 0, Max Pipe Depth: 0
		KerArgItSpace: 1 logical tiles, 1 physical tiles
			@ 50508 (Total Size: 988 )[Tile0, 1:[1x494], 2]
		KerArgItSpace (User Kernel Iter Order):
			[Tile0, 1:[1x494], 2]
		Tile0: [0, 988, 988], Tile1: [0, 988, 988], Tile2; [0, 988, 988]
	Ker Arg: DCT_Coeff, Tiled Space: Buffer
		Min Pipe Depth: 0, Max Pipe Depth: 0
		KerArgItSpace: 1 logical tiles, 1 physical tiles
			@ 51496 (Total Size: 3200 )[Tile0, 1:[1x1600], 2]
		KerArgItSpace (User Kernel Iter Order):
			[Tile0, 1:[1x1600], 2]
		Tile0: [0, 3200, 3200], Tile1: [0, 3200, 3200], Tile2; [0, 3200, 3200]
	======================== End Ker Arg Iter Spaces =========================================*/
	/*=========================== Call Kernel, Invariant assignment =====================*/
	KerArg0->OutFrame = (void *__restrict__) (L1_Memory+36688);
	KerArg0->Window = (void *__restrict__) (L1_Memory+44892);
	KerArg0->FrameSize = (int) (640);
	KerArg0->FFT_Dim = (int) (1024);
	KerArg1->Data = (void * __restrict__) (L1_Memory+36688);
	KerArg1->RFFT_Out = (void * __restrict__) (L1_Memory+38736);
	KerArg1->Twiddles = (void * __restrict__) (L1_Memory+46172);
	KerArg1->RTwiddles = (void * __restrict__) (L1_Memory+48220);
	KerArg1->SwapTable = (void * __restrict__) (L1_Memory+47196);
	KerArg1->N_fft = (short int) (1024);
	KerArg1->Inverse = (unsigned char) (0);
	KerArg2->FrameIn = (void *__restrict__) (L1_Memory+38736);
	KerArg2->FrameOut = (void *__restrict__) (L1_Memory+40788);
	KerArg2->Nfft = (int) (1024);
	KerArg3->FramePower = (void *__restrict__) (L1_Memory+40788);
	KerArg3->MelSpectr = (void *__restrict__) (L1_Memory+36688);
	KerArg3->Mel_Coeffs = (void *__restrict__) (L1_Memory+50508);
	KerArg3->Mel_FilterBank = (short int *__restrict__) (L1_Memory+50268);
	KerArg3->Mel_NBanks = (short int) (40);
	KerArg3->Mel_Coeff_dyn = (short int) (15);
	KerArg3->IsMagSquared = (signed char) (1);
	KerArg4->FrameIn = (void *__restrict__) (L1_Memory+36688);
	KerArg4->FrameOut = (void *__restrict__) (L1_Memory+40788);
	KerArg4->FrameSize = (unsigned int) (40);
	KerArg4->LogOffset = (float) (0.000000);
	KerArg5->Data = (void *__restrict__) (L1_Memory+40788);
	KerArg5->DCTCoeff = (void *__restrict__) (L1_Memory+51496);
	KerArg5->n_input = (short int) (40);
	KerArg5->n_dct = (short int) (40);
	/*================================= Read Tiles Prolog ===============================*/
	AT_L2_COPY(0, ((AT_L2_EXT_ADDR_TYPE) In+0), ((AT_L2_INT_ADDR_TYPE) L1_Memory+0), 32768, 0, DmaR_Evt1);
	AT_L2_COPY_MERGED(0, ((AT_L2_EXT_ADDR_TYPE) DCT_Coeff+0), ((AT_L2_INT_ADDR_TYPE) L1_Memory+51496), 3200, 0, 0);
	AT_L2_COPY_MERGED(0, ((AT_L2_EXT_ADDR_TYPE) Twiddles_rfft+0), ((AT_L2_INT_ADDR_TYPE) L1_Memory+48220), 2048, 0, 0);
	AT_L2_COPY_MERGED(0, ((AT_L2_EXT_ADDR_TYPE) WinTable+0), ((AT_L2_INT_ADDR_TYPE) L1_Memory+44892), 1280, 0, 0);
	AT_L2_COPY_MERGED(0, ((AT_L2_EXT_ADDR_TYPE) Twiddles_fft_int+0), ((AT_L2_INT_ADDR_TYPE) L1_Memory+46172), 1024, 0, 0);
	AT_L2_COPY_MERGED(0, ((AT_L2_EXT_ADDR_TYPE) SwapTable_fft+0), ((AT_L2_INT_ADDR_TYPE) L1_Memory+47196), 1024, 0, 0);
	AT_L2_COPY_MERGED(0, ((AT_L2_EXT_ADDR_TYPE) Mel_Coeffs+0), ((AT_L2_INT_ADDR_TYPE) L1_Memory+50508), 988, 0, 0);
	AT_L2_COPY_MERGED(0, ((AT_L2_EXT_ADDR_TYPE) Mel_FilterBank+0), ((AT_L2_INT_ADDR_TYPE) L1_Memory+50268), 240, 0, 0);
	AT_L2_WAIT(0, DmaR_Evt1); /* Wait previous DMA read Mel_FilterBank */
	/*============================= End Read Tiles Prolog ===============================*/
	for (D0Ind=0; D0Ind<49; D0Ind++) { /* Iteration on D0 */
		int D0Ind_Last = (D0Ind==48);
		{ /* Single iteration on Tile0 */
			int T0Ind_Last = 1;
			/*====================== Call Kernel LOC_LOOP =========================*/
			KerArg0->Frame = (void *__restrict__) (L1_Memory+0+((D0Ind)*640));
			AT_FORK(gap_ncore(), (void *) WindowingReal2Real_PadCenter_f16, (void *) KerArg0);
			__CALL(WindowingReal2Real_PadCenter_f16, KerArg0);
			AT_FORK(gap_ncore(), (void *) RFFT_DIF_Par_f16, (void *) KerArg1);
			__CALL(RFFT_DIF_Par_f16, KerArg1);
			AT_FORK(gap_ncore(), (void *) CmplxMagSquared_f16, (void *) KerArg2);
			__CALL(CmplxMagSquared_f16, KerArg2);
			AT_FORK(gap_ncore(), (void *) MelFilterBank_f16, (void *) KerArg3);
			__CALL(MelFilterBank_f16, KerArg3);
			AT_FORK(gap_ncore(), (void *) MFCC_ComputeLog_f16, (void *) KerArg4);
			__CALL(MFCC_ComputeLog_f16, KerArg4);
			KerArg5->FeatList = (void *__restrict__) (L1_Memory+32768+((D0Ind)*80));
			AT_FORK(gap_ncore(), (void *) MFCC_ComputeDCT_II_f16, (void *) KerArg5);
			__CALL(MFCC_ComputeDCT_II_f16, KerArg5);
		} /* End iteration on Tile0 */
	} /* End iteration on D0 */
	/*================================ Write Tiles Epilog ===============================*/
	AT_L2_COPY(0, ((AT_L2_EXT_ADDR_TYPE) Out+0), ((AT_L2_INT_ADDR_TYPE) L1_Memory+32768), 3920, 1, DmaW_Evt1);
	AT_L2_WAIT(0, DmaW_Evt1); /* Wait DMA write Out */
	/*============================ End Write Tiles Epilog ===============================*/
}
#pragma GCC diagnostic pop
