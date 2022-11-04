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
#define pack(x,y,z,t) __builtin_pulp_pack4(x,y,z,t)

void pulp_nn_dw_conv_i8_u8 (
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

  /* parallelization */
  int core_id = rt_core_id();
  uint8_t * bufferA = bufferC  + (2*core_id*ch_im_in*dim_kernel_y*dim_kernel_x);

  // local vars
  int16_t i_out_y, i_out_x, i_ker_y, i_ker_x;
  int Log2Core = log2(NUM_CORES);

  /*chunks are built along the spatial dimension of the OFM */
  int chunk = (dim_im_out_y >> Log2Core) + ((dim_im_out_y & (NUM_CORES-1))!=0);

  /* defining the specific pixels computed by each core */
  int start_pixel, stop_pixel;
  start_pixel = MIN(chunk * core_id, dim_im_out_y);
  stop_pixel = MIN(start_pixel+chunk, dim_im_out_y);

  uint8_t *colBuffer = bufferA;
  uint8_t *pBuffer = colBuffer;
  uint8_t *pOut = Im_out + start_pixel * ch_im_out * dim_im_out_x;

  int32_t *pBuffAcc = pOutBufferAcc + start_pixel * dim_im_out_x;

  const int8_t *pBias = bias;
  uint16_t rowCnt;
  uint16_t row_shift;

  /* check if it is a depthwise */
  if (ch_im_in != ch_im_out)
  {
    return -1;
  }

  /* start kernel: this first phase is devoted to building the im2col buffers */
  for (i_out_y = start_pixel; i_out_y < stop_pixel; i_out_y++)
  {
    for (i_out_x = 0; i_out_x < dim_im_out_x; i_out_x++)
    {
      /* image-like to columns transform*/
      if(i_out_y < padding_y_top)
      {
        /* This part implements the im2col function */
        for (i_ker_y = i_out_y * stride_y - padding_y_top; i_ker_y < i_out_y * stride_y - padding_y_top + dim_kernel_y;i_ker_y++)
        {
          for (i_ker_x = i_out_x * stride_x - padding_x_left; i_ker_x < i_out_x * stride_x - padding_x_left + dim_kernel_x;i_ker_x++)
          {
            if (i_ker_y < 0 || i_ker_y >= dim_im_in_y || i_ker_x < 0 || i_ker_x >= dim_im_in_x)
            {
              pulp_zero_mem(pBuffer, ch_im_in);
            }
            else
            {
              pulp_nn_im2col_int8_dmafree((uint8_t *) Im_in + (i_ker_y * dim_im_in_x + i_ker_x) * ch_im_in,pBuffer, ch_im_in);
            }
            pBuffer += ch_im_in;
          }
        }
      }
      else if(i_out_y < dim_im_out_y - padding_y_bottom)
      {
        if(i_out_x < padding_x_left)
        {
          for (i_ker_y = i_out_y * stride_y - padding_y_top; i_ker_y < i_out_y * stride_y - padding_y_top + dim_kernel_y; i_ker_y++)
          {
            for (i_ker_x = i_out_x * stride_x - padding_x_left; i_ker_x < i_out_x * stride_x - padding_x_left + dim_kernel_x; i_ker_x++)
            {
              if (i_ker_x < 0 || i_ker_x >= dim_im_in_x)
              {
                pulp_zero_mem(pBuffer, ch_im_in);
              }
              else
              {
                pulp_nn_im2col_int8_dmafree((uint8_t *) Im_in + (i_ker_y * dim_im_in_x + i_ker_x) * ch_im_in, pBuffer, ch_im_in);
              }
              pBuffer += ch_im_in;
            }
          }
        }
        else if(i_out_x < dim_im_out_x - padding_x_right)
        {
          for (i_ker_y = i_out_y * stride_y - padding_y_top; i_ker_y < i_out_y * stride_y - padding_y_top + dim_kernel_y; i_ker_y++)
          {
            pulp_nn_im2col_int8_dmafree((uint8_t *) Im_in + (i_ker_y * dim_im_in_x + i_out_x * stride_x - padding_x_left) * ch_im_in,
                                                pBuffer, ch_im_in * dim_kernel_x);
            pBuffer += ch_im_in * dim_kernel_x;
          }
        }
        else
        {
          /* This part implements the im2col function */
          for (i_ker_y = i_out_y * stride_y - padding_y_top; i_ker_y < i_out_y * stride_y - padding_y_top + dim_kernel_y; i_ker_y++)
          {
            for (i_ker_x = i_out_x * stride_x - padding_x_left; i_ker_x < i_out_x * stride_x - padding_x_left + dim_kernel_x; i_ker_x++)
              {
                if (i_ker_x < 0 || i_ker_x >= dim_im_in_x)
                {
                  pulp_zero_mem(pBuffer, ch_im_in);
                }
                else
                {
                  pulp_nn_im2col_int8_dmafree((uint8_t *) Im_in + (i_ker_y * dim_im_in_x + i_ker_x) * ch_im_in,pBuffer, ch_im_in);
                }
                pBuffer += ch_im_in;
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
            if (i_ker_y < 0 || i_ker_y >= dim_im_in_y || i_ker_x < 0 || i_ker_x >= dim_im_in_x)
            {
              pulp_zero_mem(pBuffer, ch_im_in);
            }
            else
            {
              pulp_nn_im2col_int8_dmafree((uint8_t *) Im_in + (i_ker_y * dim_im_in_x + i_ker_x) * ch_im_in, pBuffer, ch_im_in);
            }
            pBuffer += ch_im_in;
          }
        }
      }

      rowCnt = ch_im_out >> 2;
      row_shift = 0;
      pBias = bias;

      while (rowCnt)
      {
        int sum  = 0;
        int sum2 = 0;
        int sum3 = 0;
        int sum4 = 0;

        /* colCnt is the iterator: take into account the 8bit SIMD insns used */
        uint16_t colCnt = (dim_kernel_y * dim_kernel_x) >> 2;

        /* fetch the IFM pixels: note that each channel is computed independently */
        const uint8_t *pB = colBuffer +  row_shift;

        /* fetch weights at right location */
        /* in the HWC format, for dw, the weights are already ordered to feed the GEMM */
        /* Co -- H -- W -- Ci but Ci == 1 --> Co -- H -- W  */
        int8_t *pA = wt + row_shift* dim_kernel_y * dim_kernel_x;
        int8_t *pA2 = pA  + dim_kernel_y* dim_kernel_x;
        int8_t *pA3 = pA2 + dim_kernel_y* dim_kernel_x;
        int8_t *pA4 = pA3 + dim_kernel_y* dim_kernel_x;
        row_shift += 4;

        /* support vectors to swap the pixels */
        /* on the fly HWC to CHW transformation (sort of) */
        uint8_t *pI1, *pI2, *pI3, *pI4;
        uint8_t left_p[4];
        uint8_t left_w[4];

        while(colCnt)
        {
          /* need a strong optimization */
          pI1 = pB;
          pB += ch_im_in;
          pI2 = pB;
          pB += ch_im_in;
          pI3 = pB;
          pB += ch_im_in;
          pI4 = pB;
          pB += ch_im_in;

          /* on the fly HWC to CHW  --> high overhead */
          v4u i1 = pack(pI1[0], pI2[0], pI3[0], pI4[0]);
          v4u i2 = pack(pI1[1], pI2[1], pI3[1], pI4[1]);
          v4u i3 = pack(pI1[2], pI2[2], pI3[2], pI4[2]);
          v4u i4 = pack(pI1[3], pI2[3], pI3[3], pI4[3]);

          v4s w1 = *((v4s*) pA);
          pA +=4;
          v4s w2 = *((v4s*) pA2);
          pA2 +=4;
          v4s w3 = *((v4s*) pA3);
          pA3 +=4;
          v4s w4 = *((v4s*) pA4);
          pA4 +=4;
          
          sum  = SumDotp(i1,w1, sum );
          sum2 = SumDotp(i2,w2, sum2);
          sum3 = SumDotp(i3,w3, sum3);
          sum4 = SumDotp(i4,w4, sum4);

          colCnt--;
        }
        colCnt = (dim_kernel_y * dim_kernel_x) & 0x3;
        while(colCnt)
        {
          /* this loop, if optimized, does a mess at compiling time */
          int16_t A = (int16_t) *(pA++);
          int16_t B = (int16_t) *(pA2++);
          int16_t C = (int16_t) *(pA3++);
          int16_t D = (int16_t) *(pA4++);

          *((v4s*)left_p) = *((v4s*) pB);

          /* dummy variable to prevent the compiler doing a mess */
          int16_t a = left_p[0];
          int16_t b = left_p[1];
          int16_t c = left_p[2];
          int16_t d = left_p[3];

          /* bad stuff but needed to prevent the compiler doing a mess */
          asm volatile("": : :"memory");
          sum  += a * A;
          sum2 += b * B;
          sum3 += c * C;
          sum4 += d * D;

          pB+=ch_im_in;
          colCnt--;
        }
        if(flag_acc_buff_out == 1)
        {
          if(flag_first_ch_out == 1)
          {
            *pBuffAcc = 0;
          }
          else
          {
            *pBuffAcc+=(sum + sum2 + sum3 + sum4);
          }
        }
        if (FLAG_BATCH_NORM && FLAG_RELU)
        {
          *pOut = pulp_nn_bn_quant_u8(sum, *k, *lambda, out_mult, out_shift);
          pOut++;
          k++;
          lambda++;

          *pOut = pulp_nn_bn_quant_u8(sum2, *k, *lambda, out_mult, out_shift);
          pOut++;
          k++;
          lambda++;

          *pOut = pulp_nn_bn_quant_u8(sum3, *k, *lambda, out_mult, out_shift);
          pOut++;
          k++;
          lambda++;

          *pOut = pulp_nn_bn_quant_u8(sum4, *k, *lambda, out_mult, out_shift);
          pOut++;
          k++;
          lambda++;
        }
        else
        {
          if (FLAG_RELU == 1)
          {
            *pOut = pulp_nn_quant_u8(sum, out_mult, out_shift);
            pOut++;
            *pOut = pulp_nn_quant_u8(sum2, out_mult, out_shift);
            pOut++;
            *pOut = pulp_nn_quant_u8(sum3, out_mult, out_shift);
            pOut++;
            *pOut = pulp_nn_quant_u8(sum4, out_mult, out_shift);
            pOut++;
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
          }
        }

        rowCnt--;
      }

      rowCnt = ch_im_out & 0x3;
      while(rowCnt)
      {
        uint8_t *pB = colBuffer + row_shift;
        const int8_t *pA = wt + row_shift*dim_kernel_y*dim_kernel_x;
        int32_t sum = 0;
        uint16_t colCnt = (dim_kernel_y * dim_kernel_x);
        row_shift += 1;

        while (colCnt)
        {
          int8_t A1 = *pA;
          uint8_t B1 = *pB;
          pA ++;
          pB += ch_im_in;
          asm volatile("": : :"memory");
          sum += A1 * B1;
          asm volatile("": : :"memory");

          colCnt--;
        }
        if(flag_acc_buff_out == 1)
        {
          if(flag_first_ch_out == 1)
          {
            *pBuffAcc = 0;
          }
          else
          {
            *pBuffAcc+=sum;
          }
        }
        if (FLAG_BATCH_NORM && FLAG_RELU)
        {
          *pOut = pulp_nn_bn_quant_u8(sum, *k, *lambda, out_mult, out_shift);
          pOut++;
          k++;
          lambda++;
        }
        else
        {
          if (FLAG_RELU == 1)
          {
            *pOut = pulp_nn_quant_u8(sum, out_mult, out_shift);
            pOut++;
          }
          else
          {
            *pOut = (uint8_t) clip8(sum >> out_shift);
            pOut++;
          }
        }
        rowCnt--;
      }

      /* clear counter and pointers */
      pBuffer = colBuffer;
      k -= ch_im_out;
      lambda -= ch_im_out;
    }
    pBuffAcc++;
  }
  rt_team_barrier();
}
