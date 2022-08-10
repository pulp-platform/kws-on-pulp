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

#define  L2_BUFFER_SIZE 80000  // ORIGINAL: 380000. TODO: Why it works???
#define  BUF_SIZE       16500 
#define  STACK_SIZE     2048
#define  NORM           6
#define  N_FRAME        49
#define  N_MFCC         10
#define  N_CLASSES      12
#define  ALIM_1_VOLT    1
#define  FREQ_FC (10000000)
#define  FREQ_CL (10000000)

#define __XSTR(__s) __STR(__s)
#define __STR(__s) #__s

// ADDED NOW
#define FLASH_BUFF_SIZE 128
#define VERBOSE 1

static struct pi_hyperflash_conf flash_conf;
static struct pi_hyper_conf ram_conf;
static struct pi_device ram;
static int activations_input;
static uint8_t flashBuffer[FLASH_BUFF_SIZE];

char* L2_output;



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

// filesystem management functions
void open_filesystem_and_ram(struct pi_device *flash, struct pi_device *fs)
{
    struct pi_readfs_conf conf;
    struct pi_hyperflash_conf flash_conf;

    /* Init & open flash. */
    pi_hyperflash_conf_init(&flash_conf);
    pi_open_from_conf(flash, &flash_conf);
    if (pi_flash_open(flash))
    {
        printf("Error flash open !\n");
        pmsis_exit(-1);
    }

    /* Open filesystem on flash. */
    pi_readfs_conf_init(&conf);
    conf.fs.flash = flash;
    pi_open_from_conf(fs, &conf);
    if (pi_fs_mount(fs))
    {
        printf("Error FS mounting !\n");
        pmsis_exit(-2);
    }
    pi_task_t task = {0};
    pi_task_block(&task);
    pi_hyperram_conf_init(&ram_conf);
    pi_open_from_conf(&ram, &ram_conf);
    pi_ram_open(&ram);
}

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

        printf ("Test kickoff - init \n");

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

    feat_char = (char*) pi_l2_malloc(N_FRAME * N_MFCC * sizeof(char));
    out_feat = (OUT_TYPE *) pi_l2_malloc(N_FRAME * frame_size * sizeof(OUT_TYPE));    
    inWav    = (short int *) pi_l2_malloc(BUF_SIZE * sizeof(short));   
    MfccInSig = (MFCC_IN_TYPE *) pi_l2_malloc(BUF_SIZE * sizeof(MFCC_IN_TYPE));   

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
    
    printf ("SDK: %s", PULPSDK);
    if (strcmp(PULPSDK, "pulp_sdk") == 0) {
        // PULP
        struct pi_cluster_task cluster_task = {0};
        // pi_cluster_task(&cluster_task, pulp_parallel, NULL);
        // Preplace pulp_parallel with pi_cl_team_fork - is NUM_CORE included?
        pi_cluster_task(&cluster_task, pi_cl_team_fork, NULL); 
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


    pi_l2_free(out_feat, (uint32_t) N_FRAME * frame_size * sizeof(OUT_TYPE));
    pi_l2_free(inWav, (uint32_t) BUF_SIZE * sizeof(short));
    pi_l2_free(MfccInSig, (uint32_t) BUF_SIZE * sizeof(MFCC_IN_TYPE));

   
}

#ifndef __EMUL__

