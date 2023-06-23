/*
 * Copyright (C) 2022 GreenWaves Technologies
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 *
 */

/* 
    include files
*/

// #include "mram.h"
// #define pi_default_flash_conf pi_mram_conf

#include "denoiser.h"

// L2
#include "input.h"

// Peripherals
#include "Gap.h"
#include "bsp/ram.h"
#include <bsp/fs/hostfs.h>
#include "gaplib/wavIO.h" 
#include "Graph_L2_Descr.h" // pdm_in_test
#include "localutil.h"

// MFCC
#include "MFCC_params.h"
#include "MfccKernels.h"
#include "DCTTwiddles.def"
#include "MelFBSparsity.def"
#include "WindowLUT.def"
#include "FFTTwiddles.def"
#include "RFFTTwiddles.def"
#include "MelFBCoeff.def"
#include "SwapTable.def"

// DORY
#include "mem.h"
#include "network.h"

// PULP TrainLib
#include "net.h"


/* 
     global variables
*/
struct pi_device DefaultRam; 
struct pi_device* ram = &DefaultRam;

#ifdef AUDIO_EVK
    // GPIO defines
    pi_gpio_e gpio_pin_o; /* PI_GPIO_A02-PI_GPIO_A05 */
    int val_gpio;
#endif

//static struct pi_default_flash_conf flash_conf;
static pi_fs_file_t * file[1];
static struct pi_device fs;
static struct pi_device flash;

// Load args
char *WavName = NULL;
char *mfcc = NULL;
char *input = NULL;

// Arrays handling data movement
short int *inWav;
MFCC_IN_TYPE *MfccInSig;
OUT_TYPE *out_feat;
char * feat_char;

SFU_uDMA_Channel_T *ChanOutCtxt_0;
void * BufferInList;


// Microphone handling
static int open_i2s_PDM(struct pi_device *i2s, unsigned int SAIn, unsigned int Frequency, unsigned int Polarity, unsigned int Diff)
{
    struct pi_i2s_conf i2s_conf;
    pi_i2s_conf_init(&i2s_conf);

    // polarity: b0: SDI: slave/master, b1:SDO: slave/master    1:RX, 0:TX
    // i2s_conf.options = PI_I2S_OPT_REF_CLK_FAST;
    i2s_conf.frame_clk_freq = Frequency;                // In pdm mode, the frame_clk_freq = i2s_clk
    i2s_conf.itf = SAIn;                                // Which sai interface
    i2s_conf.mode = PI_I2S_MODE_PDM;                // Choose PDM mode
    i2s_conf.pdm_direction = Polarity;                   // 2b'11 slave on both SDI and SDO (SDO under test)
    i2s_conf.pdm_diff = Diff;                           // Set differential mode on pairs (TX only)

//    i2s_conf.options |= PI_I2S_OPT_EXT_CLK;             // Put I2S CLK in input mode for safety

    pi_open_from_conf(i2s, &i2s_conf);

    if (pi_i2s_open(i2s))
        return -1;

    pi_pad_set_function(SAI_SCK(SAIn),PI_PAD_FUNC0);
    pi_pad_set_function(SAI_SDI(SAIn),PI_PAD_FUNC0);
    pi_pad_set_function(SAI_SDO(SAIn),PI_PAD_FUNC0);
    pi_pad_set_function(SAI_WS(SAIn),PI_PAD_FUNC0);

    return 0;
}

