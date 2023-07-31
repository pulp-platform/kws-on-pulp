/*
 * layer_template.c
 * Alessio Burrello <alessio.burrello@unibo.it>
 * Francesco Conti <f.conti@unibo.it>
 *
 * Copyright (C) 2018-2020 University of Bologna
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License. 
 */

#include "BNReluConvolution7.h"
#include "pmsis.h"
#include "dory.h"
#include "thorir_dma.h"
#include "pulp_nn_kernels.h"


void BNReluConvolution7(
  void *args
) {
  //////////////////////////////////////////////////////////////////////////
  // arguments assigning: keeping same interface between L2 and L3 memory //
  //////////////////////////////////////////////////////////////////////////
  unsigned int *real_arg = (unsigned int *) args;
  unsigned int l3_x =(unsigned int)  real_arg[0];
  unsigned int l3_y =(unsigned int)  real_arg[1];
  unsigned int l3_W =(unsigned int)  real_arg[2];
  unsigned int l2_x =(unsigned int)  real_arg[3];
  unsigned int l2_x_2 =(unsigned int)  real_arg[4];
  unsigned int l2_y =(unsigned int)  real_arg[5];
  unsigned int l2_W =(unsigned int)  real_arg[6];
  unsigned int l1_buffer =(unsigned int)  real_arg[7];
  unsigned int hyperram =(unsigned int)  real_arg[8];
  unsigned int out_mult_in =(unsigned int)  real_arg[9];
  unsigned int out_shift_in = (unsigned int) real_arg[10];

  /////////////////////
  // DMA declaration //
  /////////////////////
  volatile DMA_copy DMA_copy_k, DMA_copy_lambda;
  volatile DMA_copy DMA_copy_W, DMA_copy_x, DMA_copy_y;
  DMA_copy_k.hwc_to_chw = 0;
  DMA_copy_k.stride_2d = 0;
  DMA_copy_k.stride_1d = 0;
  DMA_copy_k.number_of_2d_copies = 1;
  DMA_copy_k.number_of_1d_copies = 1;
  DMA_copy_k.dir = 1;

  DMA_copy_lambda.hwc_to_chw = 0;
  DMA_copy_lambda.stride_2d = 0;
  DMA_copy_lambda.stride_1d = 0;
  DMA_copy_lambda.number_of_2d_copies = 1;
  DMA_copy_lambda.number_of_1d_copies = 1;
  DMA_copy_lambda.dir = 1;
  
  DMA_copy_x.hwc_to_chw = 1;
  DMA_copy_x.stride_2d = 320;
  DMA_copy_x.stride_1d = 64;
  DMA_copy_x.dir = 1;
  
  DMA_copy_W.hwc_to_chw = 0;
  DMA_copy_W.stride_2d = 9;
  DMA_copy_W.stride_1d = 1;
  DMA_copy_W.number_of_2d_copies = 1;
  DMA_copy_W.number_of_1d_copies = 1;
  DMA_copy_W.dir = 1;
  
  DMA_copy_y.hwc_to_chw = 0;
  DMA_copy_y.stride_2d = 320;
  DMA_copy_y.stride_1d = 64;
  DMA_copy_y.dir = 0;

  volatile int p_r, p_l, p_t, p_b;
  volatile  unsigned short x_tile_size_nif;
  volatile unsigned short  x_tile_size_h;
  volatile unsigned short  x_tile_size_w;
  volatile unsigned short  x_tile_size_byte;
  volatile unsigned short  x_length_nif_byte;
  volatile int pad_offset_h, pad_offset_w;
  volatile unsigned short  W_tile_size_nof;
  volatile unsigned short  W_tile_size_nif;
  volatile unsigned short  W_tile_size_byte;
  volatile unsigned short W_length_nif_byte;
  volatile uint8_t *x, *W, *y, *b;
  volatile int64_t *k;
  volatile int64_t *lambda;
  volatile int y_tile_size_nof;
  volatile int y_tile_size_h;
  volatile int y_tile_size_w;
  volatile int y_tile_size_byte;
  volatile int y_length_nof_byte;
  // last-tile flags
  int iter;
  // tile loop indeces
  int _i_nof_load=0, _i_nif_load=0, _i_h_load=0, _i_w_load=0;
  int _i_nof_exec=1, _i_nif_exec=1, _i_h_exec=1, _i_w_exec=1;
  volatile uint8_t *im2col;
  im2col = l1_buffer + 17640;
  volatile uint8_t *pwt_buffer;
  pwt_buffer = im2col + 672;
  uint16_t out_mult = out_mult_in;
  uint16_t out_shift = out_shift_in;

  ////////////////////////////
  // First tile transfering //
  ////////////////////////////
  pi_cl_team_barrier(0);

  int total_tiles = 1;
  // tile loop nest
  for(iter=0; iter < total_tiles; iter++) {
    // check if last in any dimension
      x_tile_size_nif = (_i_nif_load+1 == 1) ? 64 : 64;
      x_tile_size_h   = (_i_h_load+1 == 1)   ? 25 : 25;
      x_tile_size_w   = (_i_w_load+1 == 1)   ? 5 : 5;
      x_tile_size_byte = x_tile_size_nif*x_tile_size_h*x_tile_size_w*8/8;
      x_length_nif_byte = (_i_nif_load+1 == 1)   ? 64 : 64;
      // additionally overlap by padding for the first tile after a border one
      //this because in the first tile we use less pixels from x_buffer, since we have the ones of padding
      pad_offset_h=0, pad_offset_w=0;
      if(_i_h_load > 0)
        pad_offset_h = 1;
      if(_i_w_load > 0)
        pad_offset_w = 1;
      y_tile_size_h   = (_i_h_load+1 == 1)   ? 25 : 25;
      y_tile_size_w   = (_i_w_load+1 == 1)   ? 5 : 5;
      y_tile_size_nof = (_i_nof_load+1 == 1) ? 64 : 64;
      y_tile_size_byte = y_tile_size_nof*y_tile_size_h*y_tile_size_w*8/8;
      y_length_nof_byte = (_i_nof_load+1 == 1)   ? 64 : 64;
      W_tile_size_nof = (_i_nof_load+1 == 1) ? 64 : 64;
      W_tile_size_nif = (_i_nif_load+1 == 1) ? 1 : 1;
      W_tile_size_byte = W_tile_size_nof*W_tile_size_nif*3*3;
      W_length_nif_byte = (_i_nif_load+1 == 1) ? 1 : 1;
      // transfer of next input tile in double buffering
      if (_i_nif_load!=_i_nif_exec || _i_w_load!=_i_w_exec || _i_h_load!=_i_h_exec)
      {
        DMA_copy_x.ext = dory_get_tile_3d(l2_x, _i_h_load, _i_w_load, _i_nif_load, 25, 5, 64, 5, 64,  2, 2,0, pad_offset_h, pad_offset_w, 0, 8);
        DMA_copy_x.loc = (l1_buffer + 0);
        DMA_copy_x.number_of_2d_copies = x_tile_size_h;
        DMA_copy_x.number_of_1d_copies = x_tile_size_w;
        DMA_copy_x.length_1d_copy = x_length_nif_byte;
        thorir_dma(&DMA_copy_x);
        pi_cl_team_barrier(0);
      }
      // transfer of next weight tile if changed input or output channels
      if (_i_nif_load!=_i_nif_exec || _i_nof_load!=_i_nof_exec)
      {
        DMA_copy_W.ext = dory_get_tile_3d(l2_W, _i_nof_load, 0, 0, 64, 3*3, 1, 3*3, 1, 0,0,0,0,0,0, 8);
        DMA_copy_W.loc = (l1_buffer + 16016);
        DMA_copy_W.length_1d_copy = (int) W_tile_size_nof * 8 * 9 / 8;
        thorir_dma(&DMA_copy_W);
        pi_cl_team_barrier(0);

        DMA_copy_k.ext = (uint32_t) l2_W+576 + 512*_i_nof_load;
        DMA_copy_k.loc = (uint32_t) l1_buffer + 16600;
        DMA_copy_k.length_1d_copy = (uint16_t) W_tile_size_nof * 8;
        thorir_dma(&DMA_copy_k);
        pi_cl_team_barrier(0);

        DMA_copy_lambda.ext = (uint32_t) l2_W+1088 + 512*_i_nof_load;
        DMA_copy_lambda.loc = (uint32_t) l1_buffer + 17120;
        DMA_copy_lambda.length_1d_copy = (uint16_t) W_tile_size_nof * 8;
        thorir_dma(&DMA_copy_lambda);
        pi_cl_team_barrier(0);
      }
    // creation of the pointers to input, output, weights, lambda and k
    asm volatile("": : :"memory");
    x = (uint8_t *) (l1_buffer + 0);
    k = (int64_t *) (l1_buffer + 16600);
    lambda = (int64_t *) (l1_buffer + 17120);
    W = (uint8_t *) (l1_buffer + 16016);
    y = (uint8_t *) (l1_buffer + 8008);
    p_r = 0;
    p_l = 0;
    p_t = 0;
    p_b = 0;
    if (_i_h_load == 0)
      p_t = 1;
    if (_i_w_load == 0)
      p_l = 1;
    if (_i_h_load == 1-1)
      p_b = 1;
    if (_i_w_load == 1-1)
      p_r = 1;
    pi_cl_team_barrier(0);
    asm volatile("": : :"memory");
    pulp_nn_depthwise_generic(
      x, im2col,
      NULL,
      y, W,
      pwt_buffer,
      k, lambda,
      out_mult, out_shift,
      x_tile_size_w, x_tile_size_h, x_tile_size_nif,
      y_tile_size_w, y_tile_size_h, y_tile_size_nof,
      3, 3,
      p_t, p_b, p_l, p_r, 1, 1,
      1, 1
      );
    pi_cl_team_barrier(0);
      DMA_copy_y.ext = dory_get_tile_3d(l2_y, _i_h_load, _i_w_load, _i_nof_load, 25, 5, 64, 5, 64, 0, 0, 0, 0, 0, 0, 8);
      DMA_copy_y.loc = (l1_buffer + 8008);
      DMA_copy_y.number_of_2d_copies = y_tile_size_h;
      DMA_copy_y.number_of_1d_copies = y_tile_size_w;
      DMA_copy_y.length_1d_copy = y_length_nof_byte;
      thorir_dma(&DMA_copy_y); 
      pi_cl_team_barrier(0);  
    // update prev iterators

    _i_nof_exec = _i_nof_load;
    _i_nif_exec = _i_nif_load;
    _i_h_exec = _i_h_load;
    _i_w_exec = _i_w_load;
      _i_w_load += 1;
      if(_i_w_load==1) 
      {
        _i_w_load = 0;
        _i_h_load += 1;
        if(_i_h_load==1) 
        {
          _i_h_load = 0;
        _i_nif_load += 1;
          _i_nof_load += 1;
        }
      }
    pi_cl_team_barrier(0);
  }

  // wait for final write
}
