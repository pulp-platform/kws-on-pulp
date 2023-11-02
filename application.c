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

// PMSIS SFU
#include "sfu_pmsis_runtime.h"
#define NB_BUF_IN_RING 2
#define FREQ_PDM_BIT (3072000)
#define FREQ_PCM (48000)
#define SAI_RX (1)
#define SAI_TX (0)


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

// measurement
// unsigned int GPIOs = PI_GPIO_A89;
// unsigned int GPIOs = 89;
// #define WRITE_GPIO(x) pi_gpio_pin_write(GPIOs,x)
pi_gpio_e gpio_pin_measurement;


/* 
     global variables
*/
struct pi_device DefaultRam; 
struct pi_device* ram = &DefaultRam;

// #ifdef AUDIO_EVK
//     // GPIO defines
//     pi_gpio_e gpio_pin_o; /* PI_GPIO_A02-PI_GPIO_A05 */
//     int val_gpio;
// #endif




//static struct pi_default_flash_conf flash_conf;
static pi_fs_file_t * file[1];
static struct pi_device fs;
static struct pi_device flash;

// Load args
char *WavName = NULL;
char *mfcc = NULL;
char *noise_eval_input = NULL;
char *uttr_eval_input = NULL;
char *appl_input = NULL;

// Arrays handling data movement
short int *inWav;
MFCC_IN_TYPE *MfccInSig;
MFCC_IN_TYPE *MfccInSig_prev;
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
void * BufferOutList;

// PMSIS SFU
// SFU
static pi_sfu_graph_t *sfu_graph;
static uint8_t sfu_input_id;
static uint8_t sfu_output_id;

// SAI used for receiving and sending PDM
static pi_device_t sai_dev_rx;
static pi_device_t sai_dev_tx;

// Audio buffers
static pi_sfu_buffer_t sfu_out_buffers[NB_BUF_IN_RING]; // Buffers for SFU(MEM_OUT) -> L2 transfers
static pi_sfu_buffer_t sfu_in_buffers[NB_BUF_IN_RING]; // BUffers for L2 -> SFU(MEM_IN) transfer
static int sfu_out_buffer_idx = 0;
static int sfu_out_buffer_cnt = 0;
static int sfu_in_buffer_idx = 0;
static int sfu_in_buffer_cnt = 0;

static pi_evt_t sfu_out_task;
static pi_evt_t sfu_in_task;

static pi_sfu_mem_port_t * memin_port;
static pi_sfu_mem_port_t * memout_port;

static int sfu_buffer_filled = 0;

int noise_seconds = 1;

static const pi_gpio_e gpio_boot_pin_1 = PAD_GPIO_UPB;

// Global declaration 
struct pi_device cluster_dev;
struct pi_cluster_conf cl_conf;
struct pi_cluster_task cl_task;

static pi_event_t inference_task;

#define tinytestsize 10
static MFCC_IN_TYPE *MfccInSig_buff[tinytestsize];


char yes[490];
char no[490]; 
char up[490];
char down[490];
char left[490];
char right[490];
char on[490];
char off[490];
char stop[490];
char go[490];


// Configure PDM RX interface
static int configure_pdm()
{
    int res = 0;
    int err;

    pi_pad_function_set(SAI_SCK(SAI_RX), PI_PAD_FUNC0);
    pi_pad_function_set(SAI_WS(SAI_RX),  PI_PAD_FUNC0);
    pi_pad_function_set(SAI_SDI(SAI_RX), PI_PAD_FUNC0);
    pi_pad_function_set(SAI_SDO(SAI_RX), PI_PAD_FUNC0);

    struct pi_i2s_conf i2s_conf;
    pi_i2s_conf_init(&i2s_conf);
    i2s_conf.options = PI_I2S_OPT_INT_CLK | PI_I2S_OPT_REF_CLK_FAST;
    i2s_conf.frame_clk_freq = FREQ_PDM_BIT;
    i2s_conf.itf = SAI_RX;
    i2s_conf.mode = PI_I2S_MODE_PDM;
    i2s_conf.pdm_direction = 0b11;
    i2s_conf.pdm_diff = 0b00;

    pi_open_from_conf(&sai_dev_rx, &i2s_conf);
    if (pi_i2s_open(&sai_dev_rx))
    {
        printf("Failed to open PDM Rx\n");
        res = -1;
    }

    // Connect to SFU
    if (res == 0)
    {
        pi_sfu_pdm_itf_id_t itf_id =
        {
            SAI_RX,
            2,
            0
        };
        err = pi_sfu_graph_pdm_bind(sfu_graph, SFU_Name(Graph, PdmIn1), &itf_id);
        if (err != 0)
            res = -1;
    }

    return res;
}