// MFCC Computation
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
        Tensorflow_MFCC(MfccInSig, out_feat, FFTTwiddles, RFFTTwiddles, SwapTable, WindowLUT, MelFBSparsity, MelFBCoeff);
        #elif (DATA_TYPE==1)
        Tensorflow_MFCC(MfccInSig, out_feat, FFTTwiddles, SwapTable, WindowLUT, MelFBSparsity, MelFBCoeff, NORM);
        #else
        Tensorflow_MFCC(MfccInSig, out_feat, FFTTwiddles, RFFTTwiddles, SwapTable, WindowLUT, MelFBSparsity, MelFBCoeff, NORM);
        #endif
    #else
        #if (DATA_TYPE==2) || (DATA_TYPE==3)
        Tensorflow_MFCC(MfccInSig, out_feat, FFTTwiddles, RFFTTwiddles, SwapTable, WindowLUT, MelFBSparsity, MelFBCoeff, DCTTwiddles);
        #elif (DATA_TYPE==1)
        Tensorflow_MFCC(MfccInSig, out_feat, FFTTwiddles, SwapTable, WindowLUT, MelFBSparsity, MelFBCoeff, NORM, DCTTwiddles);
        #else
        Tensorflow_MFCC(MfccInSig, out_feat, FFTTwiddles, RFFTTwiddles, SwapTable, WindowLUT, MelFBSparsity, MelFBCoeff, NORM, DCTTwiddles);
        #endif
    #endif

    #ifdef PERF
        int elapsed = gap_cl_readhwtimer() - start;
        printf("Total Cycles: %d over %d Frames %d Cyc/Frame\n", elapsed, 49, elapsed / 49);
    #endif
}

static int chunk_in_cnt;

