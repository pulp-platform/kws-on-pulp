/*
 * pulp_nn_maxpool_u8.c
 * Angelo Garofalo <angelo.garofalo@unibo.it>
 * Nazareno Bruschi <nazareno.bruschi@studio.unibo.it>
 * Francesco Conti <f.conti@unibo.it>
 * 
 * Copyright (C) 2019 ETH Zurich, University of Bologna.
 * All rights reserved.
 */

#include "rt/rt_api.h"
#include "utils.h"
#include "kernels.h"

#define log2(x) __builtin_pulp_fl1(x)
#define min(a,b) ((a)<(b)?(a):(b))

void __attribute__ ((noinline))  pulp_nn_add_u8_u8 (
	uint8_t * Im_in_1,             // pointer to the input feature map1
	uint8_t * Im_in_2,             // pointer to the input feature map2
	uint16_t  ch_im_in,          // number of channels of the IFM
	uint16_t  dim_im_in_h,      
	uint16_t  dim_im_in_w,
	uint8_t * Im_out,            // pointer to the output
	uint16_t out_mult1,            // paramter to requantize
	uint16_t out_mult2,            // paramter to requantize
	uint16_t out_shift            // paramter to requantize
) 
{
	int core_id = rt_core_id();
	int n_cores = NUM_CORES;
	if (dim_im_in_h < NUM_CORES)
	{
	  n_cores = dim_im_in_h;
	}
	int  Log2Core = log2(n_cores);

	int chunck = (dim_im_in_h >> Log2Core) + ((dim_im_in_h & (NUM_CORES-1))!=0);

	int start = min(chunck * core_id, dim_im_in_h);
	int stop = min(start + chunck, dim_im_in_h);
	uint8_t *target1 = Im_in_1 + start*ch_im_in*dim_im_in_w;
	uint8_t *target2 = Im_in_2 + start*ch_im_in*dim_im_in_w;
	uint8_t *pOut = Im_out + start*ch_im_in*dim_im_in_w;
	for (int spatial = 0; spatial<dim_im_in_w*ch_im_in*(stop-start); spatial+=1)
	{
		*pOut = pulp_nn_add_quant_u8(*target1, *target2, out_mult1, out_mult2, out_shift);
		target1 += 1;
		target2 += 1;
		pOut += 1;
	}
   rt_team_barrier();
}