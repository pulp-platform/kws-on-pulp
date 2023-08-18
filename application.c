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

#include "application.h"

#define DATA_TYPE 2 // TODO: Understand why this works
#if (DATA_TYPE==2)
// typedef F16_DSP MFCC_IN_TYPE;
// typedef F16_DSP OUT_TYPE;
// typedef F16 MFCC_IN_TYPE;
// typedef F16 OUT_TYPE;
// typedef f16 MFCC_IN_TYPE;
// typedef f16 OUT_TYPE;
typedef float16 MFCC_IN_TYPE;
typedef float16 OUT_TYPE;
// typedef struct float16 MFCC_IN_TYPE;
// typedef struct float16 OUT_TYPE;
#elif (DATA_TYPE==3)
typedef float MFCC_IN_TYPE;
typedef float OUT_TYPE;
#else
typedef short int OUT_TYPE;  // Save MFCCs works 
typedef short int MFCC_IN_TYPE; // Save MFCCs works
#endif


// #define DATA_TYPE 1
// typedef short int OUT_TYPE;  // Save MFCCs works 
// typedef short int MFCC_IN_TYPE; // Save MFCCs works

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

// Clean utterances
#include "utterances.h"

// Test utterances
#include "tinytest.h"

#include "noise_meeting.h"


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
char *eval_input = NULL;
char *appl_input = NULL;

// Arrays handling data movement
short int *inWav;
MFCC_IN_TYPE *MfccInSig;
int16_t *MfccInSig_int16; // for saving
MFCC_IN_TYPE *RecordedNoise;
int16_t *RecordedNoise_int16; // for saving
OUT_TYPE *out_feat;
char * feat_char;

void *l2_buffer;
void * L2_FC_weights_float;
void *L2_FC_weights_int8;

SFU_uDMA_Channel_T *ChanOutCtxt_0;
void * BufferInList;

int noise_seconds = 1;

static const pi_gpio_e gpio_boot_pin_1 = PAD_GPIO_UPB;

// Global declaration 
struct pi_device cluster_dev;
struct pi_cluster_conf cl_conf;
struct pi_cluster_task cl_task;