int denoiser(void)
{
    printf("Entering main controller\n");

    /****
        Change Frequency if needed
    ****/
 
    // Voltage-Frequency settings
    uint32_t voltage =VOLTAGE;
    pi_freq_set(PI_FREQ_DOMAIN_FC,      FREQ_FC*1000*1000);
    pi_freq_set(PI_FREQ_DOMAIN_PERIPH,  FREQ_FC*1000*1000);

#ifdef AUDIO_EVK
    pi_pmu_voltage_set(PI_PMU_VOLTAGE_DOMAIN_CHIP, VOLTAGE);
    pi_pmu_voltage_set(PI_PMU_VOLTAGE_DOMAIN_CHIP, VOLTAGE);
#endif 

    //PMU_set_voltage(voltage, 0);
    printf("Set VDD voltage as %.2f, FC Frequency as %d MHz, CL Frequency = %d MHz\n", 
        (float)voltage/1000, FREQ_FC, FREQ_CL);

#ifdef AUDIO_EVK
    /****
        Configure GPIO Output.
    ****/

    //struct pi_gpio_conf gpio_conf = {0};
    gpio_pin_o = PI_GPIO_A89; /* PI_GPIO_A02-PI_GPIO_A05 */
    pi_gpio_flags_e flags = PI_GPIO_OUTPUT;
    pi_gpio_pin_configure(gpio_pin_o, flags);
#endif
    /****
        Configure And Open the External Ram. 
    ****/
    struct pi_default_ram_conf ram_conf;
    pi_default_ram_conf_init(&ram_conf);
    ram_conf.baudrate = FREQ_FC*1000*1000;
    pi_open_from_conf(&DefaultRam, &ram_conf);
    if (pi_ram_open(&DefaultRam))
    {
        printf("Error ram open !\n");
        pmsis_exit(-3);
    }
    printf("RAM Opened\n");

    /****
        Configure And open cluster. 
    ****/

    struct pi_device cluster_dev;
    struct pi_cluster_conf cl_conf;
    pi_cluster_conf_init(&cl_conf);
    cl_conf.cc_stack_size = STACK_SIZE;
    cl_conf.id = 0;                /* Set cluster ID. */
                       // Enable the special icache for the master core
    cl_conf.icache_conf = PI_CLUSTER_MASTER_CORE_ICACHE_ENABLE |   
                       // Enable the prefetch for all the cores, it's a 9bits mask (from bit 2 to bit 10), each bit correspond to 1 core
                       PI_CLUSTER_ICACHE_PREFETCH_ENABLE |      
                       // Enable the icache for all the cores
                       PI_CLUSTER_ICACHE_ENABLE;
    pi_open_from_conf(&cluster_dev, (void *) &cl_conf);
    if (pi_cluster_open(&cluster_dev))
    {
        PRINTF("Cluster open failed !\n");
        pmsis_exit(-4);
    }
    printf("Cluster Opened\n");
    pi_freq_set(PI_FREQ_DOMAIN_CL, FREQ_CL*1000*1000);

    if (input == "1") {
        // Instead of listening from microphone, we read from WAV.
        // Allocate L3 buffers for audio IN
        inWav    = (short int *) pi_l2_malloc(AUDIO_BUFFER_SIZE * sizeof(short));  
        header_struct header_info;
        if (ReadWavFromFile(WavName, inWav, AUDIO_BUFFER_SIZE*sizeof(short), &header_info)){
            printf("Error reading wav file\n");
            pmsis_exit(1);
        }
        int num_samples = header_info.DataSize * 8 / (header_info.NumChannels * header_info.BitsPerSample);
        MfccInSig = (MFCC_IN_TYPE *) pi_l2_malloc(AUDIO_BUFFER_SIZE * sizeof(MFCC_IN_TYPE));
    }


    /******
        Setup MFCC task
    ******/
    printf("Setup MFCC task!\n");
    struct pi_cluster_task* task_mfcc;
    task_mfcc = pi_l2_malloc(sizeof(struct pi_cluster_task));
    pi_cluster_task(task_mfcc,&RunMFCC,NULL);
    if (task_mfcc == NULL) {
        PRINTF("failed to allocate memory for task\n");
    }
    pi_cluster_task_stacks(task_mfcc, NULL, SLAVE_STACK_SIZE);

    
    if (input == "0") {
        
        /****
            Setup the SFU for PDM in/out
        ****/
        struct pi_device i2s_sai1;

        // Configure PDM
        if (open_i2s_PDM(&i2s_sai1, SAI1,   3072000, 3, 0)) return -1;

        StartSFU(FREQ_SFU*1000*1000, 1);
        ChanOutCtxt_0  = (SFU_uDMA_Channel_T *) pi_l2_malloc(sizeof(SFU_uDMA_Channel_T));


        BufferInList = (void*) pi_l2_malloc(BUFF_SIZE);
            
        // Get uDMA channels for Graph
        SFU_Allocate_uDMA_Channel(ChanOutCtxt_0, 0, &SFU_RTD(Graph));

        //Next API will have a value to replace this high number with -1
        //To be able to 
        SFU_Enqueue_uDMA_Channel(ChanOutCtxt_0, BufferInList, BUFF_SIZE);
        
        // Connect Channels to SFU for Mic IN (PDM IN)
        SFU_GraphConnectIO(SFU_Name(Graph, Out1), ChanOutCtxt_0->ChannelId, 0, &SFU_RTD(Graph));
        SFU_GraphConnectIO(SFU_Name(Graph, In1), SAI_ITF_IN, 2, &SFU_RTD(Graph));

        pi_l2_free(ChanOutCtxt_0, sizeof(SFU_uDMA_Channel_T));

        fxl6408_setup();

        printf("Recording!\n");

        //Starting In and Out Graphs
        pi_i2s_ioctl(&i2s_sai1, PI_I2S_IOCTL_START, NULL);
        // Let the microphone start
        pi_time_wait_us(30000); 

        chunk_in_cnt=0;
        SFU_StartGraph(&SFU_RTD(Graph));
        pi_time_wait_us(2000000);

        pi_i2s_ioctl(&i2s_sai1, PI_I2S_IOCTL_STOP, NULL);

        printf("Finished!\n");

        MfccInSig = (MFCC_IN_TYPE *) pi_l2_malloc(BUFF_SIZE);

    }

    // WRITE WAV
    dump_wav_open("test_gap.wav", 16, 16000, 1, sizeof(short)*AUDIO_BUFFER_SIZE);
    dump_wav_write(MfccInSig, sizeof(short)*AUDIO_BUFFER_SIZE);
    dump_wav_close();

    printf("Writing wav file to test_gap.wav completed successfully\n");

    // Dory init
    // TODO: Remove flash init and/or ram init duplicates
    mem_init();
    network_initialize(); // Absent in L2

    while(1){

#ifdef AUDIO_EVK
        pi_gpio_pin_write(gpio_pin_o, 1);
#endif


        if (input == "0") {
            int round = (chunk_in_cnt%CHUNK_NUM);
            int round_out = (chunk_in_cnt>(STRUCT_DELAY-1))? ((chunk_in_cnt-(STRUCT_DELAY-1))%CHUNK_NUM):0;


            printf ("Scale data\n");
            int outidx = 0;
            for(int i=0;i<BUFF_SIZE;i+=3){
                MfccInSig[outidx] = (MFCC_IN_TYPE)(((float)((int32_t*)BufferInList)[i]) /((int)(1<<16)));
                outidx++;
                if (outidx == AUDIO_BUFFER_SIZE){
                    break;
                }
                
            }
            pi_l2_free(BufferInList, BUFF_SIZE);
        }

        if (input == "1"){
            #if (DATA_TYPE==2) || (DATA_TYPE==3)
                for (int i=0; i<AUDIO_BUFFER_SIZE; i++) { // BUFF_SIZE for MIC, AUDIO_BUFFER_SIZE for WAV
                    MfccInSig[i] = (MFCC_IN_TYPE) inWav[i] / (1<<15);
                }
            #else
                for (int i=0; i<AUDIO_BUFFER_SIZE; i++) { // BUFF_SIZE for MIC, AUDIO_BUFFER_SIZE for WAV
                    MfccInSig[i] = (MFCC_IN_TYPE) gap_clip(((int) inWav[i]), 15);
                }
            #endif
            pi_l2_free(inWav, AUDIO_BUFFER_SIZE * sizeof(short));
        }
        
        out_feat = (OUT_TYPE *) pi_l2_malloc(49 * 10 * 4 * sizeof(OUT_TYPE));    
        feat_char = (char*) pi_l2_malloc(49 * 10 * sizeof(char));

        /******
            Compute the MFCC
        ******/
        printf("\n\n****** Computing MFCC ***** \n");
        pi_cluster_task(task_mfcc,&RunMFCC,NULL);
        L1_Memory = pi_l1_malloc(&cluster_dev, _L1_Memory_SIZE);
        if (L1_Memory==NULL){
            printf("Error allocating L1\n");
            pmsis_exit(-1);
        }

        pi_cluster_send_task_to_cl(&cluster_dev, task_mfcc);

        pi_l2_free(task_mfcc, sizeof(struct pi_cluster_task));

        if (input == "0"){
            pi_l2_free(MfccInSig, BUFF_SIZE);
        } else {
            pi_l2_free(MfccInSig, AUDIO_BUFFER_SIZE * sizeof (MFCC_IN_TYPE));
        }

        pi_cluster_close(&cluster_dev);

        printf("MFCC Computation complete. Rescaling data\n");
        int k = 0;
        for (int i = 0; i < 1960;i++){                
            
            feat_char[k] = (char) (((int) floor(out_feat[i] * pow(2, -1) * sqrt(0.05))) + 128); // 23.883617 QSNR w/ float

            // Select 10 MFCC per window
            if (i == 40*(k/10) + 9){
                i = 40*(k/10) + 39;
            }
            k++;
        } 

        // TEST

        // #if (DATA_TYPE==2) || (DATA_TYPE==3)
        // float QSNR_THR = 40;
        // #else
        // float QSNR_THR = 38;
        // #endif
        // int N_FRAME = 49;
        // int frame_size = 10;
        // float MSE = 0.0, SUM = 0.0;
        //     for (int i=0; i<N_FRAME; i++) {
        //         for (int j=0; j<frame_size; j++) {
        //             #if (DATA_TYPE==2) || (DATA_TYPE==3)
        //                   MSE += (L2_input_h[i*frame_size+j] - feat_char[i*frame_size+j])*(L2_input_h[i*frame_size+j] - feat_char[i*frame_size+j]);
        //             #else
        //                   int QMFCC = 15 - NORM - 7;
        //                   MSE += (L2_input_h[i*frame_size+j] - FIX2FP(feat_char[i*frame_size+j], QMFCC)) * (L2_input_h[i*frame_size+j] - FIX2FP(feat_char[i*frame_size+j], QMFCC));
        //             #endif
        //             SUM += (L2_input_h[i*frame_size+j])*(L2_input_h[i*frame_size+j]);
        //         }
        //     }

        //     float QSNR = 10*log10(SUM / MSE);
        //     // Sum is: 7163328.000000, whereas the MSE is: 12514.000000
        //     // Sum is: 7565786.000000, whereas the MSE is: 1696096.000000
        //     printf("\nSum is: %f, whereas the MSE is: %f\n", SUM, MSE);
        //     printf("QSNR: %f (thr: %f) --> ", QSNR, QSNR_THR);
        //     if (QSNR < QSNR_THR) {
        //         printf("Test NOT PASSED\n");
        //         // pmsis_exit(-1);
        //     } else {
        //         printf("Test PASSED\n");
        //     }


        pi_l2_free(out_feat, 49*10*4*sizeof(OUT_TYPE));

        void *l2_buffer;
        l2_buffer = pi_l2_malloc(L2_MEMORY_SIZE);
        if (l2_buffer == NULL) {
            printf("failed to allocate memory for l2_buffer\n");
        }

        for (int i = 0; i < 490; i++){
            if (mfcc == "1"){
                ((uint8_t *)l2_buffer)[i] = L2_input_h[i]; // Precomputed MFCC
            }
            else {
                ((uint8_t *)l2_buffer)[i] = feat_char[i]; // Online computed MFCC
            }
            // printf("%i\n", feat_char[i]); // Online computed MFCC
        }

        printf("Memory allocated.\n");
        // L3
        void *L3_weights_curr; // passing curr weights address, that will be used to update
        network_run(l2_buffer, L2_MEMORY_SIZE, l2_buffer, &L3_weights_curr, 0); // L2_input_h extra-arg for L2-only

        int predicted_class = predict (l2_buffer);

        // Network update
        struct pi_device cluster_dev;
        struct pi_cluster_conf cl_conf;
        struct pi_cluster_task cl_task;

        pi_cluster_conf_init(&cl_conf);
        pi_open_from_conf(&cluster_dev, &cl_conf);
        if (pi_cluster_open(&cluster_dev))
        {
          return -1;
        }

        printf("\nLaunching training procedure...\n");

        // Move weights from TrainLib to Dory
        void *L2_weights_curr_updated;
        L2_weights_curr_updated = pi_l2_malloc(784 * sizeof(float));

        int update = 0;
        int init = 1;

        unsigned int args[2];
        args[0] = (unsigned int) l2_buffer;
        args[1] = (unsigned int) L3_weights_curr;
        args[2] = (unsigned int) L2_weights_curr_updated;
        args[3] = (unsigned int) update;
        args[4] = (unsigned int) init;

        pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, net_step, args));

        printf("Exiting DNN Training.\n");
        pi_cluster_close(&cluster_dev);


        printf("Copied weights:\n");
        for (int i = 0; i < 10; i++){
            printf("W[%i] %f\n", i, ((float*)L2_weights_curr_updated)[i]);
        }

        // clean buffer
        pi_l2_free(l2_buffer, L2_MEMORY_SIZE);


        break;


        // block until next input audio frame is ready
#ifdef  AUDIO_EVK
        pi_gpio_pin_write(gpio_pin_o, 0);
#endif
        chunk_in_cnt++;
    }


    // Close the cluster
    pi_cluster_close(&cluster_dev);
    PRINTF("Ended\n");
    pmsis_exit(0);
    return 0;
}

int main()
{
    PRINTF("\n\n\t *** Denoiser ***\n\n");

    #define __XSTR(__s) __STR(__s)
    #define __STR(__s) #__s
    WavName = __XSTR(WAV_FILE); 
    mfcc = __XSTR(MFCC);
    input = __XSTR(INPUT);

    return denoiser();
}

