/*
 * pulp_nn_dw_conv_i8_u8.c
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
#define SumDotp(a, b, c) __builtin_pulp_sdotusp4(a, b, c)
#define MIN(a,b) ((a)<(b)?(a):(b))
#define clip8(x) __builtin_pulp_clipu_r(x, 255)
#define NN_ROUND(out_shift) ((out_shift) ? (0x1 << (out_shift -1)) : (0))


void dw_fast_C_parallel_int8(
  const uint8_t * Im_in,
  const uint16_t  dim_im_in_x,
  const uint16_t  dim_im_in_y,
  const uint16_t  ch_im_in,
  const int8_t *  wt,
  const uint16_t  ch_im_out,
  const uint16_t  dim_kernel_x,
  const uint16_t  dim_kernel_y,
  const uint16_t  padding_y_top,
  const uint16_t  padding_y_bottom,
  const uint16_t  padding_x_left,
  const uint16_t  padding_x_right,
  const uint16_t  stride_x,
  const uint16_t  stride_y,
  const int8_t *  bias,
  const uint16_t  bias_shift,
  uint16_t        out_shift,
  uint16_t        out_mult,
  int16_t *       k,
  int32_t *       lambda,
  uint8_t *       Im_out,
  const uint16_t  dim_im_out_x,
  const uint16_t  dim_im_out_y,
  uint8_t *       bufferC,
  uint8_t *       bufferB,
  int32_t *       pOutBufferAcc,
  int8_t          FLAG_RELU,
  int8_t          FLAG_BATCH_NORM,
  int8_t          flag_acc_buff_out,
  int8_t          flag_first_ch_out
) {
  int core_id = rt_core_id();
  int Log2Core = log2(NUM_CORES);
  int chunk = (ch_im_out >> Log2Core) + ((ch_im_out & (NUM_CORES - 1)) != 0);

  uint16_t start_channel = MIN(chunk * core_id, ch_im_out);
  uint16_t stop_channel = MIN(start_channel + chunk, ch_im_out);

  uint16_t dim_kernel_x_size_rem = dim_kernel_x & 0x3;
  uint16_t dim_kernel_x_size_padded = (dim_kernel_x >> 2) + (dim_kernel_x_size_rem != 0);
  uint16_t dim_incr = (dim_kernel_x_size_padded << 2) - dim_kernel_x;
  uint16_t dim_incr_pad_left = (dim_kernel_x_size_padded << 2) - (dim_kernel_x - padding_x_left);
  uint16_t dim_incr_pad_right = (dim_kernel_x_size_padded << 2) - (dim_kernel_x - padding_x_right);
  // possibile incremento di 4 
  uint8_t * bufferA = bufferC + (core_id * ((dim_kernel_x * (dim_im_in_y + padding_y_top + padding_y_bottom)) + dim_incr+4));

  int16_t i_out_ch, i_out_x, i_buff_y;

  uint16_t colCnt = (dim_kernel_y * dim_kernel_x) >> 2;
  uint16_t leftCnt = (dim_kernel_y * dim_kernel_x) & 0x3;
  int16_t * k1 = k+start_channel;
  int32_t * lambda1 = lambda+start_channel;
  // printf("[%d]: chunk=%d, start=%d, stop=%d\nIm_in=%X, Im_out=%X\n", core_id, chunk, start_channel, stop_channel, Im_in, Im_out);
  // if (core_id==0)
  //   printf("First 4 pixels in filter %d %d %d %d", *(Im_in),*(Im_in+1),*(Im_in+20),*(Im_in+21) );
  for (i_out_ch = start_channel; i_out_ch < stop_channel; i_out_ch++)
  {
    //printf("[%d]: !! CHANNEL NUMBER %d !!\n", core_id, i_out_ch);
    for (i_out_x = 0; i_out_x < dim_im_out_x; i_out_x++)
    {
      //printf("[%d]: buffer number %d\n", core_id, i_out_x);
      uint8_t *pOut = Im_out + i_out_ch + (i_out_x * ch_im_out);
      uint8_t *pBuffer = bufferA;

      //int32_t *pBuffAcc = pOutBufferAcc;

      for (i_buff_y = - padding_y_top; i_buff_y < dim_im_in_y + padding_y_bottom; i_buff_y++)
      {
        if((i_buff_y < 0) || (i_buff_y >= dim_im_in_y))
        {
          //printf("[%d]: --> padding top/bottom --> pBuff=%X, pOut=%X\n", core_id, pBuffer, pOut);
          for (int i=0; i<dim_kernel_x_size_padded; i++)
          {
            *(v4u *) pBuffer = (v4u) {0, 0, 0, 0};
            pBuffer+=4;
          }
          pBuffer-=dim_incr;
          //printf("[%d]: ----> pbuff=%X\n", core_id, pBuffer);
        }
        else
        {
          if((i_out_x * stride_x) < padding_x_left)
          {
            //printf("[%d]: --> padding left\n", core_id);
            for(int j=0; j<(padding_x_left - (i_out_x * stride_x)); j++)
            {
              *(uint8_t *) pBuffer = 0;
              pBuffer++;
            }
            //printf("[%d]: ----> pbuff=%X\n", core_id, pBuffer);
            //printf("[%d]: --> partial im2col (left)\n", core_id);
            for (int i=0; i<dim_kernel_x_size_padded; i++)
            {
              *((v4u*) pBuffer) = *((v4u*) (Im_in + (i_out_ch * dim_im_in_x * dim_im_in_y) + (i_buff_y * dim_im_in_x) + (i << 2)));
              pBuffer+=4;
            }
            pBuffer-=(dim_incr_pad_left - (i_out_x * stride_x));
            //printf("[%d]: ----> pbuff=%X, in=%X\n", core_id, pBuffer, (Im_in + (i_out_ch * dim_im_in_x * dim_im_in_y) + (i_buff_y * dim_im_in_x)));
          }
          else if(((i_out_x * stride_x) + dim_kernel_x) > (dim_im_in_x + padding_x_left))
          {
            //printf("[%d]: --> partial im2col (right)\n", core_id);
            for (int i=0; i<dim_kernel_x_size_padded; i++)
            {
              *((v4u*) pBuffer) = *((v4u*) (Im_in + (i_out_ch * dim_im_in_x * dim_im_in_y) + (i_buff_y * dim_im_in_x) + (i_out_x * stride_x) - padding_x_left + (i << 2)));
              pBuffer+=4;
            }
            // printf("[%d]: ----> pbuff=%X, in=%X\n", core_id, pBuffer, (Im_in + (i_out_ch * dim_im_in_x * dim_im_in_y) + (i_buff_y * dim_im_in_x) + (i_out_x * stride_x) - padding_x_right));
            pBuffer-=(dim_incr_pad_right - ((dim_im_in_x + padding_x_left) - ((i_out_x) * stride_x) - (dim_kernel_x-stride_x)));
            // printf("[%d]: ----> pbuff=%X, in=%X\n", core_id, pBuffer, (Im_in + (i_out_ch * dim_im_in_x * dim_im_in_y) + (i_buff_y * dim_im_in_x) + (i_out_x * stride_x) - padding_x_right));
            //printf("[%d]: --> padding right\n", core_id);
            for(int j=0; j<(padding_x_right - ((dim_im_in_x + padding_x_left) - ((i_out_x) * stride_x) - (dim_kernel_x-stride_x))); j++)
            {
              *(uint8_t *) pBuffer = 0;
              pBuffer++;
            }
            // printf("[%d]: ----> pbuff=%X, in=%X\n", core_id, pBuffer, (Im_in + (i_out_ch * dim_im_in_x * dim_im_in_y) + (i_buff_y * dim_im_in_x) + (i_out_x * stride_x) - padding_x_right));
            //printf("[%d]: ----> pbuff=%X\n", core_id, pBuffer);
          }
          else
          {
            //printf("[%d]: --> im2col\n", core_id);
            for (int i=0; i<dim_kernel_x_size_padded; i++)
            {
              *((v4u*) pBuffer) = *((v4u*) (Im_in + (i_out_ch * dim_im_in_x * dim_im_in_y) + (i_buff_y * dim_im_in_x) + (i_out_x * stride_x) - padding_x_left + (i << 2)));
              pBuffer+=4;
            }
            pBuffer-=dim_incr;
            //printf("[%d]: ----> pbuff=%X, in=%X\n", core_id, pBuffer, (Im_in + (i_out_ch * dim_im_in_x * dim_im_in_y) + ((i_buff_y * dim_im_in_x) + (i_out_x * stride_x) - padding_x_left)));
          }
        }
      }



      for(int l=0; l<dim_im_out_y; l++)
      {
        int8_t *pW = wt + (i_out_ch * dim_kernel_y * dim_kernel_x);
        int sum = 0;

        pBuffer = (bufferA + ((l * stride_y) * dim_kernel_x));
        //printf("[%d]: ------> pbuff=%X\n", core_id, pBuffer);

        for(int j=0; j<colCnt; j++)
        {
          //printf("[%d]: ------> output computing\n", core_id);

          v4s w = *(v4s *) pW;
          pW += 4;
          v4u x = *(v4u *) pBuffer;
          pBuffer += 4;
          // asm volatile("": : :"memory");
          sum  = SumDotp(x, w, sum);
          // if ( i_out_x ==dim_im_out_x-1 && i_out_ch==0){
          //   printf("I'm in position %d\n",l);
          // printf("X: ------------> %d, %d, %d, %d, core_id %d\n", x[0],x[1],x[2],x[3], core_id);
          // printf("Weights: ------------> %d, %d, %d, %d, core_id %d\n", w[0],w[1],w[2],w[3], core_id);}
        }
        for(int j=0; j<leftCnt; j++)
        {
          //printf("[%d]: --------> output computing with leftover\n", core_id);

          int8_t w = *(int8_t *) pW++;
          uint8_t x = *(uint8_t *) pBuffer++;
          // asm volatile("": : :"memory");
          sum += x * w;
          // if ( i_out_x ==dim_im_out_x-1  && i_out_ch==0){
          // printf("X: ------------> %d, core_id %d\n", x, core_id);
          // printf("Weights: ------------> %d, core_id %d\n", w, core_id);}
        }

        //printf("[%d]: ----------> sum=%d\n", core_id, sum);

        if (FLAG_BATCH_NORM && FLAG_RELU)
        {
          //printf("--> bn + quant\n");
          *pOut = pulp_nn_bn_quant_u8(sum, *k1, *lambda1, out_mult, out_shift);
          // if ( i_out_x ==dim_im_out_x-1  && i_out_ch==0)
          // printf("[%d]: ------------> add=%X, pout=%d, sum=%d, core_id %d\n", core_id, pOut, *pOut, sum, core_id);
        }
        else if (FLAG_RELU)
        {
          //printf("--> act quant\n");
          *pOut = pulp_nn_quant_u8(sum, out_mult, out_shift);
        }
        else
        {
          //printf("--> round + shift\n");
          *pOut = (int8_t) clip8((sum + NN_ROUND(out_shift)) >> out_shift);
        }
        // *pOut = (int8_t) clip8((sum + NN_ROUND(out_shift)) >> out_shift);
        //*pBuffAcc+=*pOut;
        //printf("[%d]: ------------> add=%X, pout=%d\n", core_id, pOut, *pOut);
        pOut+=(dim_im_out_x * ch_im_out);
      }
    }
    k1++;
    lambda1++;
  }
  rt_team_barrier();
}
