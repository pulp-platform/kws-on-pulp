// Copyright (C) 2022 ETH Zurich
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


#ifndef __EMUL__
    #include "pmsis.h"
#else
    #include <stdlib.h>
    #include <stdio.h>
    #define pmsis_exit(a)   exit(a)     
#endif

#include <math.h>

#include "gaplib/wavIO.h"
#include "MFCC_params.h"
#include "MFCCKernels.h"
#include "TwiddlesDef.h"
#include "RFFTTwiddlesDef.h"
#include "SwapTablesDef.h"
#include "network.h"

#include "LUT.def"
#include "MFCC_FB.def"

#define  BUF_SIZE       16500 
#define  STACK_SIZE     2048
#define  NORM           6
#define  N_FRAME        49
#define ALIM_1_VOLT 1
#define FREQ_FC (10000000)
#define FREQ_CL (10000000)


#if (DATA_TYPE==2)
typedef f16 MFCC_IN_TYPE;
typedef f16 OUT_TYPE;
#elif (DATA_TYPE==3)
typedef float MFCC_IN_TYPE;
typedef float OUT_TYPE;
#else
typedef short int OUT_TYPE; 
typedef short int MFCC_IN_TYPE;
#endif

OUT_TYPE *out_feat;
OUT_TYPE *out_fft;
MFCC_IN_TYPE *MfccInSig;
short int *inWav;
int num_samples;
volatile char *FileName;
volatile char *PULPSDK;
char * feat_char;

static void RunMFCC()
{
    #ifdef PERF
        gap_cl_starttimer();
        gap_cl_resethwtimer();
        int start = gap_cl_readhwtimer();
    #endif

    // Compute MFCC following Tensorflow settings
    #if (N_DCT == 0)
            #if (DATA_TYPE==2) || (DATA_TYPE==3)
            Tensorflow_MFCC(MfccInSig, out_feat, R2_Twiddles_float_512, RFFT_Twiddles_float_1024, R2_SwapTable_float_512, WindowLUT, MFCC_FilterBank, MFCC_Coeffs);
            #else
            Tensorflow_MFCC(MfccInSig, out_feat, R2_Twiddles_fix_512,   RFFT_Twiddles_fix_1024,   R2_SwapTable_fix_512,   WindowLUT, MFCC_FilterBank, MFCC_Coeffs, NORM);
            #endif
    #else
            #if (DATA_TYPE==2) || (DATA_TYPE==3)
            Tensorflow_MFCC(MfccInSig, out_feat, R2_Twiddles_float_512, RFFT_Twiddles_float_1024, R2_SwapTable_float_512, WindowLUT, MFCC_FilterBank, MFCC_Coeffs, DCT_Coeff);
            #else
            Tensorflow_MFCC(MfccInSig, out_feat, R2_Twiddles_fix_512,   RFFT_Twiddles_fix_1024,   R2_SwapTable_fix_512,   WindowLUT, MFCC_FilterBank, MFCC_Coeffs, NORM, DCT_Coeff);
            #endif
    #endif
    #ifdef PERF
        int elapsed = gap_cl_readhwtimer() - start;
        printf("Total Cycles: %d over %d Frames %d Cyc/Frame\n", elapsed, N_FRAME, elapsed / N_FRAME);
    #endif
}

