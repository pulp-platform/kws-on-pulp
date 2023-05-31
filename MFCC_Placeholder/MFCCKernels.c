#include "MFCCKernels.h"
L1_CL_MEM AT_L1_POINTER L1_Memory;
L2_MEM AT_L2_POINTER L2_Memory;
static AT_HYPERFLASH_FS_T HyperFlash;
void Tensorflow_MFCC(
		short int * __restrict__ In,
		short int * __restrict__ Out,
		short int * __restrict__ Twiddles_fft_int,
		short int * __restrict__ Twiddles_rfft,
		short int * SwapTable_fft,
		short int * __restrict__ WinTable,
		fbank_type_t * Mel_FilterBank,
		short int * __restrict__ Mel_Coeffs,
		int Norm,
		short int * __restrict__ DCT_Coeff)

{
	/* Shared L1: 25300 bytes, L2 buffer: 0 bytes */
	/* Local variables used by this kernel */
	AT_L2_EVENT DmaR_Evt1;
	AT_L2_EVENT DmaR_Evt2;
	AT_L2_EVENT DmaR_Evt3;
	AT_L2_EVENT DmaR_Evt4;
	AT_L2_EVENT DmaR_Evt5;
	AT_L2_EVENT DmaR_Evt6;
	AT_L2_EVENT DmaR_Evt7;
	AT_L2_EVENT DmaR_Evt8;
	AT_L2_EVENT DmaW_Evt1;
	PreEmphasis_T S_KerArg0, *KerArg0 = &S_KerArg0;
	Windowing_T S_KerArg1, *KerArg1 = &S_KerArg1;
	RFFT_Arg_T S_KerArg2, *KerArg2 = &S_KerArg2;
	CmplxMag_T S_KerArg3, *KerArg3 = &S_KerArg3;
	MelFilterBank_T S_KerArg4, *KerArg4 = &S_KerArg4;
	MFCC_Log_T S_KerArg5, *KerArg5 = &S_KerArg5;
	DCT_II_Arg_T S_KerArg6, *KerArg6 = &S_KerArg6;

	/* Iteration space related variables */
	int D0Ind, D0Ind_Total=0, D0Ind_Last, D0Ind_NextLast;
	int T0Ind, T0Ind_Last;
	/* User kernel arguments related variables */
	unsigned int _N_In;
	unsigned int _SN_In;
	/*============================= Ker Arg Iter Spaces =========================================
	User Kernel Iteration Space:
		[D0 Dim: 49][Tile0 Dim: 1]
	Ker Arg: In, Tiled Space: D0
		Min Pipe Depth: 0, Max Pipe Depth: 1
		KerArgItSpace: 49 logical tiles, 49 physical tiles
			Total Size: 32000 [D0, [48 x 1280, 1280]]
		KerArgItSpace (User Kernel Iter Order):
			[D0, [48 x 1280, 1280]]
		Tile0: [0, 1280, 1280], Tile1: [640, 1280, 1280], Tile2; [1280, 1280, 1280]
	Ker Arg: Out, Tiled Space: Buffer
		Min Pipe Depth: 0, Max Pipe Depth: 0
		KerArgItSpace: 49 logical tiles, 1 physical tiles
			Total Size: 3920 [D0, [48 x 80, 80]]
		KerArgItSpace (User Kernel Iter Order):
			[D0, [48 x 80, 80]]
		Tile0: [0, 3920, 3920], Tile1: [0, 3920, 3920], Tile2; [0, 3920, 3920]
	Ker Arg: In_rfft, Tiled Space: Buffer
		Min Pipe Depth: 0, Max Pipe Depth: 0
		KerArgItSpace: 1 logical tiles, 1 physical tiles
			Total Size: 2048 [Tile0, 1:[1x1024], 2]
		KerArgItSpace (User Kernel Iter Order):
			[Tile0, 1:[1x1024], 2]
		Tile0: [0, 2048, 2048], Tile1: [0, 2048, 2048], Tile2; [0, 2048, 2048]
	Ker Arg: FFT_Out, Tiled Space: Buffer
		Min Pipe Depth: 0, Max Pipe Depth: 0
		KerArgItSpace: 1 logical tiles, 1 physical tiles
			Total Size: 2052 [Tile0, 1:[1x1026], 2]
		KerArgItSpace (User Kernel Iter Order):
			[Tile0, 1:[1x1026], 2]
		Tile0: [0, 2052, 2052], Tile1: [0, 2052, 2052], Tile2; [0, 2052, 2052]
	Ker Arg: Buff2, Tiled Space: Buffer
		Min Pipe Depth: 0, Max Pipe Depth: 0
		KerArgItSpace: 1 logical tiles, 1 physical tiles
			Total Size: 4104 [Tile0, 1:[1x1026], 4]
		KerArgItSpace (User Kernel Iter Order):
			[Tile0, 1:[1x1026], 4]
		Tile0: [0, 4104, 4104], Tile1: [0, 4104, 4104], Tile2; [0, 4104, 4104]
	Ker Arg: Shift, Tiled Space: Buffer
		Min Pipe Depth: 0, Max Pipe Depth: 0
		KerArgItSpace: 1 logical tiles, 1 physical tiles
			Total Size: 2 [Tile0, 1:[1x1], 2]
		KerArgItSpace (User Kernel Iter Order):
			[Tile0, 1:[1x1], 2]
		Tile0: [0, 2, 2], Tile1: [0, 2, 2], Tile2; [0, 2, 2]
	Ker Arg: WinTable, Tiled Space: Buffer
		Min Pipe Depth: 0, Max Pipe Depth: 0
		KerArgItSpace: 1 logical tiles, 1 physical tiles
			Total Size: 2048 [Tile0, 1:[1x1024], 2]
		KerArgItSpace (User Kernel Iter Order):
			[Tile0, 1:[1x1024], 2]
		Tile0: [0, 2048, 2048], Tile1: [0, 2048, 2048], Tile2; [0, 2048, 2048]
	Ker Arg: Twiddles_fft_int, Tiled Space: Buffer
		Min Pipe Depth: 0, Max Pipe Depth: 0
		KerArgItSpace: 1 logical tiles, 1 physical tiles
			Total Size: 1024 [Tile0, 1:[1x512], 2]
		KerArgItSpace (User Kernel Iter Order):
			[Tile0, 1:[1x512], 2]
		Tile0: [0, 1024, 1024], Tile1: [0, 1024, 1024], Tile2; [0, 1024, 1024]
	Ker Arg: SwapTable_fft, Tiled Space: Buffer
		Min Pipe Depth: 0, Max Pipe Depth: 0
		KerArgItSpace: 1 logical tiles, 1 physical tiles
			Total Size: 1024 [Tile0, 1:[1x512], 2]
		KerArgItSpace (User Kernel Iter Order):
			[Tile0, 1:[1x512], 2]
		Tile0: [0, 1024, 1024], Tile1: [0, 1024, 1024], Tile2; [0, 1024, 1024]
	Ker Arg: Twiddles_rfft, Tiled Space: Buffer
		Min Pipe Depth: 0, Max Pipe Depth: 0
		KerArgItSpace: 1 logical tiles, 1 physical tiles
			Total Size: 2048 [Tile0, 1:[1x1024], 2]
		KerArgItSpace (User Kernel Iter Order):
			[Tile0, 1:[1x1024], 2]
		Tile0: [0, 2048, 2048], Tile1: [0, 2048, 2048], Tile2; [0, 2048, 2048]
	Ker Arg: Mel_FilterBank, Tiled Space: Buffer
		Min Pipe Depth: 0, Max Pipe Depth: 0
		KerArgItSpace: 1 logical tiles, 1 physical tiles
			Total Size: 240 [Tile0, 1:[1x40], 6]
		KerArgItSpace (User Kernel Iter Order):
			[Tile0, 1:[1x40], 6]
		Tile0: [0, 240, 240], Tile1: [0, 240, 240], Tile2; [0, 240, 240]
	Ker Arg: Mel_Coeffs, Tiled Space: Buffer
		Min Pipe Depth: 0, Max Pipe Depth: 0
		KerArgItSpace: 1 logical tiles, 1 physical tiles
			Total Size: 988 [Tile0, 1:[1x494], 2]
		KerArgItSpace (User Kernel Iter Order):
			[Tile0, 1:[1x494], 2]
		Tile0: [0, 988, 988], Tile1: [0, 988, 988], Tile2; [0, 988, 988]
	Ker Arg: shift_buff, Tiled Space: Buffer
		Min Pipe Depth: 0, Max Pipe Depth: 0
		KerArgItSpace: 1 logical tiles, 1 physical tiles
			Total Size: 40 [Tile0, 1:[1x40], 1]
		KerArgItSpace (User Kernel Iter Order):
			[Tile0, 1:[1x40], 1]
		Tile0: [0, 40, 40], Tile1: [0, 40, 40], Tile2; [0, 40, 40]
	Ker Arg: DCT_Coeff, Tiled Space: Buffer
		Min Pipe Depth: 0, Max Pipe Depth: 0
		KerArgItSpace: 1 logical tiles, 1 physical tiles
			Total Size: 3200 [Tile0, 1:[1x1600], 2]
		KerArgItSpace (User Kernel Iter Order):
			[Tile0, 1:[1x1600], 2]
		Tile0: [0, 3200, 3200], Tile1: [0, 3200, 3200], Tile2; [0, 3200, 3200]
	======================== End Ker Arg Iter Spaces =========================================*/
	/*=========================== Call Kernel, Invariant assignment =====================*/
	KerArg0->Out = (void * __restrict__) (L1_Memory+6480);
	KerArg0->Prev = (short int) (0);
	KerArg0->PreempFactor = (short int) (0);
	KerArg0->FrameSize = (int) (640);
	KerArg0->Shift = (short int *) (L1_Memory+14684);
	KerArg0->QIn_FFT = (short int) (13);
	KerArg1->Frame = (void *__restrict__) (L1_Memory+6480);
	KerArg1->OutFrame = (void *__restrict__) (L1_Memory+6480);
	KerArg1->Window = (void *__restrict__) (L1_Memory+14688);
	KerArg1->FrameSize = (int) (640);
	KerArg1->FFT_Dim = (int) (1024);
	KerArg2->Data = (void * __restrict__) (L1_Memory+6480);
	KerArg2->RFFT_Out = (void * __restrict__) (L1_Memory+8528);
	KerArg2->Twiddles = (void * __restrict__) (L1_Memory+16736);
	KerArg2->RTwiddles = (void * __restrict__) (L1_Memory+18784);
	KerArg2->SwapTable = (void * __restrict__) (L1_Memory+17760);
	KerArg2->N_fft = (short int) (1024);
	KerArg2->Inverse = (unsigned char) (0);
	KerArg3->FrameIn = (void *__restrict__) (L1_Memory+8528);
	KerArg3->FrameOut = (void *__restrict__) (L1_Memory+10580);
	KerArg3->Nfft = (int) (1024);
	KerArg3->Input_QFormat = (short int) (8);
	KerArg4->FramePower = (void *__restrict__) (L1_Memory+10580);
	KerArg4->MelSpectr = (void *__restrict__) (L1_Memory+6480);
	KerArg4->Mel_Coeffs = (void *__restrict__) (L1_Memory+21072);
	KerArg4->Mel_FilterBank = (fbank_type_t *__restrict__) (L1_Memory+20832);
	KerArg4->Mel_NBanks = (short int) (40);
	KerArg4->Mel_Coeff_dyn = (short int) (15);
	KerArg4->IsMagSquared = (signed char) (1);
	KerArg4->shift_buff = (signed char *__restrict__) (L1_Memory+22060);
	KerArg5->FrameIn = (void *__restrict__) (L1_Memory+6480);
	KerArg5->FrameOut = (void *__restrict__) (L1_Memory+10580);
	KerArg5->FrameSize = (unsigned int) (40);
	KerArg5->Norm = (unsigned short int) (Norm);
	KerArg5->Q_FFT_Out = (short int) (8);
	KerArg5->Mel_Coeff_Dyn = (short int) (15);
	KerArg5->IsMagSquared = (signed char) (1);
	KerArg5->shift_buff = (signed char *__restrict__) (L1_Memory+22060);
	KerArg6->Data = (void *__restrict__) (L1_Memory+10580);
	KerArg6->DCTCoeff = (void *__restrict__) (L1_Memory+22100);
	KerArg6->n_input = (short int) (40);
	KerArg6->n_dct = (short int) (40);
	/*================================= Read Tiles Prolog ===============================*/
	AT_L2_COPY(0, ((AT_L2_EXT_ADDR_TYPE) In+0), ((AT_L2_INT_ADDR_TYPE) L1_Memory+0+0), 1280, 0, &DmaR_Evt1);
	_N_In=0;
	AT_L2_COPY(0, ((AT_L2_EXT_ADDR_TYPE) WinTable+0), ((AT_L2_INT_ADDR_TYPE) L1_Memory+14688), 2048, 0, &DmaR_Evt2);
	AT_L2_WAIT(0, &DmaR_Evt2); /* Wait previous DMA read WinTable */
	AT_L2_COPY(0, ((AT_L2_EXT_ADDR_TYPE) Twiddles_fft_int+0), ((AT_L2_INT_ADDR_TYPE) L1_Memory+16736), 1024, 0, &DmaR_Evt3);
	AT_L2_WAIT(0, &DmaR_Evt3); /* Wait previous DMA read Twiddles_fft_int */
	AT_L2_COPY(0, ((AT_L2_EXT_ADDR_TYPE) SwapTable_fft+0), ((AT_L2_INT_ADDR_TYPE) L1_Memory+17760), 1024, 0, &DmaR_Evt4);
	AT_L2_WAIT(0, &DmaR_Evt4); /* Wait previous DMA read SwapTable_fft */
	AT_L2_COPY(0, ((AT_L2_EXT_ADDR_TYPE) Twiddles_rfft+0), ((AT_L2_INT_ADDR_TYPE) L1_Memory+18784), 2048, 0, &DmaR_Evt5);
	AT_L2_WAIT(0, &DmaR_Evt5); /* Wait previous DMA read Twiddles_rfft */
	AT_L2_COPY(0, ((AT_L2_EXT_ADDR_TYPE) Mel_FilterBank+0), ((AT_L2_INT_ADDR_TYPE) L1_Memory+20832), 240, 0, &DmaR_Evt6);
	AT_L2_WAIT(0, &DmaR_Evt6); /* Wait previous DMA read Mel_FilterBank */
	AT_L2_COPY(0, ((AT_L2_EXT_ADDR_TYPE) Mel_Coeffs+0), ((AT_L2_INT_ADDR_TYPE) L1_Memory+21072), 988, 0, &DmaR_Evt7);
	AT_L2_WAIT(0, &DmaR_Evt7); /* Wait previous DMA read Mel_Coeffs */
	AT_L2_COPY(0, ((AT_L2_EXT_ADDR_TYPE) DCT_Coeff+0), ((AT_L2_INT_ADDR_TYPE) L1_Memory+22100), 3200, 0, &DmaR_Evt8);
	AT_L2_WAIT(0, &DmaR_Evt8); /* Wait previous DMA read DCT_Coeff */
	/*============================= End Read Tiles Prolog ===============================*/
	for (D0Ind=0; D0Ind<49; D0Ind++, D0Ind_Total++) { /* Iteration on D0 */
		int D0Ind_Last = (D0Ind==48), D0Ind_NextLast = ((D0Ind+1)==48);
		/*================================= Prepare Tiles ===================================*/
		_SN_In = 0;
		if (!(D0Ind_Last)) {
			_N_In = _N_In + (640); _SN_In = (1280); 
		}
		/*============================= End Prepare Tiles ===================================*/
		/*================================= Read Tiles ======================================*/
		AT_L2_WAIT(0, &DmaR_Evt1); /* Wait previous DMA read In */
		if (_SN_In) {
			AT_L2_COPY(0, ((AT_L2_EXT_ADDR_TYPE) In+_N_In), ((AT_L2_INT_ADDR_TYPE) L1_Memory+0+1280*((D0Ind_Total+1)%2)),
					_SN_In, 0, &DmaR_Evt1);
		}
		/*============================= End Read Tiles ======================================*/
		{ /* Single iteration on Tile0 */
			int T0Ind_Last = 1;
			/*====================== Call Kernel LOC_LOOP =========================*/
			KerArg0->Frame = (void * __restrict__) (L1_Memory+0+1280*((D0Ind_Total)%2));
			AT_FORK(gap_ncore(), (void *) PreEmphasis, (void *) KerArg0);
			__CALL(PreEmphasis, KerArg0);
			AT_FORK(gap_ncore(), (void *) WindowingReal2Real_Fix16, (void *) KerArg1);
			__CALL(WindowingReal2Real_Fix16, KerArg1);
			AT_FORK(gap_ncore(), (void *) RFFT_DIF_Par_Fix16, (void *) KerArg2);
			__CALL(RFFT_DIF_Par_Fix16, KerArg2);
			KerArg3->ExtraQ = (short int) (((short int *)(L1_Memory+14684))[0]);
			AT_FORK(gap_ncore(), (void *) CmplxMagSquared_Fix16, (void *) KerArg3);
			__CALL(CmplxMagSquared_Fix16, KerArg3);
			AT_FORK(gap_ncore(), (void *) MelFilterBank_Fix32, (void *) KerArg4);
			__CALL(MelFilterBank_Fix32, KerArg4);
			KerArg5->ExtraQ = (short int) (((short int *)(L1_Memory+14684))[0]);
			AT_FORK(gap_ncore(), (void *) MFCC_ComputeLog_Fix32, (void *) KerArg5);
			__CALL(MFCC_ComputeLog_Fix32, KerArg5);
			KerArg6->FeatList = (void *__restrict__) (L1_Memory+2560+((D0Ind)*80));
			AT_FORK(gap_ncore(), (void *) MFCC_ComputeDCT_II_Fix16, (void *) KerArg6);
			__CALL(MFCC_ComputeDCT_II_Fix16, KerArg6);
		} /* End iteration on Tile0 */
		/*================================= Update Arg Pipeline =============================*/
		/*============================= End Update Arg Pipeline =============================*/
	} /* End iteration on D0 */
	/*================================ Write Tiles Epilog ===============================*/
	AT_L2_COPY(0, ((AT_L2_EXT_ADDR_TYPE) Out+0), ((AT_L2_INT_ADDR_TYPE) L1_Memory+2560), 3920, 1, &DmaW_Evt1);
	AT_L2_WAIT(0, &DmaW_Evt1); /* Wait DMA write Out */
	/*============================ End Write Tiles Epilog ===============================*/
}