// Microphone handling
static int open_i2s_PDM(struct pi_device *i2s, unsigned int SAIn, unsigned int Frequency, unsigned int Polarity, unsigned int Diff)
{
    struct pi_i2s_conf i2s_conf;
    pi_i2s_conf_init(&i2s_conf);

    // polarity: b0: SDI: slave/master, b1:SDO: slave/master    1:RX, 0:TX
    i2s_conf.options = PI_I2S_OPT_REF_CLK_FAST;
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

void input_mic(int save, int free, int noise){
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
    SFU_GraphConnectIO(SFU_Name(Graph, In1), SAI1, 2, &SFU_RTD(Graph));
    pi_l2_free(ChanOutCtxt_0, sizeof(SFU_uDMA_Channel_T));

    printf("Start rec!\n");

    //Starting In and Out Graphs
    pi_i2s_ioctl(&i2s_sai1, PI_I2S_IOCTL_START, NULL);
    // Let the microphone start
    pi_time_wait_us(30000); 

    SFU_StartGraph(&SFU_RTD(Graph));
    pi_time_wait_us(2000000);

    pi_i2s_ioctl(&i2s_sai1, PI_I2S_IOCTL_STOP, NULL);

    printf("Finish rec!\n");

    int outidx;

    if (noise){
        RecordedNoise = (MFCC_IN_TYPE *) pi_l2_malloc(BUFF_SIZE/3/2);
        RecordedNoise_int16 = (int16_t *) pi_l2_malloc(sizeof(int16_t) * AUDIO_BUFFER_SIZE);
        for(int i=0;i<BUFF_SIZE;i+=3){
            RecordedNoise[outidx] = (MFCC_IN_TYPE) (((float)((int32_t *)BufferInList)[i]) / (float)(1<<31 - 1));
            RecordedNoise_int16[outidx] = (int16_t) (RecordedNoise[outidx] * (1<<15));

            outidx++;
            if (outidx == AUDIO_BUFFER_SIZE){
                break;
            }
        } 
    }
    else {

        MfccInSig = (MFCC_IN_TYPE *) pi_l2_malloc(BUFF_SIZE/3/2);
        MfccInSig_int16 = (int16_t *) pi_l2_malloc(sizeof(int16_t) * AUDIO_BUFFER_SIZE);
        for(int i=0;i<BUFF_SIZE;i+=3){
            MfccInSig[outidx] = (MFCC_IN_TYPE) (((float)((int32_t *)BufferInList)[i]) / (float)(1<<31 - 1));
            MfccInSig_int16[outidx] = (int16_t) (MfccInSig[outidx] * (1<<15));

            outidx++;
            if (outidx == AUDIO_BUFFER_SIZE){
                break;
            }
        }
    }


    for (int i = 0; i < AUDIO_BUFFER_SIZE; i++){
        PRINTF("%i\n", MfccInSig_int16[i]);
    }

    if (save) {
        if (noise){
            // Dumping the treated buffer
            dump_wav_open("recording_noise.wav", 16, 16000, 1, sizeof(int16_t) * AUDIO_BUFFER_SIZE);
            dump_wav_write(RecordedNoise_int16, sizeof(int16_t) *AUDIO_BUFFER_SIZE);

            // Dumping the buffer
            // dump_wav_open("recording.wav", 32, 48000, 1, BUFF_SIZE);
            // dump_wav_write(BufferInList, BUFF_SIZE);
            dump_wav_close();
            PRINTF("Writing wav file to recording.wav completed successfully\n");
            pi_l2_free(RecordedNoise_int16, sizeof(int16_t) * AUDIO_BUFFER_SIZE);
        }
        else{
            // Dumping the treated buffer
            dump_wav_open("recording_utterance.wav", 16, 16000, 1, sizeof(int16_t) * AUDIO_BUFFER_SIZE);
            dump_wav_write(MfccInSig_int16, sizeof(int16_t) *AUDIO_BUFFER_SIZE);

            // Dumping the buffer
            // dump_wav_open("recording.wav", 32, 48000, 1, BUFF_SIZE);
            // dump_wav_write(BufferInList, BUFF_SIZE);
            dump_wav_close();
            PRINTF("Writing wav file to recording.wav completed successfully\n");
            pi_l2_free(MfccInSig_int16, sizeof(int16_t) * AUDIO_BUFFER_SIZE);
        }
    }

    pi_l2_free(BufferInList, BUFF_SIZE);
}


void input_wav(int save, int free, char* wavfile, int noise){
    // Allocate L3 buffers for audio IN
     
    header_struct header_info;

    inWav = NULL;
    inWav    = (short int *) pi_l2_malloc(AUDIO_BUFFER_SIZE * sizeof(short)); 
    if (inWav == NULL){
        printf("Failed allocating inWav.\n");
        pmsis_exit(-1);
    }

    PRINTF("File is: %s\n", wavfile);

    if (ReadWavFromFile(wavfile, inWav, AUDIO_BUFFER_SIZE*sizeof(short), &header_info)){
        printf("Error reading wav file\n");
        pmsis_exit(1);
    }

    for (int i = 0; i < 5; i++){
        PRINTF("inWav[%i] = %i, ", i, inWav[i]);
    }
    PRINTF("\n");


    if (noise){
        RecordedNoise = NULL;
        RecordedNoise = (MFCC_IN_TYPE *) pi_l2_malloc(noise_seconds*AUDIO_BUFFER_SIZE * sizeof(MFCC_IN_TYPE));
        if (RecordedNoise == NULL){
            printf("Failed allocating RecordedNoise.\n");
            pmsis_exit(-1);
        }
        #if (DATA_TYPE==2) || (DATA_TYPE==3)
            for (int i=0; i<noise_seconds*AUDIO_BUFFER_SIZE; i++) { // BUFF_SIZE for MIC, AUDIO_BUFFER_SIZE for WAV
                // READ WAV
                RecordedNoise[i] = (MFCC_IN_TYPE) inWav[i] / (1<<15);
                // READ TEXT
                // RecordedNoise[i] = (MFCC_IN_TYPE) noisemeeting[i] / (1<<15);
            }
        #else
            for (int i=0; i<noise_seconds*AUDIO_BUFFER_SIZE; i++) { // BUFF_SIZE for MIC, AUDIO_BUFFER_SIZE for WAV
                // READ WAV
                RecordedNoise[i] = (MFCC_IN_TYPE) gap_clip(((int) inWav[i]), 15);
                // READ TEXT
                // RecordedNoise[i] = (MFCC_IN_TYPE) gap_clip(((int) noisemeeting[i]), 15);
            }
        #endif

    }
    else {
        MfccInSig = NULL;
        MfccInSig = (MFCC_IN_TYPE *) pi_l2_malloc(AUDIO_BUFFER_SIZE * sizeof(MFCC_IN_TYPE));
        if (MfccInSig == NULL){
            printf("Failed allocating MfccInSig.\n");
            pmsis_exit(-1);
        }
    
        #if (DATA_TYPE==2) || (DATA_TYPE==3)
            for (int i=0; i<AUDIO_BUFFER_SIZE; i++) { // BUFF_SIZE for MIC, AUDIO_BUFFER_SIZE for WAV
                MfccInSig[i] = (MFCC_IN_TYPE) inWav[i] / (1<<15);
            }
        #else
            for (int i=0; i<AUDIO_BUFFER_SIZE; i++) { // BUFF_SIZE for MIC, AUDIO_BUFFER_SIZE for WAV
                MfccInSig[i] = (MFCC_IN_TYPE) gap_clip(((int) inWav[i]), 15);
            }
        #endif
    }
    
    if (save){
        // Log WAV 
        // TODO: use *_int16 for saving
        if (noise){
            dump_wav_open("noise_file.wav", 16, 16000, 1, noise_seconds*sizeof(short)*AUDIO_BUFFER_SIZE);
            dump_wav_write(inWav, noise_seconds*sizeof(short)*AUDIO_BUFFER_SIZE);
            dump_wav_close();
            PRINTF("Writing wav file to noise_file.wav completed successfully\n");
        }
        else{   
            dump_wav_open("utter_file.wav", 16, 16000, 1, sizeof(short)*AUDIO_BUFFER_SIZE);
            dump_wav_write(inWav, sizeof(short)*AUDIO_BUFFER_SIZE);
            dump_wav_close();
            PRINTF("Writing wav file to utter_file.wav completed successfully\n");
        }
        
    }

    if (noise){
        pi_l2_free(inWav, noise_seconds*AUDIO_BUFFER_SIZE * sizeof(short));
    }
    else{
        pi_l2_free(inWav, AUDIO_BUFFER_SIZE * sizeof(short));
    }
    
    

}

void compute_mfcc(mode){
    /******
        Compute the MFCC
    ******/
    out_feat = (OUT_TYPE *) pi_l2_malloc(49 * 10 * 4 * sizeof(OUT_TYPE));    
    PRINTF("\n\n********** Computing MFCC **********\n");


    // struct pi_cluster_task task_mfcc;

    struct pi_cluster_task* task_mfcc;
    task_mfcc = pi_l2_malloc(sizeof(struct pi_cluster_task));
    pi_cluster_task(task_mfcc, &RunMFCC, NULL);
    pi_cluster_task_stacks(task_mfcc, NULL, SLAVE_STACK_SIZE);

    pi_cluster_conf_init(&cl_conf);
    pi_open_from_conf(&cluster_dev, &cl_conf);
    if (pi_cluster_open(&cluster_dev))
    {
      return -1;
    }
    L1_Memory = pi_l1_malloc(&cluster_dev, _L1_Memory_SIZE);
    if (L1_Memory==NULL){
        printf("Error allocating L1\n");
        pmsis_exit(-1);
    }
   
    // pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(task_mfcc, RunMFCC, NULL));

    pi_cluster_send_task_to_cl(&cluster_dev, task_mfcc);
    pi_l2_free(task_mfcc, sizeof(struct pi_cluster_task));

    pi_cluster_close(&cluster_dev);

    // mode: 0 - default application; 1 - evaluate
    if (mode == 0){
        if (appl_input == "0"){
            pi_l2_free(MfccInSig, BUFF_SIZE/3/2);
        } else if (eval_input == "1") {
            // TODO: separate noise and utterances
            pi_l2_free(MfccInSig, noise_seconds * AUDIO_BUFFER_SIZE * sizeof (short));
        }
    }
    else{
        if (eval_input == "0"){
            pi_l2_free(MfccInSig, BUFF_SIZE/3/2);
        } else if (eval_input == "1") {
            // TODO: separate noise and utterances
            pi_l2_free(MfccInSig, noise_seconds * AUDIO_BUFFER_SIZE * sizeof (short));
        }
    }

}


void evaluate_tinytest(){

    for (int tinytestidx = 0; tinytestidx < 10; tinytestidx++){ // only non-unknown

        PRINTF ("-----------------------------Loop evaluation (itteration %i)-------------------------\n", tinytestidx);

        // Read from WAV
        if (eval_input == "1") {
            printf("Tested input: %s\n", tinytestutter[tinytestidx]);
            input_wav(1, 1, tinytestutter[tinytestidx], 0); // save, free, noise
        }
        // Read from MIC
        else if (eval_input == "0") {
            input_mic(1, 1, 0); // save, free, noise
#ifdef AUDIO_EVK
            pi_gpio_pin_write(gpio_pin_o, 1);
#endif
        }
        for (int i = 0; i < 10; i++){
            PRINTF("MfccInSig[%i] = %f, ", i, MfccInSig[i]);
        }
        PRINTF("\n");

        MfccInSig_int16 = (int16_t *) pi_l2_malloc (sizeof(int16_t) * AUDIO_BUFFER_SIZE);
        for (int i = 0; i < AUDIO_BUFFER_SIZE; i++){
            MfccInSig_int16[i] = (int16_t) (MfccInSig[i] * (1<<15));
        }
        // Save the noise-augmented recording
        dump_wav_open("utter.wav", 16, 16000, 1, sizeof(int16_t) * AUDIO_BUFFER_SIZE);
        dump_wav_write(MfccInSig_int16, sizeof(int16_t) * AUDIO_BUFFER_SIZE);
        dump_wav_close();
        PRINTF("Writing wav file to utter.wav completed successfully\n");

        int addnoise = 1;
        if (addnoise == 1 && eval_input == "1") {
            int noisesamplestart = 0; // TODO: random sample between (0, len(wav)-16000)
            
            for (int samplepos = 0; samplepos < AUDIO_BUFFER_SIZE; samplepos++){
                MfccInSig[samplepos] = MfccInSig[samplepos] + 1*RecordedNoise[noisesamplestart+samplepos];
            }

            for (int i = 0; i < AUDIO_BUFFER_SIZE; i++){
                MfccInSig_int16[i] = (int16_t)(MfccInSig[i] * (1<<15)) + 1*RecordedNoise_int16[i];
            }

            // Save the noise-augmented recording
            dump_wav_open("utter_noise.wav", 16, 16000, 1, sizeof(int16_t) * AUDIO_BUFFER_SIZE);
            dump_wav_write(MfccInSig_int16, sizeof(int16_t) *AUDIO_BUFFER_SIZE);
            dump_wav_close();

            pi_l2_free(MfccInSig_int16, sizeof(int16_t) * AUDIO_BUFFER_SIZE);

            PRINTF("Writing wav file to utter_noise.wav completed successfully\n");
        }

        compute_mfcc(1);

        for (int i = 0; i < 5; i++){
            PRINTF("out_feat[%i] = %f, ", i, out_feat[i]);
        }
        PRINTF("\n");
        
       feat_char = (char*) pi_l2_malloc(49 * 10 * sizeof(char));
        // Rescale data
        int k = 0;
        for (int i = 0; i < 1960;i++){                
            
            feat_char[k] = (char) ((int) floor(out_feat[i] * pow(2, -1) * sqrt(0.05)) + 128);

            // Select 10 MFCC per window
            if (i == 40*(k/10) + 9){
                i = 40*(k/10) + 39;
            }
            k++;
        } 

        // // PATCH
        // for (int i = 0; i < 10; i++){
        //     feat_char[i+480] = feat_char[i];  
        // }     

        pi_l2_free(out_feat, 49*10*4*sizeof(OUT_TYPE));

        // Fill input buffer
        for (int i = 0; i < 490; i++){
            if (mfcc == "1"){
                ((uint8_t *)l2_buffer)[i] = L2_input_h[i]; // Precomputed MFCC
            }
            else {
                ((uint8_t *)l2_buffer)[i] = feat_char[i]; // Online computed MFCC
                PRINTF("%i,", feat_char[i]);

            }
        }
        PRINTF("\n");

        for (int i = 0; i < 5; i++){
            PRINTF("feat_char[%i] = %i, ", i, feat_char[i]);
        }
        PRINTF("\n");

        pi_l2_free(feat_char, 49 * 10 * sizeof(char));


        // Extract backbone features
        void *dump; // dump to copy FC weights, won't be used; TODO: Parametrize DORY
        network_run(l2_buffer, L2_MEMORY_SIZE, l2_buffer, &dump, 0); // L2_input_h extra-arg for L2-only

        for (int i=0; i < 64; i++){
            PRINTF("%i, ", ((uint8_t *) l2_buffer)[i]);
        }
        PRINTF("\n");

        PRINTF ("********** Run classifier **********\n");

        pi_cluster_conf_init(&cl_conf);
        pi_open_from_conf(&cluster_dev, &cl_conf);
        if (pi_cluster_open(&cluster_dev))
        {
          return -1;
        }

        unsigned int args_inference_classifier[5];
        args_inference_classifier[0] = (unsigned int) l2_buffer;
        args_inference_classifier[1] = (unsigned int) dump;
        args_inference_classifier[2] = (unsigned int) L2_FC_weights_float;
        args_inference_classifier[3] = (unsigned int) 0; // update = 1
        args_inference_classifier[4] = (unsigned int) 0; // init = 1
        args_inference_classifier[5] = (unsigned int) tinytestidx + 2; // tinytest already ordered

        pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, net_step, args_inference_classifier));
        pi_cluster_close(&cluster_dev);

        PRINTF ("********** Task completed **********\n");

        
    #ifdef  AUDIO_EVK
        // block until next input audio frame is ready
        pi_gpio_pin_write(gpio_pin_o, 0);
    #endif
    }
}


