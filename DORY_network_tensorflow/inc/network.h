/*
 * network.h
 * Alessio Burrello <alessio.burrello@unibo.it>
 *
 * Copyright (C) 2019-2020 University of Bologna
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

#ifndef __NETWORK_H__
#define __NETWORK_H__

#include <stddef.h>


void network_terminate();
void network_initialize();
void network_run_cluster(void * args);
void network_run(void *l2_buffer, size_t l2_buffer_size, void *l2_final_output, void **L3_weights_curr, int exec);
void execute_layer_fork(void *arg);


#ifdef DEFINE_CONSTANTS
// allocation of buffers with parameters needed by the network execution
static const char * L3_weights_files[] = {
  "BNReluConvolution0_weights.hex", "BNReluConvolution1_weights.hex", "BNReluConvolution2_weights.hex", "BNReluConvolution3_weights.hex", "BNReluConvolution4_weights.hex", "BNReluConvolution5_weights.hex", "BNReluConvolution6_weights.hex", "BNReluConvolution7_weights.hex", "BNReluConvolution8_weights.hex", "FullyConnected10_weights.hex"
};
static int L3_weights_size[10];
static int layers_pointers[11];
static char * Layers_name[11] = {"BNReluConvolution0", "BNReluConvolution1", "BNReluConvolution2", "BNReluConvolution3", "BNReluConvolution4", "BNReluConvolution5", "BNReluConvolution6", "BNReluConvolution7", "BNReluConvolution8", "ReluPooling9", "FullyConnected10"};
static int L3_input_layers[11] = {1,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static int L3_output_layers[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static int allocate_layer[11] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1};
static int branch_input[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static int branch_output[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static int branch_change[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static int weights_checksum[11] = {403790, 153165, 595890, 160339, 603362, 153090, 605852, 151749, 587893, 0, 125264};
static int weights_size[11] = {3584, 1600, 5120, 1600, 5120, 1600, 5120, 1600, 5120, 0, 768};
static int activations_checksum[11][1] = {{
  59661  },
{
  103633  },
{
  35959  },
{
  66665  },
{
  51092  },
{
  74976  },
{
  56124  },
{
  52139  },
{
  18665  },
{
  31961  },
{
  226  }
};
static int activations_size[11] = {490, 8000, 8000, 8000, 8000, 8000, 8000, 8000, 8000, 8000, 64};
static int out_mult_vector[11] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 4125, 1};
static int out_shift_vector[11] = {23, 20, 22, 21, 22, 21, 22, 20, 22, 12, 0};
static int activations_out_checksum[11][1] = {{
  103633 },
{
  35959 },
{
  66665 },
{
  51092 },
{
  74976 },
{
  56124 },
{
  52139 },
{
  18665 },
{
  31961 },
{
  226 },
{
  10051 }
};
static int activations_out_size[11] = {8000, 8000, 8000, 8000, 8000, 8000, 8000, 8000, 8000, 64, 48};
static int layer_with_weights[11] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1};
static int NODEs_MACS[11] = {320000, 72000, 512000, 72000, 512000, 72000, 512000, 72000, 512000, 0, 768};
#endif

#endif  // __NETWORK_H__
