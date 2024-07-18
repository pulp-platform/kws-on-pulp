// Copyright (C) 2023-2024 ETH Zurich
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// SPDX-License-Identifier: Apache-2.0
// ==============================================================================
//
// Author: Cristian Cioflan, ETH (cioflanc@iis.ee.ethz.ch)


#include "preprocess.h"

// MFCC
#include "MfccKernels.h"
#include "DCTTwiddles.def"
#include "MelFBSparsity.def"
#include "WindowLUT.def"
#include "FFTTwiddles.def"
#include "RFFTTwiddles.def"
#include "MelFBCoeff.def"
#include "SwapTable.def"

// L2 input for offline MFCC
#include "input.h"

MFCC_IN_TYPE * MfccInputSignal;
OUT_TYPE * MfccOutputSignal;


// Initialize MFCC computation
void mfcc_kernel()
{
    #ifdef PERF
        gap_cl_starttimer();
        gap_cl_resethwtimer();
        int start = gap_cl_readhwtimer();
    #endif

    // unsigned int * args = (unsigned int *) args_mfcc;
    // MFCC_IN_TYPE * MfccInputSignal = (MFCC_IN_TYPE *) args[0];
    // OUT_TYPE * MfccOutputSignal = (OUT_TYPE *) args[1];

    // Compute MFCC following Tensorflow settings
    #if (N_DCT == 0)
        #if (DATA_TYPE==2) || (DATA_TYPE==3)
        Tensorflow_MFCC(MfccInputSignal, MfccOutputSignal, FFTTwiddles, RFFTTwiddles, SwapTable, WindowLUT, MelFBSparsity, MelFBCoeff);
        #elif (DATA_TYPE==1)
        Tensorflow_MFCC(MfccInputSignal, MfccOutputSignal, FFTTwiddles, SwapTable, WindowLUT, MelFBSparsity, MelFBCoeff, NORM);
        #else
        Tensorflow_MFCC(MfccInputSignal, MfccOutputSignal, FFTTwiddles, RFFTTwiddles, SwapTable, WindowLUT, MelFBSparsity, MelFBCoeff, NORM);
        #endif
    #else
        #if (DATA_TYPE==2) || (DATA_TYPE==3)
        Tensorflow_MFCC(MfccInputSignal, MfccOutputSignal, FFTTwiddles, RFFTTwiddles, SwapTable, WindowLUT, MelFBSparsity, MelFBCoeff, DCTTwiddles);
        #elif (DATA_TYPE==1)
        Tensorflow_MFCC(MfccInputSignal, MfccOutputSignal, FFTTwiddles, SwapTable, WindowLUT, MelFBSparsity, MelFBCoeff, NORM, DCTTwiddles);
        #else
        Tensorflow_MFCC(MfccInputSignal, MfccOutputSignal, FFTTwiddles, RFFTTwiddles, SwapTable, WindowLUT, MelFBSparsity, MelFBCoeff, NORM, DCTTwiddles);
        #endif
    #endif

    #ifdef PERF
        int elapsed = gap_cl_readhwtimer() - start;
        printf("Total Cycles: %d over %d Frames %d Cyc/Frame\n", elapsed, N_MFCC_WINS, elapsed / N_MFCC_WINS);
    #endif
}


// Set up MFCC computation
void mfcc_computation(MFCC_IN_TYPE * MfccInputSignal, OUT_TYPE * MfccOutputSignal){
   
    struct pi_cluster_task* task_mfcc;
    task_mfcc = pi_l2_malloc(sizeof(struct pi_cluster_task));
    pi_cluster_task(task_mfcc,&mfcc_kernel,NULL);
    if (task_mfcc == NULL) {
        printf("failed to allocate memory for task\n");
    }
    pi_cluster_task_stacks(task_mfcc, NULL, SLAVE_STACK_SIZE);
    

    pi_cluster_conf_init(&cl_conf);
    pi_open_from_conf(&cluster_dev, &cl_conf);
    if (pi_cluster_open(&cluster_dev))
    {
      return -1;
    }

    pi_cluster_task(task_mfcc,&mfcc_kernel,NULL);
    L1_Memory = pi_l1_malloc(&cluster_dev, _L1_Memory_SIZE);
    if (L1_Memory==NULL){
        printf("Error allocating L1\n");
        pmsis_exit(-1);
    }
    pi_cluster_send_task_to_cl(&cluster_dev, task_mfcc);
    pi_l2_free(task_mfcc, sizeof(struct pi_cluster_task));
    pi_cluster_close(&cluster_dev);
}


void preprocess(MFCC_IN_TYPE * input_buffer, uint8_t * output_buffer, int input_src){

    OUT_TYPE * MfccOutSig = NULL;

    MfccOutSig = (OUT_TYPE *) pi_l2_malloc(N_MFCC_WINS * N_MELS * sizeof(OUT_TYPE)); 
    if (MfccOutSig==NULL){
        printf("Error allocating MfccOutSig\n");
        pmsis_exit(-1);
    }

    MfccInputSignal = (MFCC_IN_TYPE *) pi_l2_malloc(16000 * sizeof(MFCC_IN_TYPE)); 
    MfccOutputSignal = (OUT_TYPE *) pi_l2_malloc(N_MFCC_WINS * N_MELS * sizeof(OUT_TYPE));

    // for (int debugi = 0; debugi < 16000; debugi++){
    //     MfccInputSignal[debugi] = 0;
    // }
    for (int debugi = 0; debugi < 16000; debugi++){
        MfccInputSignal[debugi] = input_buffer[debugi];
    }
    
    mfcc_computation(input_buffer, MfccOutputSignal);
    
    pi_l2_free(input_buffer, AUDIO_BUFFER_SIZE * sizeof(MFCC_IN_TYPE)); 

    int k = 0;
    for (int i = 0; i < N_MFCC_WINS * N_MELS; i++){                
        
        // Fill input buffer
        if (input_src == OFFLINE){
            ((uint8_t *)output_buffer)[k] = L2_input_h[k]; // Precomputed MFCC
        }
        else {
            // ((uint8_t *)output_buffer)[k] = (char) ((int) floor(MfccOutSig[i] * pow(2, -1) * sqrt(0.05)) + 128);
            ((uint8_t *)output_buffer)[k] = (char) ((int) floor(MfccOutputSignal[i] * 0.1118) + 128); // Online computed MFCC
            // ((uint8_t *)output_buffer)[k] = (char) (((int) floor(MfccOutSig[i] * 0.1118) + 128) * 0.38179088); // Incl. eps_in division
        }

        if (N_MELS == 40){
            // Select 10 MFCC per window
            if (i == 40*(k/10) + 9){
                i = 40*(k/10) + 39;
            }
        }
        k++;
    } 
    pi_l2_free(MfccOutSig, N_MFCC_WINS * N_MELS * sizeof(OUT_TYPE));
    pi_l2_free(MfccInputSignal, 16000 * sizeof(MFCC_IN_TYPE));
    pi_l2_free(MfccOutputSignal, N_MFCC_WINS * N_MELS * sizeof(OUT_TYPE));
}