void * test_kickoff(void *arg)
{
    #ifndef __EMUL__
        struct pi_device cluster_dev;
        struct pi_cluster_conf cl_conf;
        cl_conf.id = 0;

        pi_open_from_conf(&cluster_dev, (void *) &cl_conf);
        if (pi_cluster_open(&cluster_dev))
        {
            printf("Cluster open failed !\n");
            pmsis_exit(-4);
        }
    #endif
    
    L1_Memory = (AT_L1_POINTER) AT_L1_ALLOC(0, _L1_Memory_SIZE);
    if (L1_Memory==NULL){
        printf("Error allocating L1\n");
        pmsis_exit(-1);
    }

    int frame_size;
    if (N_DCT > 0) frame_size = N_DCT;
    else           frame_size = MFCC_BANK_CNT;

    feat_char = (char*) AT_L2_ALLOC(0, 490 * sizeof(char));    
    out_feat = (OUT_TYPE *) AT_L2_ALLOC(0, N_FRAME * frame_size * sizeof(OUT_TYPE));    
    inWav    = (short int *) AT_L2_ALLOC(0, BUF_SIZE * sizeof(short));   
    MfccInSig = (MFCC_IN_TYPE *) AT_L2_ALLOC(0, BUF_SIZE * sizeof(MFCC_IN_TYPE));   

    if (inWav==NULL){
        printf("Error allocating inWav\n");
        pmsis_exit(1);
    }
    if (MfccInSig==NULL){
        printf("Error allocating MfccInSig\n");
        pmsis_exit(1);
    }
    if (out_feat==NULL){
        printf("Error allocating out_feat\n");
        pmsis_exit(1);
    }

    header_struct header_info;
    if (ReadWavFromFile(FileName, inWav, BUF_SIZE*sizeof(short), &header_info)){
        printf("Error reading wav file\n");
        pmsis_exit(1);
    }
    num_samples = header_info.DataSize * 8 / (header_info.NumChannels * header_info.BitsPerSample);

    #if (DATA_TYPE==2) || (DATA_TYPE==3)
        for (int i=0; i<num_samples; i++) {
            MfccInSig[i] = (MFCC_IN_TYPE) inWav[i] / (1<<15);
        }
    #else
        for (int i=0; i<num_samples; i++) {
            MfccInSig[i] = (MFCC_IN_TYPE) gap_clip(((int) inWav[i]), 15);
        }
    #endif
    
    if (strcmp(PULPSDK, "pulp_sdk") == 0) {
        // PULP
        struct pi_cluster_task cluster_task = {0};
        pi_cluster_task(&cluster_task, pulp_parallel, NULL);
        cluster_task.stack_size = STACK_SIZE;
        cluster_task.slave_stack_size = STACK_SIZE;
        cluster_task.entry = RunMFCC;
        cluster_task.arg = NULL;
        pi_cluster_send_task_to_cl(&cluster_dev, &cluster_task);
    } else {
        // GAP
        struct pi_cluster_task task = {0};
        task.entry = RunMFCC;
        task.arg = NULL;
        task.stack_size = (unsigned int) STACK_SIZE;
        pi_cluster_send_task_to_cl(&cluster_dev, &task);

    }
    // Closing the cluster once the task is finished
    pi_cluster_close(&cluster_dev);

    int k = 0;
    for (int i = 0; i < 1960;i++){
        
        // Rescale MFCCs to match Tensorflow-generated ones
        feat_char[k] = (char) (((int) floor(out_feat[i] * pow(2, -4) * sqrt(0.2))) + 128);
        // Select 10 MFCC per window
        if (i == 40*(k/10) + 9){
            i = 40*(k/10) + 39;
        }
        k++;
    }
}

#ifndef __EMUL__
int main()
{
    #define __XSTR(__s) __STR(__s)
    #define __STR(__s) #__s
    FileName = __XSTR(AT_WAV);
    PULPSDK = __XSTR(SDK);

    // Compute MFCCs
    test_kickoff(NULL); 

    // for (int i = 0; i < 490; i++){
    //     printf("%i\n", feat_char[i]);
    // }
    
    // Model inference
    network_setup(feat_char, 490, 1);
    network_run_FabricController(); 
}
#else
int main(int argc, char *argv[])
{
        if (argc < 2) {
            printf("Usage: mnist [image_file]\n");
            exit(-1);
        }
        FileName = argv[1];
        
        // Compute MFCCs
        test_kickoff(NULL);

        // for (int i = 0; i < 490; i++){
        //     printf("%i\n", feat_char[i]);
        // }

        // Model inference
        network_setup(feat_char, 490, 1);
        network_run_FabricController(); 
}
#endif
