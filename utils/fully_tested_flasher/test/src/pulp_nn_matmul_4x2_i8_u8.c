/*
 * pulp_nn_matmul_4x2_i8_u8.c
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

#define SumDotp(a, b, c)    __builtin_pulp_sdotusp4(a, b, c)
#define nn_round(out_shift) (0x1 << (out_shift -1))
#define clip8(x)            __builtin_pulp_clipu_r(x, 255)

uint8_t __attribute__ ((noinline)) *pulp_nn_matmul_4x2_i8_u8(
  const int8_t * pWeight,
  uint8_t *      pInBuffer,
  int32_t *      pInBufferAcc,
  uint16_t       ch_out,
  uint16_t       num_col_im2col,
  uint16_t       bias_shift,
  uint16_t       out_shift,
  uint16_t       out_mult,
  int16_t *      k,
  int32_t *      lambda,
  const int8_t * bias,
  uint8_t *      pOut,
  int32_t *      pOutBufferAcc,
  int            flag_relu,
  int            flag_batch_norm,
  int            flag_acc_buff_in,
  int            flag_acc_buff_out,
  int            flag_first_ch_in,
  int            flag_first_ch_out,
  int8_t         displacement
) {
	uint8_t *pOut2 = pOut + ch_out;
  int8_t  *pA = pWeight;
  uint16_t chan_left = ch_out & 0x3;

  int32_t *pInAcc_A = pInBufferAcc;
  int32_t *pInAcc_B = pInAcc_A + num_col_im2col;

  int32_t *pOutAcc_A = pOutBufferAcc;
  int32_t *pOutAcc_B = pOutAcc_A + num_col_im2col;

  int32_t bias_acc_A = (*pInAcc_A * displacement);
  int32_t bias_acc_B = (*pInAcc_B * displacement);

  int32_t *partial_res_A;
  int32_t *partial_res_B;

  v4s vecA;
  v4s vecA2;
  v4s vecA3;
  v4s vecA4;
  v4u vecB;
  v4u vecB2;

  /* this loop over the OFM channels */
  for (int i = 0; i < ch_out>>2; i++)
  {
    uint8_t *pB  =  pInBuffer ;
    uint8_t *pB2 = (pB + num_col_im2col);
    int8_t *pA2 = (pA + num_col_im2col);
    int8_t *pA3 = (pA2 + num_col_im2col);
    int8_t *pA4 = (pA3 + num_col_im2col);
    int32_t *pB_acc  = pInBufferAcc;
    int32_t *pB2_acc = pB_acc + num_col_im2col;

    int bias1 = 0;
    int bias2 = 0;
    int bias3 = 0;
    int bias4 = 0;
    int bias5 = 0;
    int bias6 = 0;
    int bias7 = 0;
    int bias8 = 0;

    /* when channel input are tiled, the reloading of partial results is needed until the tiling of input channel are completed.
        I'm assuming that this flag are 1 when is perfoming the tiling and 0 when isn't performing the tiling OR we are having the
        first tiling block (partial results are zeros). */
    if(flag_first_ch_in == 0)
    {
      /* in a 4x2 mat mult kernel there are 8 partial results at each iteration, so 4 bias more are needed than standard case.
          probably doesn't work because pOut enter as index zero and then to have negative index isn't possible. Maybe these
          operations are necessary in conv function, before the calling of mat mul function. */

      // FIXME -- this cannot work!!! partial results must be saved in FULL precision (INT32)!!!
      partial_res_A = (pOut - (ch_out << 1));
      partial_res_B = (pOut - ch_out);

      bias1 = *partial_res_A++;
      bias2 = *partial_res_A++;
      bias3 = *partial_res_A++;
      bias4 = *partial_res_A++;

      bias5 = *partial_res_B++;
      bias6 = *partial_res_B++;
      bias7 = *partial_res_B++;
      bias8 = *partial_res_B++;
    }
    else if(bias != NULL)
    {
      bias1 = ((int) (*bias++)  << bias_shift) + nn_round(out_shift);
      bias2 = ((int) (*bias++)  << bias_shift) + nn_round(out_shift);
      bias3 = ((int) (*bias++)  << bias_shift) + nn_round(out_shift);
      bias4 = ((int) (*bias++)  << bias_shift) + nn_round(out_shift);

      bias5 = bias1;
      bias6 = bias2;
      bias7 = bias3;
      bias8 = bias4;
    }

    /* init the accumulators with corresponding biases. if there is the accumulation buffer, i included it here */
    int sum =  bias1;
    int sum2 = bias2;
    int sum3 = bias3;
    int sum4 = bias4;

    int sum5 = bias5;
    int sum6 = bias6;
    int sum7 = bias7;
    int sum8 = bias8;

    if(flag_acc_buff_in == 1)
    {
      sum+=bias_acc_A;
      sum2+=bias_acc_A;
      sum3+=bias_acc_A;
      sum4+=bias_acc_A;

      sum5+=bias_acc_B;
      sum6+=bias_acc_B;
      sum7+=bias_acc_B;
      sum8+=bias_acc_B;
    }

    uint16_t  col_cnt_im2col = num_col_im2col & 0x3;

    for (int j=0; j < num_col_im2col >> 2 ; j++)
    {
      vecA  = * ( (v4s*) pA  );
      vecA2 = * ( (v4s*) pA2 );
      vecA3 = * ( (v4s*) pA3 );
      vecA4 = * ( (v4s*) pA4 );
      vecB  = * ( (v4u*) pB  );
      vecB2 = * ( (v4u*) pB2 );

      sum  =  SumDotp (vecB,  vecA,  sum  );
      sum2 =  SumDotp (vecB,  vecA2, sum2 );
      sum3 =  SumDotp (vecB,  vecA3, sum3 );
      sum4 =  SumDotp (vecB,  vecA4, sum4 );

      sum5 =  SumDotp (vecB2, vecA,  sum5 );
      sum6 =  SumDotp (vecB2, vecA2, sum6 );
      sum7 =  SumDotp (vecB2, vecA3, sum7 );
      sum8 =  SumDotp (vecB2, vecA4, sum8 );

      pA  += 4;
      pA2 += 4;
      pA3 += 4;
      pA4 += 4;
      pB  += 4;
      pB2 += 4;
    }

    while(col_cnt_im2col)
    {
      int8_t      inA  = *pA++;
      int8_t      inA2 = *pA2++;
      int8_t      inA3 = *pA3++;
      int8_t      inA4 = *pA4++;
      uint8_t     inB  = *pB++;
      uint8_t     inB2 = *pB2++;
      asm volatile("": : :"memory");
      sum  += inA  * inB;
      sum2 += inA2 * inB;
      sum3 += inA3 * inB;
      sum4 += inA4 * inB;
      sum5 +=  inA * inB2;
      sum6 += inA2 * inB2;
      sum7 += inA3 * inB2;
      sum8 += inA4 * inB2;

      col_cnt_im2col--;
    }

    /* for example if conv kernel is followed by max pool layer, the computation of accumulation buffer isn't necessary at this stage */
    if (flag_acc_buff_out == 1)
    {
      if (flag_first_ch_out) {
        *pOutAcc_A  = sum  + sum2 + sum3 + sum4;
        *pOutAcc_B  = sum5 + sum6 + sum7 + sum8;
      }
      else {
        *pOutAcc_A += sum  + sum2 + sum3 + sum4;
        *pOutAcc_B += sum5 + sum6 + sum7 + sum8;
      }
    }
    if (flag_batch_norm && flag_relu)
    {
      *pOut = pulp_nn_bn_quant_u8(sum, *k, *lambda, out_mult, out_shift);
      pOut++;
      *pOut2 = pulp_nn_bn_quant_u8(sum5, *k, *lambda, out_mult, out_shift);
      pOut2++;
      k++;
      lambda++;

      *pOut = pulp_nn_bn_quant_u8(sum2, *k, *lambda, out_mult, out_shift);
      pOut++;
      *pOut2 = pulp_nn_bn_quant_u8(sum6, *k, *lambda, out_mult, out_shift);
      pOut2++;
      k++;
      lambda++;

      *pOut = pulp_nn_bn_quant_u8(sum3, *k, *lambda, out_mult, out_shift);
      pOut++;
      *pOut2 = pulp_nn_bn_quant_u8(sum7, *k, *lambda, out_mult, out_shift);
      pOut2++;
      k++;
      lambda++;

      *pOut = pulp_nn_bn_quant_u8(sum4, *k, *lambda, out_mult, out_shift);
      pOut++;
      *pOut2 = pulp_nn_bn_quant_u8(sum8, *k, *lambda, out_mult, out_shift);
      pOut2++;
      k++;
      lambda++;
    }
    else
    {
      if (flag_relu == 1)
      {
        *pOut = pulp_nn_quant_u8(sum, out_mult, out_shift);
        pOut++;
        *pOut = pulp_nn_quant_u8(sum2, out_mult, out_shift);
        pOut++;
        *pOut = pulp_nn_quant_u8(sum3, out_mult, out_shift);
        pOut++;
        *pOut = pulp_nn_quant_u8(sum4, out_mult, out_shift);
        pOut++;

        *pOut2 = pulp_nn_quant_u8(sum5, out_mult, out_shift);
        pOut2++;
        *pOut2 = pulp_nn_quant_u8(sum6, out_mult, out_shift);
        pOut2++;
        *pOut2 = pulp_nn_quant_u8(sum7, out_mult, out_shift);
        pOut2++;
        *pOut2 = pulp_nn_quant_u8(sum8, out_mult, out_shift);
        pOut2++;
      }
      else
      {
        *pOut = (uint8_t) clip8(sum >> out_shift);
        pOut++;
        *pOut = (uint8_t) clip8(sum2 >> out_shift);
        pOut++;
        *pOut = (uint8_t) clip8(sum3 >> out_shift);
        pOut++;
        *pOut = (uint8_t) clip8(sum4 >> out_shift);
        pOut++;

        *pOut2 = (uint8_t) clip8(sum5 >> out_shift);
        pOut2++;
        *pOut2 = (uint8_t) clip8(sum6 >> out_shift);
        pOut2++;
        *pOut2 = (uint8_t) clip8(sum7 >> out_shift);
        pOut2++;
        *pOut2 = (uint8_t) clip8(sum8 >> out_shift);
        pOut2++;
      }
    }

    pA +=  3 * num_col_im2col;
  }

  while(chan_left)
  {
    uint8_t *pB  =  pInBuffer ;
    uint8_t *pB2 = (pB + num_col_im2col);

    int bias1 = 0;
    int bias2 = 0;

    if(flag_first_ch_in == 0)
    {
      // FIXME -- this cannot work!!! partial results must be saved in FULL precision (INT32)!!!
      bias1 = *partial_res_A++;
      bias2 = *partial_res_B++;
    }
    else if (bias != NULL)
    {
      bias1 = ((int) (*bias++)  << bias_shift) + nn_round(out_shift);
      bias2 = bias1;
    }

    int sum  =  bias1;
    int sum2 =  bias2;

    if(flag_acc_buff_in == 1)
    {
      sum += bias_acc_A;
      sum += bias_acc_B;
    }

    for (int j=0; j < num_col_im2col >> 2 ; j++)
    {
      vecA  = * ( (v4s*) pA  );
      vecB  = * ( (v4u*) pB  );
      vecB2 = * ( (v4u*) pB2 );

      sum  =  SumDotp (vecB, vecA, sum  );
      sum2 =  SumDotp (vecB2,vecA, sum2 );

      pA  += 4;
      pB  += 4;
      pB2 += 4;
    }

    uint16_t  col_cnt_im2col =num_col_im2col & 0x3;

    while(col_cnt_im2col)
    {
      int8_t      inA  = *pA++;
      uint8_t     inB  = *pB++;
      uint8_t     inB2 = *pB2++;
      asm volatile("": : :"memory");
      sum  += inA  * inB;
      sum2 +=  inA * inB2;

      col_cnt_im2col--;
    }
    if (flag_acc_buff_out == 1)
    {
      if (flag_first_ch_out == 1)
      {
        *pOutAcc_A  = sum;
        *pOutAcc_B  = sum2;
      }
      else
      {
        *pOutAcc_A += sum;
        *pOutAcc_B += sum2;
      }
    }
    if (flag_batch_norm && flag_relu)
    {
      *pOut = pulp_nn_bn_quant_u8(sum, *k, *lambda, out_mult, out_shift);
      pOut++;
      *pOut2 = pulp_nn_bn_quant_u8(sum2, *k, *lambda, out_mult, out_shift);
      pOut2++;
      k++;
      lambda++;
    }
    else
    {
      if (flag_relu == 1)
      {
        *pOut = pulp_nn_quant_u8(sum, out_mult, out_shift);
        pOut++;

        *pOut2 = pulp_nn_quant_u8(sum2, out_mult, out_shift);
        pOut2++;
      }
      else
      {
        *pOut = (uint8_t) clip8(sum >> out_shift);
        pOut++;

        *pOut2 = (uint8_t) clip8(sum2 >> out_shift);
        pOut2++;
      }
    }
    chan_left--;
  }
  pOut += ch_out;
  return pOut;
}