int main () {


    FileName = __XSTR(AT_WAV);
    PULPSDK = __XSTR(SDK);


    // Compute MFCCs
    test_kickoff(NULL); 

    for (int i = 0; i < 490; i++){
        printf("%i\n", feat_char[i]);
    }

    
    char* L2_memory_buffer;
    char* L2_input;
    if (strcmp(PULPSDK, "gap_sdk") == 0){
        // PMU_set_voltage(1000, 0);
    }
    pi_time_wait_us(10000);
    pi_freq_set(PI_FREQ_DOMAIN_FC, FREQ_FC);
    pi_time_wait_us(10000);
    pi_freq_set(PI_FREQ_DOMAIN_CL, 10000000);
    pi_time_wait_us(10000);

    if (strcmp(PULPSDK, "pulp_sdk") == 0){
        #if __PLATFORM__ == ARCHI_PLATFORM_FPGA
            *(int*)(ICACHE_PREFETCH) = 0xFFFF;
        #endif
    }

    int use_mfcc = 1;
    int use_l3 = 0;


    int rdDone;
    if (use_l3 == 1) {
        rdDone = 0;
    }
    else {
        rdDone = N_FRAME * N_MFCC;
    }

    struct pi_device fs;
    struct pi_device flash;
    // Opening of Filesystem and Ram
    if (use_l3 == 1) {

        open_filesystem_and_ram(&flash, &fs);
        pi_ram_alloc(&ram, &activations_input, (uint32_t) 500000);
        
        if (use_mfcc == 0) {

            pi_fs_file_t *file;
            file = pi_fs_open(&fs, "inputs.hex", 0);
            if (file == NULL)
            {
                printf("file open failed\n");
                return -1;
            }


            // Copying the input file from flash to ram
            int flashBuffSize = FLASH_BUFF_SIZE * sizeof(char);
            // loop on chunk in file
            // while(rdDone < (${int(DORY_HW_graph[0].tiling_dimensions["L2"]["input_activation_memory"])} / sizeof(char)))
            while(rdDone < ( N_FRAME * N_MFCC / sizeof(char)))
            {
                // read from HyperFlash
                int size = pi_fs_read(file, flashBuffer, flashBuffSize);
                // write to HyperRam
                pi_ram_write(&ram, activations_input+rdDone, flashBuffer, (uint32_t) size);
                rdDone += size / sizeof(char);
            }
        }
        else {
            int input_size = 8 * N_FRAME * N_MFCC;
            pi_ram_write(&ram, activations_input+rdDone, feat_char, (uint32_t) input_size);
        }
    }

    // Allocating space for input and copying it

    // L2_memory_buffer = pi_l2_malloc((uint32_t) ${l2_buffer_size});
    L2_memory_buffer = pi_l2_malloc((uint32_t) L2_BUFFER_SIZE);
    int begin_end = 1;
    // L2_input = L2_memory_buffer + (1 - begin_end) * (${l2_buffer_size} - rdDone);
    L2_input = L2_memory_buffer + (1 - begin_end) * (L2_BUFFER_SIZE - rdDone);
    L2_output = L2_memory_buffer;
#ifdef VERBOSE
    printf("\nL2 Buffer alloc initial\t@ 0x%08x:\t%s\n", (unsigned int)L2_memory_buffer, L2_memory_buffer?"Ok":"Failed");
#endif

    // Allocation

    if (use_l3 == 1) {
        // pi_ram_read(&ram, activations_input, L2_input, ${int(DORY_HW_graph[0].tiling_dimensions["L2"]["input_activation_memory"])});
        pi_ram_read(&ram, activations_input, L2_input, N_FRAME * N_MFCC);
        network_alloc(fs, ram);    

        // Running of the network

        // network_run(L2_memory_buffer, ${l2_buffer_size}, L2_output, begin_end, ram);
        // network_run(L2_memory_buffer, L2_BUFFER_SIZE, L2_output, begin_end, ram); # TODO: Use IFDEF
    }
    else {

        int use_precomputed_features = 0; // 0 - use now-computed, 1 - use pre-computed
        int input_size = N_MFCC * N_FRAME;

        network_alloc();  
        network_run(L2_memory_buffer, L2_BUFFER_SIZE, L2_output, begin_end, feat_char, input_size, use_precomputed_features);

    }
#ifdef VERBOSE
    printf("Network Output: ");
    // for(int i = 0; i < ${int(DORY_HW_graph[-1].tiling_dimensions["L2"]["output_activation_memory"] * (1 + int(DORY_HW_graph[-1].tiling_dimensions["L3"]["output_dimensions"] != DORY_HW_graph[-1].tiling_dimensions["L2"]["output_dimensions"]))) }; i+=4)
    for(int i = 0; i < N_CLASSES * 4; i+=4)
    {
        printf("%d ", *(int32_t *)(L2_output + i));
    }
    printf("\n");
#endif

    // Deallocation
    if (use_l3 == 1) {
        pi_ram_free(&ram, activations_input, 500000);
        network_free(ram);    
        // pi_l2_free(L2_memory_buffer, (uint32_t) ${l2_buffer_size});
        pi_l2_free(L2_memory_buffer, (uint32_t) L2_BUFFER_SIZE);
    }
    else{
        network_free();  
        pi_l2_free(L2_memory_buffer, (uint32_t) L2_BUFFER_SIZE);
    }
}


#else
//TODO: Update
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
