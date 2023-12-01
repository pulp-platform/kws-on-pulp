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
// ODDA
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
static int weights_checksum[11] = {1008695, 322340, 3871488, 334747, 3880349, 329046, 3935714, 316588, 3785005, 0, 295904};
static int weights_size[11] = {8256, 2924, 30960, 2924, 30960, 2924, 30960, 2924, 30960, 0, 2112};
static int activations_checksum[11][1] = {{
  60507  },
{
  283968  },
{
  137106  },
{
  331095  },
{
  164112  },
{
  270284  },
{
  146446  },
{
  150811  },
{
  95583  },
{
  128114  },
{
  952  }
};
static int activations_size[11] = {490, 21500, 21500, 21500, 21500, 21500, 21500, 21500, 21500, 21500, 172};
static int out_mult_vector[11] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 4125, 1};
static int out_shift_vector[11] = {22, 20, 22, 22, 23, 21, 23, 21, 23, 12, 0};
static int activations_out_checksum[11][1] = {{
  283968 },
{
  137106 },
{
  331095 },
{
  164112 },
{
  270284 },
{
  146446 },
{
  150811 },
{
  95583 },
{
  128114 },
{
  952 },
{
  7794 }
};
static int activations_out_size[11] = {21500, 21500, 21500, 21500, 21500, 21500, 21500, 21500, 21500, 172, 48};
static int layer_with_weights[11] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1};
static int NODEs_MACS[11] = {860000, 193500, 3698000, 193500, 3698000, 193500, 3698000, 193500, 3698000, 0, 2064};
#endif

#endif  // __NETWORK_H__