void STFT(
		short int * __restrict__ In,
		short int * __restrict__ Out,
		short int * __restrict__ Twiddles_fft_int,
		short int * __restrict__ Twiddles_rfft,
		short int * SwapTable_fft,
		short int * __restrict__ WinTable)

{
	/* Shared L1: 5956 bytes, L2 buffer: 0 bytes */
	/* Local variables used by this kernel */
	AT_L2_EVENT _DmaR_Evt1, *DmaR_Evt1 = &_DmaR_Evt1;
	AT_L2_EVENT _DmaW_Evt1, *DmaW_Evt1 = &_DmaW_Evt1;
	AT_L2_EVENT _DmaR_Evt2, *DmaR_Evt2 = &_DmaR_Evt2;
	AT_L2_EVENT _DmaR_Evt3, *DmaR_Evt3 = &_DmaR_Evt3;
	AT_L2_EVENT _DmaR_Evt4, *DmaR_Evt4 = &_DmaR_Evt4;
	AT_L2_EVENT _DmaR_Evt5, *DmaR_Evt5 = &_DmaR_Evt5;
	Windowing_T S_KerArg0, *KerArg0 = &S_KerArg0;
	RFFT_Arg_T S_KerArg1, *KerArg1 = &S_KerArg1;

	/* Iteration space related variables */
	int D0Ind, D0Ind_Last;
	int T0Ind, T0Ind_Last;
	/* User kernel arguments related variables */
	/*============================= Ker Arg Iter Spaces =========================================
	User Kernel Iteration Space:
		[D0 Dim: 1][Tile0 Dim: 1]
	Ker Arg: In, Tiled Space: Buffer
		Min Pipe Depth: 0, Max Pipe Depth: 0
		KerArgItSpace: 1 logical tiles, 1 physical tiles
			@ 0 (Total Size: 800 )[D0, [0 x 800, 800]]
		KerArgItSpace (User Kernel Iter Order):
			[D0, [0 x 800, 800]]
		Tile0: [0, 800, 800], Tile1: [0, 800, 800], Tile2; [0, 800, 800]
	Ker Arg: Out, Tiled Space: Buffer
		Min Pipe Depth: 0, Max Pipe Depth: 0
		KerArgItSpace: 1 logical tiles, 1 physical tiles
			@ 800 (Total Size: 1028 )[D0, [0 x 1028, 1028]]
		KerArgItSpace (User Kernel Iter Order):
			[D0, [0 x 1028, 1028]]
		Tile0: [0, 1028, 1028], Tile1: [0, 1028, 1028], Tile2; [0, 1028, 1028]
	Ker Arg: In_rfft, Tiled Space: Buffer
		Min Pipe Depth: 0, Max Pipe Depth: 0
		KerArgItSpace: 1 logical tiles, 1 physical tiles
			@ 1828 (Total Size: 1024 )[Tile0, 1:[1x512], 2]
		KerArgItSpace (User Kernel Iter Order):
			[Tile0, 1:[1x512], 2]
		Tile0: [0, 1024, 1024], Tile1: [0, 1024, 1024], Tile2; [0, 1024, 1024]
	Ker Arg: WinTable, Tiled Space: Buffer
		Min Pipe Depth: 0, Max Pipe Depth: 0
		KerArgItSpace: 1 logical tiles, 1 physical tiles
			@ 2852 (Total Size: 800 )[Tile0, 1:[1x400], 2]
		KerArgItSpace (User Kernel Iter Order):
			[Tile0, 1:[1x400], 2]
		Tile0: [0, 800, 800], Tile1: [0, 800, 800], Tile2; [0, 800, 800]
	Ker Arg: Twiddles_fft_int, Tiled Space: Buffer
		Min Pipe Depth: 0, Max Pipe Depth: 0
		KerArgItSpace: 1 logical tiles, 1 physical tiles
			@ 3652 (Total Size: 768 )[Tile0, 1:[1x384], 2]
		KerArgItSpace (User Kernel Iter Order):
			[Tile0, 1:[1x384], 2]
		Tile0: [0, 768, 768], Tile1: [0, 768, 768], Tile2; [0, 768, 768]
	Ker Arg: SwapTable_fft, Tiled Space: Buffer
		Min Pipe Depth: 0, Max Pipe Depth: 0
		KerArgItSpace: 1 logical tiles, 1 physical tiles
			@ 4420 (Total Size: 512 )[Tile0, 1:[1x256], 2]
		KerArgItSpace (User Kernel Iter Order):
			[Tile0, 1:[1x256], 2]
		Tile0: [0, 512, 512], Tile1: [0, 512, 512], Tile2; [0, 512, 512]
	Ker Arg: Twiddles_rfft, Tiled Space: Buffer
		Min Pipe Depth: 0, Max Pipe Depth: 0
		KerArgItSpace: 1 logical tiles, 1 physical tiles
			@ 4932 (Total Size: 1024 )[Tile0, 1:[1x512], 2]
		KerArgItSpace (User Kernel Iter Order):
			[Tile0, 1:[1x512], 2]
		Tile0: [0, 1024, 1024], Tile1: [0, 1024, 1024], Tile2; [0, 1024, 1024]
	======================== End Ker Arg Iter Spaces =========================================*/
	/*=========================== Call Kernel, Invariant assignment =====================*/
	KerArg0->Frame = (void *__restrict__) (L1_Memory+0);
	KerArg0->OutFrame = (void *__restrict__) (L1_Memory+1828);
	KerArg0->Window = (void *__restrict__) (L1_Memory+2852);
	KerArg0->FrameSize = (int) (400);
	KerArg0->FFT_Dim = (int) (512);
	KerArg1->Data = (void * __restrict__) (L1_Memory+1828);
	KerArg1->RFFT_Out = (void * __restrict__) (L1_Memory+800);
	KerArg1->Twiddles = (void * __restrict__) (L1_Memory+3652);
	KerArg1->RTwiddles = (void * __restrict__) (L1_Memory+4932);
	KerArg1->SwapTable = (void * __restrict__) (L1_Memory+4420);
	KerArg1->N_fft = (short int) (512);
	KerArg1->Inverse = (unsigned char) (0);
	/*================================= Read Tiles Prolog ===============================*/
	AT_L2_COPY(0, ((AT_L2_EXT_ADDR_TYPE) In+0), ((AT_L2_INT_ADDR_TYPE) L1_Memory+0), 800, 0, DmaR_Evt1);
	AT_L2_WAIT(0, DmaR_Evt1); /* Wait previous DMA read In */
	AT_L2_COPY(0, ((AT_L2_EXT_ADDR_TYPE) WinTable+0), ((AT_L2_INT_ADDR_TYPE) L1_Memory+2852), 800, 0, DmaR_Evt2);
	AT_L2_WAIT(0, DmaR_Evt2); /* Wait previous DMA read WinTable */
	AT_L2_COPY(0, ((AT_L2_EXT_ADDR_TYPE) Twiddles_fft_int+0), ((AT_L2_INT_ADDR_TYPE) L1_Memory+3652), 768, 0, DmaR_Evt3);
	AT_L2_WAIT(0, DmaR_Evt3); /* Wait previous DMA read Twiddles_fft_int */
	AT_L2_COPY(0, ((AT_L2_EXT_ADDR_TYPE) SwapTable_fft+0), ((AT_L2_INT_ADDR_TYPE) L1_Memory+4420), 512, 0, DmaR_Evt4);
	AT_L2_WAIT(0, DmaR_Evt4); /* Wait previous DMA read SwapTable_fft */
	AT_L2_COPY(0, ((AT_L2_EXT_ADDR_TYPE) Twiddles_rfft+0), ((AT_L2_INT_ADDR_TYPE) L1_Memory+4932), 1024, 0, DmaR_Evt5);
	AT_L2_WAIT(0, DmaR_Evt5); /* Wait previous DMA read Twiddles_rfft */
	/*============================= End Read Tiles Prolog ===============================*/
	{ /* Single iteration on D0 */
		int D0Ind_Last = 1;
		{ /* Single iteration on Tile0 */
			int T0Ind_Last = 1;
			/*====================== Call Kernel LOC_LOOP =========================*/
			AT_FORK(gap_ncore(), (void *) WindowingReal2Real_f16, (void *) KerArg0);
			__CALL(WindowingReal2Real_f16, KerArg0);
			AT_FORK(gap_ncore(), (void *) RFFT_DIF_Par_Fix16, (void *) KerArg1);
			__CALL(RFFT_DIF_Par_Fix16, KerArg1);
		} /* End iteration on Tile0 */
	} /* End iteration on D0 */
	/*================================ Write Tiles Epilog ===============================*/
	AT_L2_COPY(0, ((AT_L2_EXT_ADDR_TYPE) Out+0), ((AT_L2_INT_ADDR_TYPE) L1_Memory+800), 1028, 1, DmaW_Evt1);
	AT_L2_WAIT(0, DmaW_Evt1); /* Wait DMA write Out */
	/*============================ End Write Tiles Epilog ===============================*/
}