void train_wavsrc(){

    int sampleidx;
    int classidx;
    int samplestart;


    int nepochs = 10; // GVSOC - DEMO (mem leak?)
    // int nepochs = 1; // BOARD - QUICK DEMO

    for (int epidx = 0; epidx < nepochs; epidx++) {
        // for (int uttridx = 0; uttridx < 2; uttridx++){ // simple, to speed test
        for (int uttridx = 0; uttridx < 100; uttridx++){

        sampleidx = uttridx / 10;
        classidx = uttridx % 10;

        classidx += 2; // no SILENCE, no UNKNOWN
        char *utterance;

        switch (classidx) {
            case 0: // 0
                continue; // TODO: Get data 
                utterance = class_0[sampleidx];
                break;
            case 1: // 0
                continue; // TODO: Get data 
                utterance = class_1[sampleidx];
                break;
            case 2:
                utterance = class_2[sampleidx];
                break;
            case 3:
                utterance = class_3[sampleidx];
                break;
            case 4:
                utterance = class_4[sampleidx];
                break;
            case 5:
                utterance = class_5[sampleidx];
                break;
            case 6:
                utterance = class_6[sampleidx];
                break;
            case 7:
                utterance = class_7[sampleidx];
                break;
            case 8:
                utterance = class_8[sampleidx];
                break;
            case 9:
                utterance = class_9[sampleidx];
                break;
            case 10:
                utterance = class_10[sampleidx];
                break;
            case 11:
                utterance = class_11[sampleidx];
                break;
        }

        // Load Utterance
        input_wav(0, 1, utterance, 0);  // save, free, utterance, noise

        int localaddnoise = 1;
        if (localaddnoise){
            samplestart = 0; // TODO: random sample between (0, len(wav)-16000)
            for (int samplepos = 0; samplepos < AUDIO_BUFFER_SIZE; samplepos++){
                MfccInSig[samplepos] = MfccInSig[samplepos] + 1*RecordedNoise[samplestart+samplepos]; 
            }
        }

        compute_mfcc(1);

        feat_char = (char*) pi_l2_malloc(49 * 10 * sizeof(char));
        // Rescale data
        int k = 0;
        for (int i = 0; i < 1960;i++){                
            
            feat_char[k] = (char) (((int) floor(out_feat[i] * pow(2, -1) * sqrt(0.05))) + 128); // 23.883617 QSNR w/ float
            // feat_char[k] = (char) (((int) floor(out_feat[i] * pow(2, -4) * sqrt(0.2))) + 128); // kws-on-pulp


            // TODO: Determine Librosa scaling
            

            // Select 10 MFCC per window
            if (i == 40*(k/10) + 9){
                i = 40*(k/10) + 39;
            }
            k++;
        } 
        
        pi_l2_free(out_feat, 49*10*4*sizeof(OUT_TYPE));

        // Fill input buffer

        // l2_buffer = pi_l2_malloc(L2_MEMORY_SIZE);
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
        }
        pi_l2_free(feat_char, 49 * 10 * sizeof(char));


        // Extract backbone features
        network_run(l2_buffer, L2_MEMORY_SIZE, l2_buffer, &L2_FC_weights_int8, 0); // L2_input_h extra-arg for L2-only

        PRINTF ("********** Run classifier **********\n");

        pi_cluster_conf_init(&cl_conf);
        pi_open_from_conf(&cluster_dev, &cl_conf);
        if (pi_cluster_open(&cluster_dev))
        {
          return -1;
        }

        unsigned int args_train_classifier[5];
        args_train_classifier[0] = (unsigned int) l2_buffer;
        args_train_classifier[1] = (unsigned int) L2_FC_weights_int8;
        args_train_classifier[2] = (unsigned int) L2_FC_weights_float;
        args_train_classifier[3] = (unsigned int) 1; // update = 1
        if (uttridx == 0 && epidx == 0)
            args_train_classifier[4] = (unsigned int) 1; // init = 1
        else
            args_train_classifier[4] = (unsigned int) 0; // init = 0   
        
        args_train_classifier[5] = (unsigned int) classidx;

        pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, net_step, args_train_classifier));
        PRINTF ("Finished task...\n");

        pi_cluster_close(&cluster_dev);

        }

    } // samples per epoch

}

