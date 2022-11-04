/*
 * pulp_nn_relu_u8.c
 * Angelo Garofalo <angelo.garofalo@unibo.it>
 * 
 * Copyright (C) 2019 ETH Zurich, University of Bologna.
 * All rights reserved.
 */

#include "rt/rt_api.h"
#include "utils.h"
#include "kernels.h"

#define log2(x) __builtin_pulp_fl1(x)
#define min(a,b) ((a)<(b)?(a):(b))
#define max4(x,y) __builtin_pulp_max4(x,y)

void __attribute__((always_inline)) pulp_nn_relu_u8(
  int8_t * data,
  uint16_t dim_im_in_x,
  uint16_t dim_im_in_y,
  uint16_t ch_im_in
) {
  int core_id = rt_core_id();
  int  Log2Core = log2(NUM_CORES );
  int chunck = (ch_im_in >> Log2Core ) + ((ch_im_in & (NUM_CORES-1))!=0);
  int start = min(chunck * core_id, ch_im_in);
  int stop = min(start + chunck, ch_im_in);
  int8_t *pOut = data + start * dim_im_in_x * dim_im_in_y;
  int8_t *pIn = data + start * dim_im_in_x * dim_im_in_y;
  int dimension = (stop-start) * dim_im_in_x * dim_im_in_y;
  v4s in;
  v4s in2;
  v4s mask =  (v4s) 0x00000000;
  for(int i=0; i< (dimension)>>3; i++)
  {
    in = *((v4s*) (pIn));
      pIn +=4;
      in2 = *((v4s*) (pIn));
      *((v4s*) (pOut)) = max4(in,mask);
      *((v4s*) (pIn)) = max4(in2,mask);
      pIn +=4;
      pOut +=8;
  }
  if (((dimension) & 0x7)!=0)
  {
    for(int i=0; i< (dimension & 0x7); i++)
    {
      if (*(pIn) < 0)
        *(pIn) = *(pIn) & 0x0;
      else
        *(pIn) = *(pIn);
      pIn+=1;
    }
  }
  rt_team_barrier();
}
