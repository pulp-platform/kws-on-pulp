/*
 * Copyright (C) 2021-2022 ETH Zurich and University of Bologna
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

/**
 * Authors: Davide Nadalini, Leonardo Ravaglia
*/ 

#include "math.h"
#include "pulp_train_utils_fp32.h"
#include "pulp_losses_fp32.h"

static void localsoftmax(float *input, size_t input_len) {

//   // find min
//   float min = input[0];
//   for (size_t i = 0; i < input_len; i++){
//     if (input[i] < min){
//       min = input[i];
//     }
//     // printf("Input[%d] is %f\n", i, input[i]);
//   }

//   // for (size_t i = 0; i < input_len; i++){
//   //   input[i] = input[i] - min + 1e-6;
//   //   printf("Input[%d] is %f\n", i, input[i]);
//   // }


//   for (size_t i = 0; i < input_len; i++){
//     input[i] = input[i] + 1e-6;
//     printf("Input[%d] is %f\n", i, input[i]);
//   }


//   // float* output = pi_l1_malloc(input_len * sizeof(float));
//   float output[input_len];

//   struct softmax_args *args;
//   args->input = input;
//   args->output = output;
//   args->dim = input_len;

//   exponential(args);



//   for (size_t i = 0; i < input_len; i++) {

//     output[i] = exp(input[i]);
// #ifdef VERBOSE
//     printf("Exponential[%d] is %f\n", i, output[i]);
// #endif
//   }

//   float sum = 0.0;
//   for (size_t i = 0; i < input_len; i++) {
//     sum += output[i];
//   }

//   float offset = logf(sum);
//   printf("Sum is: %f, offset is: %f\n", sum, offset);

//   for (size_t i = 0; i < input_len; i++) {
//     output[i] = output[i]/sum;
//   }

// #ifdef VERBOSE
//   for (size_t i = 0; i < input_len; i++){
//     printf("Input[%d] is %f\n", i, input[i]);
//   }
// #endif

//   for (size_t i = 0; i < input_len; i++){
//     printf("Softmax[%d] is %f\n", i, output[i]);
//     input[i] = output[i];
//   }

}



void pulp_CrossEntropyLoss ( void * loss_args )
{
  struct loss_args * args = (struct loss_args *) loss_args;
  float * outData = args->output->data;
  float * outDiff = args->output->diff;
  float * target = args->target;
  float * wr_loss = args->wr_loss;
  int size = args->output->dim;

  float loss = 0.0;

  localsoftmax(outData, 12);

  for (int idx = 0; idx < 12; idx++){
    printf("Out[%i]=%f\n", idx, ((float *)outData)[idx]);
  }


  float delta = 0.000001;
  for(int i=0; i<size; i++){
    loss += -target[i]*logf(outData[i] + delta);
  }
  
    #ifdef DEBUG
      printf("target: %f, out_diff: %f, out_data:%f\n", target[i], outDiff[i], outData[i]);
      printf("loss:%f \n",loss);
    #endif

  printf("Loss is %f\n", loss);

  // Skip printf profiling in debug mode
  #ifdef DEBUG
  #ifdef PROF_NET
  pi_perf_stop();
  #endif
  // printf("\nLoss: %+.4f\n", loss);  
  #ifdef PROF_NET
  pi_perf_start();
  #endif
  #endif  

  *wr_loss = loss;

  for(int i=0; i<size; i++){
    outDiff[i] = (-target[i]+outData[i]);
    
    #ifdef DEBUG
    printf("target: %+.4f, out_diff: %+.4f, out_data:%+.4f\n", target[i], outDiff[i], outData[i]);
    #endif
  }
}


void pulp_MSELoss ( void * loss_args ) 
{
  struct loss_args * args = (struct loss_args *) loss_args;
  float * outData = args->output->data;
  float * outDiff = args->output->diff;
  float * target = args->target;
  float * wr_loss = args->wr_loss;
  int size = args->output->dim;
  int off = 0;

  float loss = 0.0f;
  float meanval = 1.0f / size;
  
  #ifdef DEBUG
  printf("loss meanval is: %f\n", meanval);
  #endif
  
  for(int i=0; i<size; i++){
    loss += meanval * (target[i] - outData[i]) * (target[i] - outData[i]);

    #ifdef DEBUG
    printf("target: %f, out_diff: %f, out_data:%f\n", target[i], outDiff[i], outData[i]);
    printf("loss:%f \n",loss);
    #endif
  }

  // Skip printf profiling in debug mode
  #ifdef DEBUG
  #ifdef PROF_NET
  pi_perf_stop();
  #endif
  printf("\nLoss: %+.4f\n", loss);
  #ifdef PROF_NET
  pi_perf_start();
  #endif
  #endif  

  *wr_loss = loss;

  for(int i=0; i<size; i++){
    outDiff[i] = meanval * 2.0f *(outData[i] - target[i]);

    #ifdef DEBUG
    printf("target: %+.4f, out_diff: %+.4f, out_data:%+.4f\n", target[i], outDiff[i], outData[i]);
    #endif
  }

}