// Configure I2S Tx interface
static int configure_i2s()
{
    int err;

    pi_pad_function_set(SAI_SCK(SAI_TX), PI_PAD_FUNC0);
    pi_pad_function_set(SAI_WS(SAI_TX),  PI_PAD_FUNC0);
    pi_pad_function_set(SAI_SDI(SAI_TX), PI_PAD_FUNC0);
    pi_pad_function_set(SAI_SDO(SAI_TX), PI_PAD_FUNC0);

    int32_t stream_ch;
    struct pi_i2s_conf i2s_conf;
    pi_i2s_conf_init(&i2s_conf);

    i2s_conf.itf = SAI_TX;
    i2s_conf.frame_clk_freq = FREQ_PCM;
    i2s_conf.slot_width = 32;
    i2s_conf.channels = 1;

    pi_open_from_conf(&sai_dev_tx, &i2s_conf);
    if (pi_i2s_open(&sai_dev_tx))
        printf("Failed to open SAI %d in I2S mode\n", SAI_TX);

    // Tx slot
    pi_sfu_i2s_itf_id_t itf_id = {SAI_TX, 1};
    err = pi_sfu_graph_i2s_bind(sfu_graph, SFU_Name(Graph, PcmOut1), &itf_id, &stream_ch);
    if (err != 0)
    {
        printf("Unable to bind I2S(SAI: %d, Ch: %d) to SFU STREAM block\n", SAI_TX, 0);
        return -1;
    }

    struct pi_i2s_channel_conf i2s_slot_conf;
    pi_i2s_channel_conf_init(&i2s_slot_conf);
    i2s_slot_conf.options = PI_I2S_OPT_IS_TX | PI_I2S_OPT_ENABLED;
    i2s_slot_conf.word_size = 32;
    i2s_slot_conf.format = PI_I2S_CH_FMT_DATA_ORDER_MSB | PI_I2S_CH_FMT_DATA_ALIGN_LEFT | PI_I2S_CH_FMT_DATA_SIGN_NO_EXTEND;
    i2s_slot_conf.stream_id = stream_ch;

    if (pi_i2s_channel_conf_set(&sai_dev_tx, 0, &i2s_slot_conf))
        return -1;

    return 0;
}

// PMSIS SFU
static void handle_out_transfer_end(void *arg)
{
    // unsigned int t1 = pi_perf_fc_read(PI_PERF_CYCLES);
    // printf("handle_out_transfer_end(): t1=%d, cnt=%d, idx=%d\n",
    //     t1, sfu_out_buffer_cnt, sfu_out_buffer_idx);

    pi_sfu_enqueue(sfu_graph, memout_port, &sfu_out_buffers[sfu_out_buffer_idx]);

    /*
     * Buffer received from MEM_OUT.
     * Here we just do a simple copy to the MEM_IN buffer that is not currently being transferred.
     */
    int in_idx = sfu_in_buffer_idx ^ 1;
    int out_idx = sfu_out_buffer_idx;
        
    int start;
    int elapsed;

    memcpy(BufferInList+sfu_out_buffer_cnt*DOUBLE_BUFF_SIZE*sizeof(int32_t), sfu_out_buffers[out_idx].data, DOUBLE_BUFF_SIZE*sizeof(int32_t));


    sfu_out_buffer_cnt++;

    // elapsed = gap_fc_readhwtimer() - start;
    // printf("memcpy time: %d\n", elapsed);

    if (sfu_out_buffer_cnt*DOUBLE_BUFF_SIZE*sizeof(int32_t) >= BUFF_SIZE) { // one seccond is added to the main buffer
        sfu_buffer_filled = 1;
    }

    if (sfu_out_buffer_cnt == BUFF_SIZE/DOUBLE_BUFF_SIZE/sizeof(int32_t)){
        sfu_out_buffer_cnt = 0;
    }

    if (sfu_buffer_filled){
        pi_evt_push(&inference_task);
    }

    // pi_evt_push(&inference_task);

    sfu_out_buffer_idx ^= 1;
}


