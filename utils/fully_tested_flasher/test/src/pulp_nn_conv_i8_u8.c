/*
 * pulp_nn_conv_i8_u8.c
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
//#include "hyperram_aligned.h"
#include "mchan_test.h"

#define log2(x) __builtin_pulp_fl1(x)
#define min(a,b) ((a)<(b)?(a):(b))
#define SumDotp(a, b, c)        __builtin_pulp_sdotusp4(a, b, c)
#define nn_round(out_shift)     (0x1 << (out_shift -1))
#define clip8(x)                __builtin_pulp_clipu_r(x, 255)
#define max4(a,b)  		    __builtin_pulp_max4(a,b)

void __attribute__ ((noinline)) pulp_nn_conv_i8_u8(
  const uint8_t * pInBuffer,
  int32_t *       pInBufferAcc,
  const uint16_t  dim_in_x,
  const uint16_t  dim_in_y,
  const uint16_t  ch_in,
  const int8_t *  pWeight,
  uint8_t         weight_displacement,
  const uint16_t  ch_out,
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
  const uint16_t  out_shift,
  const uint16_t  out_mult,
  uint8_t *       pOutBuffer,
  int32_t *       pOutBufferAcc,
  const uint16_t  dim_out_x,
  const uint16_t  dim_out_y,
  int16_t *       k,
  int32_t *       lambda,
  uint8_t *       pIm2ColBuffer,
  int8_t *        pReserved, // ignored
  int             flag_relu,
  int             flag_batch_norm,
  int             flag_acc_buff_in,
  int             flag_acc_buff_out,
  int             flag_first_ch_in,
  int             flag_first_ch_out
) {
  int core_id = rt_core_id();
  uint8_t * pIm2ColBase = pIm2ColBuffer + (2*core_id*ch_in*dim_kernel_x*dim_kernel_y);

  // local vars
  int16_t i_out_y, i_out_x, i_ker_y, i_ker_x;
  int Log2Core = log2(NUM_CORES);

  /*chunks are built along the spatial dimension of the OFM */
  int chunk = (dim_out_y >> Log2Core) + ((dim_out_y & (NUM_CORES-1))!=0);

  /* defining the specific pixels computed by each core */
  int start_pixel, stop_pixel;
  start_pixel = min(chunk *  core_id, dim_out_y);
  stop_pixel = min(start_pixel+chunk, dim_out_y);

  uint8_t *pIm2Col = pIm2ColBase;
  uint8_t *pOut    = pOutBuffer + start_pixel * ch_out * dim_out_x;
  int32_t *pInAcc  = pInBufferAcc + start_pixel * dim_out_x;

  for (i_out_y = start_pixel; i_out_y < stop_pixel; i_out_y++)
  {
    for (i_out_x = 0; i_out_x < dim_out_x; i_out_x++)
    {
      volatile unsigned int id = mchan_alloc();
      if(i_out_y < padding_y_top)
      {
        /* This part implements the im2col function */
        for (i_ker_y = i_out_y * stride_y - padding_y_top; i_ker_y < i_out_y * stride_y - padding_y_top + dim_kernel_y;i_ker_y++)
        {
          for (i_ker_x = i_out_x * stride_x - padding_x_left; i_ker_x < i_out_x * stride_x - padding_x_left + dim_kernel_x;i_ker_x++)
          {
            if (i_ker_y < 0 || i_ker_y >= dim_in_y || i_ker_x < 0 || i_ker_x >= dim_in_x)
            {
              pulp_zero_mem(pIm2Col, ch_in);
            }
            else
            {
              pulp_nn_im2col_int8((uint8_t *) pInBuffer + (i_ker_y * dim_in_x + i_ker_x) * ch_in,pIm2Col, ch_in);
            }
            pIm2Col += ch_in;
          }
        }
      }
      else if(i_out_y < dim_out_y - padding_y_bottom)
      {
        if(i_out_x < padding_x_left)
        {
          for (i_ker_y = i_out_y * stride_y - padding_y_top; i_ker_y < i_out_y * stride_y - padding_y_top + dim_kernel_y; i_ker_y++)
          {
            for (i_ker_x = i_out_x * stride_x - padding_x_left; i_ker_x < i_out_x * stride_x - padding_x_left + dim_kernel_x; i_ker_x++)
            {
              if (i_ker_x < 0 || i_ker_x >= dim_in_x)
              {
                pulp_zero_mem(pIm2Col, ch_in);
              }
              else
              {
                pulp_nn_im2col_int8((uint8_t *) pInBuffer + (i_ker_y * dim_in_x + i_ker_x) * ch_in, pIm2Col, ch_in);
              }
              pIm2Col += ch_in;
            }
          }
        }
        else if(i_out_x < dim_out_x - padding_x_right)
        {
          for (i_ker_y = i_out_y * stride_y - padding_y_top; i_ker_y < i_out_y * stride_y - padding_y_top + dim_kernel_y; i_ker_y++)
          {
            pulp_nn_im2col_int8((uint8_t *) pInBuffer + (i_ker_y * dim_in_x + i_out_x * stride_x - padding_x_left) * ch_in, pIm2Col, ch_in * dim_kernel_x);
            pIm2Col += ch_in * dim_kernel_x;
          }
        }
        else
        {
          /* This part implements the im2col function */
          for (i_ker_y = i_out_y * stride_y - padding_y_top; i_ker_y < i_out_y * stride_y - padding_y_top + dim_kernel_y; i_ker_y++)
          {
            for (i_ker_x = i_out_x * stride_x - padding_x_left; i_ker_x < i_out_x * stride_x - padding_x_left + dim_kernel_x; i_ker_x++)
              {
                if (i_ker_x < 0 || i_ker_x >= dim_in_x)
                {
                  pulp_zero_mem(pIm2Col, ch_in);
                }
                else
                {
                  pulp_nn_im2col_int8((uint8_t *) pInBuffer + (i_ker_y * dim_in_x + i_ker_x) * ch_in,pIm2Col, ch_in);
                }
                pIm2Col += ch_in;
              }
          }
        }
      }
      else
      {
        for (i_ker_y = i_out_y * stride_y - padding_y_top; i_ker_y < i_out_y * stride_y - padding_y_top + dim_kernel_y; i_ker_y++)
        {
          for (i_ker_x = i_out_x * stride_x - padding_x_left; i_ker_x < i_out_x * stride_x - padding_x_left + dim_kernel_x;i_ker_x++)
          {
            if (i_ker_y < 0 || i_ker_y >= dim_in_y || i_ker_x < 0 || i_ker_x >= dim_in_x)
            {
              pulp_zero_mem(pIm2Col, ch_in);
            }
            else
            {
              pulp_nn_im2col_int8((uint8_t *) pInBuffer + (i_ker_y * dim_in_x + i_ker_x) * ch_in, pIm2Col, ch_in);
            }
            pIm2Col += ch_in;
          }
        }
      }
      mchan_barrier(id);
      mchan_free(id);
      if (pIm2Col == pIm2ColBase + 2 * ch_in * dim_kernel_x * dim_kernel_y)
      {
        pOut = pulp_nn_matmul_4x2_i8_u8(
          pWeight,
          pIm2ColBase,
          pInAcc,
          ch_out,
          ch_in * dim_kernel_x * dim_kernel_y,
          bias_shift,
          out_shift,
          out_mult,
          k,
          lambda,
          bias,
          pOut,
          pOutBufferAcc,
          flag_relu,
          flag_batch_norm,
          flag_acc_buff_in,
          flag_acc_buff_out,
          flag_first_ch_in,
          flag_first_ch_out,
          weight_displacement * dim_kernel_x * dim_kernel_y
        );
        if(flag_acc_buff_in)
        {
          /* increments accumulation buffers by 2 because at the same time they are computed two channels in matrix multiplication kernel */
          pInBufferAcc+=2; // FIXME --> This is probably wrong!!!
        }
        if(flag_acc_buff_out)
        {
          pOutBufferAcc += 2;
        }
        pIm2Col = pIm2ColBase;

      }
    }
  }

  /* check if there is left-over for compute */
  if (pIm2Col != pIm2ColBase)
  {
    const int8_t *pA = pWeight;
    int       i;
    for (i = 0; i < ch_out; i++)
    {
      /* include the accumulation buffer in sum computation (probably doesn't work). Maybe the reloading partial result is needed as well as internally at mat mul function. */
      int sum = 0;

      if(flag_first_ch_in == 0)
      {
        sum = 0;
      }
      else if (bias != NULL)
      {
        sum = ((int)(bias[i]) << bias_shift) + nn_round(out_shift);
      }

      if(flag_acc_buff_in == 1)
      {
        /* Maybe the displacement factor is necessary. */
        sum += (*pInBufferAcc);
      }
      uint8_t *pB = pIm2ColBase;
      /* basically each time it process 4 entries */
      uint16_t  col_cnt_im2col = ch_in * dim_kernel_x * dim_kernel_y >> 2;

      for (int j=0 ; j < col_cnt_im2col; j++)
      {
        v4s inA = *((v4s*) pA);
        v4u inB = *((v4u*) pB);

        sum = SumDotp(inB, inA, sum);
        pA+=4;
        pB+=4;
      }
      col_cnt_im2col = (ch_in * dim_kernel_y * dim_kernel_x) & 0x3;
      while (col_cnt_im2col)
      {
        int8_t      inA1 = *pA++;
        uint8_t     inB1 = *pB++;
        asm volatile("": : :"memory");
        sum += inA1 * inB1;

        col_cnt_im2col--;
      }
      if (flag_acc_buff_out == 1)
      {
        if (flag_first_ch_out)
        {
          *pOutBufferAcc  = sum;
        }
        else
        {
          *pOutBufferAcc += sum;
        }
      }
      /* if activation layer follows batch normalization */
      if (flag_batch_norm && flag_relu)
      {
        *pOut = pulp_nn_bn_quant_u8(sum, *k, *lambda, out_mult, out_shift);
        k++;
        lambda++;
        *pOut++;
      }
      else
      {
        /* if there isn't batch normalization but there is activation layer */
        if(flag_relu == 1)
        {
          *pOut = pulp_nn_quant_u8(sum, out_mult, out_shift);
        }
        else
        {
          *pOut = (uint8_t) clip8(sum >> out_shift);
        }
        pOut++;
      }
    }
  }
  rt_team_barrier();
}
