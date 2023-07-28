/*
 * kernels.h
 * Angelo Garofalo <angelo.garofalo@unibo.it>
 * Nazareno Bruschi <nazareno.bruschi@studio.unibo.it>
 * Francesco Conti <f.conti@unibo.it>
 * 
 * Copyright (C) 2019 ETH Zurich, University of Bologna.
 * All rights reserved.
 */
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
);

void pulp_nn_avgpool_u8 (
  uint8_t *  Im_in,
  uint16_t  dim_im_in_x,
  uint16_t  dim_im_in_y,
  uint16_t  ch_im_in,
  uint16_t  dim_kernel_x,
  uint16_t  dim_kernel_y,
  uint16_t  padding,
  uint16_t  stride,
  uint16_t  dim_im_out_x,
  uint16_t  dim_im_out_y,
  int8_t *  bufferA,
  uint8_t *  Im_out,
  int32_t * pOutBufferAcc,
  int8_t    flag_acc_buff_out,
  int8_t    flag_first_ch_out,
  int       flag_relu,
  const uint16_t  out_shift,
  const uint16_t  out_mult
);

uint8_t pulp_nn_bn_quant_u8 (
  int32_t phi,
  int16_t k,
  int32_t lambda,
  int16_t m,
  int8_t  d
);

void pulp_nn_conv_i8_u8(
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
);


void pulp_nn_conv_i8_u8_first(
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
);

void pulp_nn_conv_pool_i8_u8 (
  const int8_t *  Im_in_out,
  int32_t *       pInBufferAcc,
  const uint16_t  dim_im_in_x,
  const uint16_t  dim_im_in_y,
  const uint16_t  ch_im_in,
  const int8_t *  wt,
  uint8_t         weight_displacement,
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
  const uint16_t  out_shift,
  const uint16_t  out_mult,
  int8_t *        temp,
  const int32_t * pOutBufferAcc,
  const uint16_t  dim_im_out_x,
  const uint16_t  dim_im_out_y,
  int16_t *       k,
  int32_t *       lambda,
  int8_t *        bufferC,
  int8_t *        bufferB,
  int             FLAG_RELU,
  int             FLAG_BATCH_NORM,
  int             FLAG_ACC_BUFF_IN,
  int             FLAG_ACC_BUFF_OUT,
  int             flag_first_ch_in,
  int             flag_first_ch_out,
  int             FLAG_PAVG,
  const uint16_t  dim_kernel_pooling,
  const uint16_t  padding_pooling,
  const uint16_t  stride_pooling,
  const uint16_t  dim_im_out_x_pooling,
  const uint16_t  dim_im_out_y_pooling
);

void pulp_nn_linear_i8_u8 (
	uint8_t * pIn,
	int32_t * pInBufferAcc,
	int8_t *  pWeights,
	uint16_t  dim_vec,
	uint16_t  num_o_neurons,
	uint16_t  bias_shift,
	uint16_t  out_shift,
	int8_t *  bias,
	uint8_t * pOut,
	int32_t * pOutBufferAcc,
	uint8_t   displacement,
	int       flag_clip,
	int       flag_displacement
);

uint8_t * pulp_nn_matmul_4x2_i8_u8(
  const int8_t *  pWeight,
  uint8_t *       pInBuffer,
  int32_t *       pInBufferAcc,
  uint16_t        ch_out,
  uint16_t        num_col_im2col,
  uint16_t        bias_shift,
  uint16_t        out_shift,
  uint16_t        out_mult,
  int16_t *       k,
  int32_t *       lambda,
  const int8_t *  bias,
  uint8_t *       pOut,
  int32_t *       pOutBufferAcc,
  int             flag_relu,
  int             flag_batch_norm,
  int             flag_acc_buff_in,
  int             flag_acc_buff_out,
  int             flag_first_ch_in,
  int             flag_first_ch_out,
  int8_t          displacement
);

void pulp_nn_maxpool_u8 (
  uint8_t * Im_in,             // pointer to the input feature map
  uint16_t  dim_im_in_x,       // spatial dimension of the input feature map
  uint16_t  dim_im_in_y,
  uint16_t  ch_im_in,          // number of channels of the IFM
  uint16_t  dim_kernel,        // spatial dimension of the pooling filter
  uint16_t  padding_t,           // amount of padding
  uint16_t  padding_b,           // amount of padding
  uint16_t  padding_l,           // amount of padding
  uint16_t  padding_r,           // amount of padding
  uint16_t  stride,            // amount of stride
  uint16_t  dim_im_out_x,      // reduced spatial dimension of output
  uint16_t  dim_im_out_y,
  int8_t *  bufferA,           // actually not used in this fx
  uint8_t * Im_out,            // pointer to the output
  int32_t * pOutBufferAcc,
  int8_t    flag_acc_buff_out,
  int8_t    flag_first_ch_out
);

uint8_t pulp_nn_quant_u8 (
  int32_t phi,
  int16_t m,
  int8_t  d
);

void pulp_nn_relu_u8 (
  int8_t * data,
  uint16_t dim_im_in_x,
  uint16_t dim_im_in_y,
  uint16_t ch_im_in
);

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
);

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
);