int read_button(){
    int button_is_pressed;
    pi_gpio_pin_read(gpio_boot_pin_1, &button_is_pressed);
    return button_is_pressed;
}


int application(void){

    printf ("----------------------------- Initializing environment ---------------------------\n");

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


    // Configure User Button
    /* set pad to gpio mode */
    /* This will open the gpio automatically */
    pi_pad_function_set(gpio_boot_pin_1, PI_PAD_FUNC1);

    /* configure gpio input */
    pi_gpio_flags_e flags_upb = PI_GPIO_INPUT;
    pi_gpio_pin_configure(gpio_boot_pin_1, flags_upb);


    PRINTF ("----------------------------- Initializing backbone ---------------------------\n");
    // Dory init
    mem_init();
    network_initialize(); // Absent in L2-only
    pi_cluster_close(&cluster_dev);

    // TODO: Comment in
    // DORY - TrainLib FC weights copy
    l2_buffer = pi_l2_malloc(L2_MEMORY_SIZE);
    if (l2_buffer == NULL) {
        printf("failed to allocate memory for l2_buffer\n");
    }
    network_run(l2_buffer, L2_MEMORY_SIZE, l2_buffer, &L2_FC_weights_int8, 0); // L2_input_h extra-arg for L2-only

    // Run classifier
    pi_cluster_conf_init(&cl_conf);
    pi_open_from_conf(&cluster_dev, &cl_conf);
    if (pi_cluster_open(&cluster_dev))
    {
      return -1;
    }
    
    PRINTF ("----------------------------- Initializing classifier ---------------------------\n");

    L2_FC_weights_float = pi_l2_malloc (768 * 4);
    if (L2_FC_weights_float == NULL) {
        printf("failed to allocate memory for L2_FC_weights_float\n");
    }

    unsigned int args_init_classifier[5];
    args_init_classifier[0] = (unsigned int) l2_buffer;
    args_init_classifier[1] = (unsigned int) L2_FC_weights_int8; // Weights buffer
    args_init_classifier[2] = (unsigned int) L2_FC_weights_float;
    args_init_classifier[3] = (unsigned int) 0; // update = 0
    args_init_classifier[4] = (unsigned int) 1; // init = 0
    args_init_classifier[5] = (unsigned int) 100; // dummy class

    pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, net_step, args_init_classifier));

    pi_cluster_close(&cluster_dev);

    // pi_l2_free(l2_buffer, L2_MEMORY_SIZE);

    int button_was_pressed = 0;

    PRINTF ("----------------------------- Starting application ---------------------------\n");
    while (1) {

        char utterName[130] = "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser_audiov2/tiny_denoiser/res/meeting_ch01_mancrop1.wav";

        if (appl_input == "0"){
            input_mic(1, 1, 0); // save, free, noise
        }
        else if (appl_input == "1"){
            input_wav(1, 1, utterName, 0); // save, free, utterName, noise
        }

        compute_mfcc(0); // mode 0: application

        for (int i = 0; i < 5; i++){
            PRINTF("out_feat[%i] = %f, ", i, out_feat[i]);
        }
        PRINTF("\n");
        
        feat_char = (char*) pi_l2_malloc(49 * 10 * sizeof(char));
        // Rescale data
        int k = 0;
        for (int i = 0; i < 1960;i++){                
            
            feat_char[k] = (char) ((int) floor(out_feat[i] * pow(2, -1) * sqrt(0.05)) + 128);

            // Select 10 MFCC per window
            if (i == 40*(k/10) + 9){
                i = 40*(k/10) + 39;
            }
            k++;
        } 

        pi_l2_free(out_feat, 49*10*4*sizeof(OUT_TYPE));

        // Fill input buffer
        for (int i = 0; i < 490; i++){
            if (mfcc == "1"){
                ((uint8_t *)l2_buffer)[i] = L2_input_h[i]; // Precomputed MFCC
            }
            else {
                ((uint8_t *)l2_buffer)[i] = feat_char[i]; // Online computed MFCC
                PRINTF("%i,", feat_char[i]);

            }
        }
        PRINTF("\n");

        for (int i = 0; i < 5; i++){
            PRINTF("feat_char[%i] = %i, ", i, feat_char[i]);
        }
        PRINTF("\n");

        pi_l2_free(feat_char, 49 * 10 * sizeof(char));


        // Extract backbone features
        void *dump; // dump to copy FC weights, won't be used; TODO: Parametrize DORY
        network_run(l2_buffer, L2_MEMORY_SIZE, l2_buffer, &dump, 0); // L2_input_h extra-arg for L2-only

        for (int i=0; i < 64; i++){
            PRINTF("%i, ", ((uint8_t *) l2_buffer)[i]);
        }
        PRINTF("\n");

        PRINTF ("********** Run classifier **********\n");

        pi_cluster_conf_init(&cl_conf);
        pi_open_from_conf(&cluster_dev, &cl_conf);
        if (pi_cluster_open(&cluster_dev))
        {
          return -1;
        }

        unsigned int args_inference_classifier[5];
        args_inference_classifier[0] = (unsigned int) l2_buffer;
        args_inference_classifier[1] = (unsigned int) dump;
        args_inference_classifier[2] = (unsigned int) L2_FC_weights_float;
        args_inference_classifier[3] = (unsigned int) 0; // update = 1
        args_inference_classifier[4] = (unsigned int) 0; // init = 1
        args_inference_classifier[5] = (unsigned int) 0; // tinytest already ordered

        pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, net_step, args_inference_classifier));
        pi_cluster_close(&cluster_dev);

        
        #ifdef  AUDIO_EVK
            // block until next input audio frame is ready
            pi_gpio_pin_write(gpio_pin_o, 0);
        #endif

        button_was_pressed = read_button();

        if (button_was_pressed){

            printf ("----------------------------- Button was pressed ---------------------------\n");

            // Add noise
            int addnoise = 1;

            if (addnoise){

                char noiseName[130] = "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser_audiov2/tiny_denoiser/res/meeting_ch01_mancrop1.wav";

                if (eval_input == "0"){
                    input_mic(1, 1, 1); // save, free, noise
                }
                else if (eval_input == "1"){

                    // input_mic(1, 1, 1); // save, free, noise // Forcefully recording noise from recording
                    input_wav(1, 1, noiseName, 1); // save, free, NoiseName, noise
                }
            }

            printf ("----------------------------- Started updating ---------------------------\n");
            // evaluate before training
            evaluate_tinytest();

            // train model with noisy data
            train_wavsrc();

            // evaluate improvement
            evaluate_tinytest();

            printf ("----------------------------- Finished updating ---------------------------\n");
            return; // breaking loop early
        }


        PRINTF ("----------------------------- Round completed ---------------------------\n");


        // // block until next input audio frame is ready
        // #ifdef  AUDIO_EVK
        //     pi_gpio_pin_write(gpio_pin_o, 0);
        // #endif

        // TODO: Trigger inference every 250 ms
        pi_time_wait_us(250000); // microseconds

    }

    printf ("----------------------------- Application completed ---------------------------\n");

    pmsis_exit(0);
    return 0;
}

int main()
{
    PRINTF("\n\n\t *** Application ***\n\n");

    #define __XSTR(__s) __STR(__s)
    #define __STR(__s) #__s
    WavName = __XSTR(WAV_FILE); 
    mfcc = __XSTR(MFCC);
    eval_input = __XSTR(EVAL);
    appl_input = __XSTR(APPL);

    return application();
}
