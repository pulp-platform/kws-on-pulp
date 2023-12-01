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
  "BNReluConvolution0_weights.hex", "BNReluConvolution1_weights.hex", "BNReluConvolution2_weights.hex", "BNReluConvolution3_weights.hex", "BNReluConvolution4_weights.hex", "BNReluConvolution5_weights.hex", "BNReluConvolution6_weights.hex", "BNReluConvolution7_weights.hex", "BNReluConvolution8_weights.hex", "BNReluConvolution9_weights.hex", "BNReluConvolution10_weights.hex", "FullyConnected12_weights.hex"
};
static int L3_weights_size[12];
static int layers_pointers[13];
static char * Layers_name[13] = {"BNReluConvolution0", "BNReluConvolution1", "BNReluConvolution2", "BNReluConvolution3", "BNReluConvolution4", "BNReluConvolution5", "BNReluConvolution6", "BNReluConvolution7", "BNReluConvolution8", "BNReluConvolution9", "BNReluConvolution10", "ReluPooling11", "FullyConnected12"};
static int L3_input_layers[13] = {1,
0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static int L3_output_layers[13] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static int allocate_layer[13] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1};
static int branch_input[13] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static int branch_output[13] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static int branch_change[13] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
static int weights_checksum[13] = {1625532, 496377, 9791968, 523679, 9862658, 513183, 9877212, 525881, 9911315, 515558, 9961445, 0, 476585};
static int weights_size[13] = {13248, 4692, 78384, 4692, 78384, 4692, 78384, 4692, 78384, 4692, 78384, 0, 3360};
static int activations_checksum[13][1] = {{
  60507  },
{
  523199  },
{
  213447  },
{
  523701  },
{
  284722  },
{
  463465  },
{
  260130  },
{
  351876  },
{
  191259  },
{
  327539  },
{
  195784  },
{
  334184  },
{
  2570  }
};
static int activations_size[13] = {490, 34500, 34500, 34500, 34500, 34500, 34500, 34500, 34500, 34500, 34500, 34500, 276};
static int out_mult_vector[13] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 4125, 1};
static int out_shift_vector[13] = {22, 20, 23, 21, 23, 21, 23, 22, 23, 21, 24, 12, 0};
static int activations_out_checksum[13][1] = {{
  523199 },
{
  213447 },
{
  523701 },
{
  284722 },
{
  463465 },
{
  260130 },
{
  351876 },
{
  191259 },
{
  327539 },
{
  195784 },
{
  334184 },
{
  2570 },
{
  6822 }
};
static int activations_out_size[13] = {34500, 34500, 34500, 34500, 34500, 34500, 34500, 34500, 34500, 34500, 34500, 276, 48};
static int layer_with_weights[13] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1};
static int NODEs_MACS[13] = {1380000, 310500, 9522000, 310500, 9522000, 310500, 9522000, 310500, 9522000, 310500, 9522000, 0, 3312};
#endif

#endif  // __NETWORK_H__