static void handle_in_transfer_end(void *arg)
{
    // unsigned int t1 = pi_perf_fc_read(PI_PERF_CYCLES);
    // printf("handle_in_transfer_end(): t1=%d, cnt=%d, idx=%d\n",
    //     t1, sfu_in_buffer_cnt, sfu_in_buffer_idx);

    pi_sfu_enqueue(sfu_graph, memin_port, &sfu_in_buffers[sfu_in_buffer_idx]);

    sfu_in_buffer_cnt++;
    sfu_in_buffer_idx ^= 1;
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

void input_mic_buffer(int save, int free, int noise){

    int err;

    // Open SFU with default frequency
    pi_sfu_conf_t conf = { .sfu_frequency=0 };
    if (pi_sfu_open(&conf))
        printf("SFU device open failed\n");
    printf("SFU activated\n");

    sfu_graph = pi_sfu_graph_open(&SFU_RTD(Graph));
    if (sfu_graph == NULL)
        printf("SFU graph open failed\n");
    printf("Graph opened\n");

    // Allocate IO buffers
    for (int i = 0; i < NB_BUF_IN_RING; i++)
    {
        void *data_out = pi_l2_malloc(DOUBLE_BUFF_SIZE * sizeof(int));
        if (data_out == NULL) return -1;
        pi_sfu_buffer_init(&sfu_out_buffers[i], data_out, DOUBLE_BUFF_SIZE, sizeof(int));

    }

    // Configure interfaces
    err = configure_pdm();
    if (err != 0)
        printf("PDM interface init failed\n");
    printf("PDM Rx interface configured\n");

    // err = configure_i2s();
    // if (err != 0)
    //     printf("I2S interface init failed\n");
    // printf("I2S Tx interface configured\n");

    // Get port refs
    // memin_port = pi_sfu_mem_port_get(sfu_graph, SFU_Name(Graph, MemIn1));
    // if (memin_port == NULL)
        // printf("Failed to get memin_port references\n");
    memout_port = pi_sfu_mem_port_get(sfu_graph, SFU_Name(Graph, MemOut1));
    if (memout_port == NULL)
        printf("Failed to get memout_port references\n");

     // Prepare buffer transfer callbacks
    pi_evt_callback_irq_init(&sfu_out_task, handle_out_transfer_end, NULL);
    // pi_evt_callback_irq_init(&sfu_in_task, handle_in_transfer_end, NULL);

    // Enqueue first two buffers on each side
    for (int i = 0; i < NB_BUF_IN_RING; i++)
    {
        sfu_out_buffers[i].task = &sfu_out_task;
        pi_sfu_enqueue(sfu_graph, memout_port, &sfu_out_buffers[i]);
    }
    // for (int i = 0; i < NB_BUF_IN_RING; i++)
    // {
    //     sfu_in_buffers[i].task = &sfu_in_task;
    //     pi_sfu_enqueue(sfu_graph, memin_port, &sfu_in_buffers[i]);
    // }

    pi_sfu_graph_load(sfu_graph);

    pi_i2s_ioctl(&sai_dev_rx, PI_I2S_IOCTL_START, NULL);
    // pi_i2s_ioctl(&sai_dev_tx, PI_I2S_IOCTL_START, NULL);

    // pi_time_wait_us(2000000);
    // pi_time_wait_us(100000);

    // pi_i2s_ioctl(&sai_dev_rx, PI_I2S_IOCTL_STOP, NULL);
    // // pi_i2s_ioctl(&sai_dev_tx, PI_I2S_IOCTL_STOP, NULL);


    // pi_sfu_graph_unload(sfu_graph);

    // pi_sfu_graph_close(sfu_graph);


    // printf("Finish rec!\n");

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
                RecordedNoise[i] = (MFCC_IN_TYPE) gap_fcip(((int) inWav[i]), 15);
                // READ TEXT
                // RecordedNoise[i] = (MFCC_IN_TYPE) gap_fcip(((int) noisemeeting[i]), 15);
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
                MfccInSig[i] = (MFCC_IN_TYPE) gap_fcip(((int) inWav[i]), 15);
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

    if (free){
        if (noise){
            pi_l2_free(inWav, noise_seconds*AUDIO_BUFFER_SIZE * sizeof(short));
        }
        else{
            pi_l2_free(inWav, AUDIO_BUFFER_SIZE * sizeof(short));
        }
    }
    

}

void compute_mfcc(){
    /******
        Compute the MFCC
    ******/
    out_feat = (OUT_TYPE *) pi_l2_malloc(49 * 10 * 4 * sizeof(OUT_TYPE));    


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

    pi_l2_free(MfccInSig, noise_seconds * AUDIO_BUFFER_SIZE * sizeof (MFCC_IN_TYPE));

}


void evaluate_tinytest(int pre){

    // int tinytestsize = 10;
    // int tinytestsize = 1;    

    if (pre == 0){
        for (int tinytestidx = 0; tinytestidx < tinytestsize; tinytestidx++){
            MfccInSig_buff[tinytestidx] = (MFCC_IN_TYPE *) pi_l2_malloc(AUDIO_BUFFER_SIZE * sizeof(MFCC_IN_TYPE));
        }
    }


    for (int tinytestidx = 0; tinytestidx < tinytestsize; tinytestidx++){ // only non-unknown

        printf ("-----------------------------Loop evaluation (itteration %i)-------------------------\n", tinytestidx);

        int addnoise;
        int save; 

        if (uttr_eval_input == "1") {
            // Read from WAV
            printf("Tested input: %s\n", tinytestutter[tinytestidx]);
            input_wav(1, 1, tinytestutter[tinytestidx], 0); // save, free, noise
            addnoise = 1;
        }
        else if (uttr_eval_input == "0") {

            // Read from MIC
            printf("Preparing reading!\n");
            MfccInSig = NULL;
            MfccInSig = (MFCC_IN_TYPE *) pi_l2_malloc(AUDIO_BUFFER_SIZE * sizeof(MFCC_IN_TYPE));

            if (MfccInSig == NULL){
                printf("Failed allocating MfccInSig in evaluate_tinytest.\n");
                pmsis_exit(-1);
            }            
              

            if (pre == 0) {

                // clean up buffer
                pi_time_wait_us(1000000);

                // read recording
                int mfccidx = 0;
                pi_evt_wait(&inference_task);
                printf("Filling input buffer...\n");  
                for (int i = 0; i < BUFF_SIZE; i+=3){
                    MfccInSig[mfccidx] = (MFCC_IN_TYPE) (((float)((int32_t *)BufferInList)[i]) / (float)(1<<31 - 1));
                    MfccInSig_buff[tinytestidx][mfccidx] = MfccInSig[mfccidx];
                    mfccidx++;
                }
            }
            else{
                for (int i = 0; i < AUDIO_BUFFER_SIZE; i++){
                    MfccInSig[i] = MfccInSig_buff[tinytestidx][i];
                }
            }

            addnoise = 0;

// #ifdef AUDIO_EVK
//             pi_gpio_pin_write(gpio_pin_o, 1);
// #endif
        }

        int noisesamplestart = 0;
        if (addnoise == 1 && noise_eval_input == "1" && uttr_eval_input == "1") {
            noisesamplestart = 0; // TODO: random sample between (0, len(wav)-16000)
            
            for (int samplepos = 0; samplepos < AUDIO_BUFFER_SIZE; samplepos++){
                MfccInSig[samplepos] = MfccInSig[samplepos] + 1*RecordedNoise[noisesamplestart+samplepos];
            }
        }

        save = 0;

        if (save){
            MfccInSig_int16 = (int16_t *) pi_l2_malloc (sizeof(int16_t) * AUDIO_BUFFER_SIZE);
            RecordedNoise_int16 = (int16_t *) pi_l2_malloc (sizeof(int16_t) * AUDIO_BUFFER_SIZE);
            for (int i = 0; i < AUDIO_BUFFER_SIZE; i++){
                MfccInSig_int16[i] = (int16_t) (MfccInSig[i] * (1<<15));
                RecordedNoise_int16[i] = (int16_t) (RecordedNoise[i] * (1<<15));
            }

            // Save the noise-augmented recording
            dump_wav_open("utter.wav", 16, 16000, 1, sizeof(int16_t) * AUDIO_BUFFER_SIZE);
            dump_wav_write(MfccInSig_int16, sizeof(int16_t) * AUDIO_BUFFER_SIZE);
            dump_wav_close();

            PRINTF("Writing wav file to utter.wav completed successfully\n");
            for (int samplepos = 0; samplepos < AUDIO_BUFFER_SIZE; samplepos++){
                MfccInSig_int16[samplepos] = (int16_t)(MfccInSig[samplepos] * (1<<15)) + 1*RecordedNoise_int16[noisesamplestart+samplepos];
            }
            // Save the noise-augmented recording
            dump_wav_open("utter_noise.wav", 16, 16000, 1, sizeof(int16_t) * AUDIO_BUFFER_SIZE);
            dump_wav_write(MfccInSig_int16, sizeof(int16_t) * AUDIO_BUFFER_SIZE);
            dump_wav_close();
            pi_l2_free(MfccInSig_int16, sizeof(int16_t) * AUDIO_BUFFER_SIZE);
            pi_l2_free(RecordedNoise_int16, sizeof(int16_t) * AUDIO_BUFFER_SIZE);

            PRINTF("Writing wav file to utter_noise.wav completed successfully\n");
        }

        compute_mfcc();

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

        for (int k = 0; k < 490; k++){
            // Data saving to elude re-recording the evaluation samples. TODO: organize workflow
            if (uttr_eval_input == "0") {
                if (pre == 0){
                    switch (tinytestidx) {
                        case 0:
                            yes[k] = feat_char[k];
                            break;
                        case 1: 
                            no[k] = feat_char[k];
                            break;
                        case 2:
                            up[k] = feat_char[k];
                            break;
                        case 3:
                            down[k] = feat_char[k];
                            break;
                        case 4:
                            left[k] = feat_char[k];
                            break;
                        case 5:
                            right[k] = feat_char[k];
                            break;
                        case 6:
                            on[k] = feat_char[k];
                            break;
                        case 7:
                            off[k] = feat_char[k];
                            break;
                        case 8:
                            stop[k] = feat_char[k];
                            break;
                        case 9:
                            go[k] = feat_char[k];
                            break;
                    } 
                }
                else{
                    switch (tinytestidx) {
                        case 0:
                            feat_char[k] = yes[k];
                            break;
                        case 1: 
                            feat_char[k] = no[k];
                            break;
                        case 2:
                            feat_char[k] = up[k];
                            break;
                        case 3:
                            feat_char[k] = down[k];
                            break;
                        case 4:
                            feat_char[k] = left[k];
                            break;
                        case 5:
                            feat_char[k] = right[k];
                            break;
                        case 6:
                            feat_char[k] = on[k];
                            break;
                        case 7:
                            feat_char[k] = off[k];
                            break;
                        case 8:
                            feat_char[k] = stop[k];
                            break;
                        case 9:
                            feat_char[k] = go[k];
                            break;
                    } 
                }
            }
        }

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

        
    // #ifdef  AUDIO_EVK
    //     // block until next input audio frame is ready
    //     pi_gpio_pin_write(gpio_pin_o, 0);
    // #endif
    }

    if (pre == 1){
        for (int tinytestidx = 0; tinytestidx < tinytestsize; tinytestidx++){
            pi_l2_free(MfccInSig_buff[tinytestidx], AUDIO_BUFFER_SIZE * sizeof(MFCC_IN_TYPE));
        }
    }
}


void train_wavsrc(){

    int sampleidx;
    int classidx;
    int samplestart;


    // int nepochs = 10; // GVSOC - DEMO (mem leak?)
    int nepochs = 1; // BOARD - QUICK DEMO

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

        compute_mfcc();

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

        for (int i = 0; i < 490; i++){
            if (mfcc == "1"){
                ((uint8_t *)l2_buffer)[i] = L2_input_h[i]; // Precomputed MFCC
            }
            else {
                ((uint8_t *)l2_buffer)[i] = feat_char[i]; // Online computed MFCC
            }
        }
        pi_l2_free(feat_char, 49 * 10 * sizeof(char));

        PRINTF ("********** Run inferecene **********\n");
        // Extract backbone features
        network_run(l2_buffer, L2_MEMORY_SIZE, l2_buffer, &L2_FC_weights_int8, 0); // L2_input_h extra-arg for L2-only


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

// #ifdef AUDIO_EVK
//     /****
//         Configure GPIO Output.
//     ****/

//     //struct pi_gpio_conf gpio_conf = {0};
//     gpio_pin_o = PI_GPIO_A89; /* PI_GPIO_A02-PI_GPIO_A05 */
//     pi_gpio_flags_e flags = PI_GPIO_OUTPUT;
//     pi_gpio_pin_configure(gpio_pin_o, flags);
// #endif

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


    // Measurement preparation

    gpio_pin_measurement = PI_GPIO_A89; /* PI_GPIO_A02-PI_GPIO_A05 */
    pi_gpio_flags_e flags = PI_GPIO_OUTPUT;
    pi_pad_function_set(gpio_pin_measurement, 1);
    pi_gpio_pin_configure(gpio_pin_measurement, flags);
    pi_gpio_pin_write(gpio_pin_measurement, 0);
    pi_gpio_pin_write(gpio_pin_measurement, 0);



    PRINTF ("----------------------------- Initializing backbone ---------------------------\n");


    // Measurement start
    pi_gpio_pin_write(gpio_pin_measurement, 1);

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

    pi_gpio_pin_write(gpio_pin_measurement, 0);
    pmsis_exit(0);
    return;

    BufferInList = (void*) pi_l2_malloc(BUFF_SIZE);
    if (BufferInList == NULL) return -1;



    int button_was_pressed = 0;


    printf ("----------------------------- Starting application ---------------------------\n");

    pi_evt_sig_init(&inference_task);

    input_mic_buffer(1, 1, 0);

    int iterations = 0;
    int sfu_out_buffer_cnt_prev = 0;
    int sfu_out_buffer_cnt_curr = 0;

    MfccInSig_prev = (MFCC_IN_TYPE *) pi_l2_malloc(1 * AUDIO_BUFFER_SIZE * sizeof (MFCC_IN_TYPE));
    int len = 0;
    int upperlim = 0;
    int lowerlim = 0;

    

    while (1){

        if (appl_input == "0"){

            PRINTF ("----------------------------- Start acquisition ---------------------------\n");        

            MfccInSig = (MFCC_IN_TYPE *) pi_l2_malloc(1 * AUDIO_BUFFER_SIZE * sizeof (MFCC_IN_TYPE));

            for(int i=0;i<AUDIO_BUFFER_SIZE;i++){ 
                MfccInSig[i] = MfccInSig_prev[i];
            }  
            pi_evt_wait(&inference_task);

            sfu_out_buffer_cnt_curr = sfu_out_buffer_cnt;

            if (sfu_out_buffer_cnt_curr > sfu_out_buffer_cnt_prev){
                len = sfu_out_buffer_cnt_curr - sfu_out_buffer_cnt_prev;
                upperlim = sfu_out_buffer_cnt_curr * DOUBLE_BUFF_SIZE;
                lowerlim = 0;
            }
            else{
                len = (48 - sfu_out_buffer_cnt_prev) + sfu_out_buffer_cnt_curr;
                upperlim = BUFF_SIZE/sizeof(int32_t);
                lowerlim = sfu_out_buffer_cnt_curr * DOUBLE_BUFF_SIZE;
            }

            int outidx = 0;
            for (int i = sfu_out_buffer_cnt_prev*DOUBLE_BUFF_SIZE; i < upperlim; i+=3){
                // using MfccInSig_prev as buffer
                MfccInSig_prev[outidx] = (MFCC_IN_TYPE) (((float)((int32_t *)BufferInList)[i]) / (float)(1<<31 - 1));
                outidx++;
            }
            for (int i = 0; i < lowerlim; i+=3){
                // using MfccInSig_prev as buffer
                MfccInSig_prev[outidx] = (MFCC_IN_TYPE) (((float)((int32_t *)BufferInList)[i]) / (float)(1<<31 - 1));
                outidx++;
            }

            int j = 0;
            for (int i = DOUBLE_BUFF_SIZE*(len)/3; i < AUDIO_BUFFER_SIZE; i++){
                MfccInSig[j] = MfccInSig[i];    
                j++;
            }
            
            int k = 0;
            for (int i = AUDIO_BUFFER_SIZE - DOUBLE_BUFF_SIZE*(len)/3; i < AUDIO_BUFFER_SIZE; i++){
                MfccInSig[i] = MfccInSig_prev[k];
                k++;
            }

            for (int i = 0; i < AUDIO_BUFFER_SIZE; i++){
                MfccInSig_prev[i] = MfccInSig[i];
            }
            sfu_out_buffer_cnt_prev = sfu_out_buffer_cnt_curr;
            
            MfccInSig_int16 = (int16_t *) pi_l2_malloc(sizeof(int16_t) * AUDIO_BUFFER_SIZE);
            for(int i=0;i<AUDIO_BUFFER_SIZE;i++){
                MfccInSig_int16[i] = (int16_t) (MfccInSig[i] * (1<<15));
            }

        }
        else if (appl_input == "1"){
            // printf ("Reading wav...\n");
            char utterName[130] = "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser_audiov2/tiny_denoiser/res/meeting_ch01_mancrop1.wav";
            input_wav(1, 1, utterName, 0); // save, free, utterName, noise
        }

        #ifdef PERF
        gap_fc_starttimer();
        gap_fc_resethwtimer();
        int start_timer_2 = gap_fc_readhwtimer();
        #endif

        PRINTF("***************************** Computing MFCC **************************\n");
        compute_mfcc();

        #ifdef PERF
        int elapsed_timer_2 = gap_fc_readhwtimer() - start_timer_2;
        printf("Compute mfcc: %d cycles\n", elapsed_timer_2);
        #endif

        for (int i = 0; i < 5; i++){
            PRINTF("out_feat[%i] = %f, ", i, out_feat[i]);
        }
        PRINTF("\n");
        
        #ifdef PERF
        gap_fc_starttimer();
        gap_fc_resethwtimer();
        start_timer_2 = gap_fc_readhwtimer();    
        #endif    

        PRINTF("***************************** Rescaling data **************************\n");
        feat_char = (char*) pi_l2_malloc(49 * 10 * sizeof(char));
        // Rescale data
        int k = 0;
        for (int i = 0; i < 1960;i++){                
            
            // feat_char[k] = (char) ((int) floor(out_feat[i] * pow(2, -1) * sqrt(0.05)) + 128);
            feat_char[k] = (char) ((int) floor(out_feat[i] * 0.1118) + 128);

            // Select 10 MFCC per window
            if (i == 40*(k/10) + 9){
                i = 40*(k/10) + 39;
            }
            k++;
        } 

        pi_l2_free(out_feat, 49*10*4*sizeof(OUT_TYPE));

        #ifdef PERF
        elapsed_timer_2 = gap_fc_readhwtimer() - start_timer_2;
        printf("Scale mfcc: %d cycles\n", elapsed_timer_2);
        #endif

        #ifdef PERF
        gap_fc_starttimer();
        gap_fc_resethwtimer();
        int start_timer_3 = gap_fc_readhwtimer();    
        #endif    

        PRINTF("***************************** Move data in L2 **************************\n");
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

        #ifdef PERF
        int elapsed_timer_3 = gap_fc_readhwtimer() - start_timer_3;
        printf("Convert mfcc: %d cycles\n", elapsed_timer_3);
        #endif

        // Extract backbone features
        void *dump; // dump to copy FC weights, won't be used; TODO: Parametrize DORY


        #ifdef PERF
        gap_fc_starttimer();
        gap_fc_resethwtimer();
        int start_timer_4 = gap_fc_readhwtimer();        
        #endif

        PRINTF("***************************** Backbone inference **************************\n");
        network_run(l2_buffer, L2_MEMORY_SIZE, l2_buffer, &dump, 0); // L2_input_h extra-arg for L2-only

        #ifdef PERF
        int elapsed_timer_4 = gap_fc_readhwtimer() - start_timer_4;
        printf("Backbone inference: %d cycles\n", elapsed_timer_4);
        #endif

        for (int i=0; i < 64; i++){
            PRINTF("%i, ", ((uint8_t *) l2_buffer)[i]);
        }
        PRINTF("\n");

        

        #ifdef PERF
        gap_fc_starttimer();
        gap_fc_resethwtimer();
        int start_timer_5 = gap_fc_readhwtimer();    
        #endif    


        PRINTF("***************************** Classifier inference **************************\n");

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

        PRINTF("***************************** Process complete **************************\n");

        #ifdef PERF
        int elapsed_timer_5 = gap_fc_readhwtimer() - start_timer_5;
        printf("FC inference: %d cycles\n", elapsed_timer_5);
        #endif

        #ifdef PERF
            dump_wav_open("recording_utterance.wav", 16, 16000, 1, sizeof(int16_t) * AUDIO_BUFFER_SIZE);
            dump_wav_write(MfccInSig_int16, sizeof(int16_t) * AUDIO_BUFFER_SIZE);
            dump_wav_close();
        #endif

        pi_l2_free(MfccInSig_int16, sizeof(int16_t) * AUDIO_BUFFER_SIZE);

        
        // #ifdef  AUDIO_EVK
        //     // block until next input audio frame is ready
        //     pi_gpio_pin_write(gpio_pin_o, 0);
        // #endif

        button_was_pressed = 0;
        button_was_pressed = read_button();

        if (button_was_pressed){

            printf ("----------------------------- Button press, begin ODDA ---------------------------\n");

            // Add noise
            int addnoise = 1;

            if (addnoise){

                if (noise_eval_input == "0"){
                    
                    // wait 1s (for the previous non-noise content to be cleaned)
                    pi_time_wait_us (1000000);

                    RecordedNoise = NULL;
                    RecordedNoise = (MFCC_IN_TYPE *) pi_l2_malloc(noise_seconds*AUDIO_BUFFER_SIZE * sizeof(MFCC_IN_TYPE));
                    if (RecordedNoise == NULL){
                        printf("Failed allocating RecordedNoise.\n");
                        pmsis_exit(-1);
                    }

                    // copy content from BufferInList
                    int noiseidx = 0;
                    pi_evt_wait(&inference_task);
                    for (int i = 0; i < BUFF_SIZE; i+=3){
                        RecordedNoise[noiseidx] = (MFCC_IN_TYPE) (((float)((int32_t *)BufferInList)[i]) / (float)(1<<31 - 1));
                        noiseidx++;
                    }
                }
                else if (noise_eval_input == "1"){
                    char noiseName[130] = "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser_audiov2/tiny_denoiser/res/meeting_ch01_mancrop1.wav";
                    // input_mic(1, 1, 1); // save, free, noise // Forcefully recording noise from recording
                    input_wav(1, 1, noiseName, 1); // save, free, NoiseName, noise
                }
            }

            // evaluate before training
            evaluate_tinytest(0);

            // train model with noisy data
            train_wavsrc();

            // evaluate improvement
            evaluate_tinytest(1);

            if (addnoise == 1 && noise_eval_input == "0"){
               pi_l2_free(RecordedNoise, noise_seconds*AUDIO_BUFFER_SIZE * sizeof(MFCC_IN_TYPE));
            }

            printf ("----------------------------- ODDA complete ---------------------------\n");

            // pmsis_exit(0);
            // return; // breaking loop early

        }

        printf ("----------------------------- Finished measurement ---------------------------\n");

        pmsis_exit(0);
        return;


        PRINTF ("----------------------------- Round completed ---------------------------\n");


        // // block until next input audio frame is ready
        // #ifdef  AUDIO_EVK
        //     pi_gpio_pin_write(gpio_pin_o, 0);
        // #endif

        // TODO: Trigger inference every 250 ms
        // pi_time_wait_us(250); // microseconds





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
    noise_eval_input = __XSTR(NOISE_EVAL);
    uttr_eval_input = __XSTR(UTTR_EVAL);
    appl_input = __XSTR(APPL);

    return application();
}




// Example :)
// Everytime a buffer is finished, you re-enqueue itself. THen you pingpong the buffer out index
// KConfig/Menuconfig for SFU for example
// Memout comes from the PDM, Memin sends to the I2S
// when buffer received: pi_evt_push (to send a new event) -> in while(1) you wait for the event (pi_even_wait)
// pi_even_init() when I finished my comp task
