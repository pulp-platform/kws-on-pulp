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


#include "application.h"

#define DATA_TYPE 2
#if (DATA_TYPE==2)
typedef float16 MFCC_IN_TYPE;
typedef float16 OUT_TYPE;
#elif (DATA_TYPE==3)
typedef float MFCC_IN_TYPE;
typedef float OUT_TYPE;
#else
typedef short int OUT_TYPE;
typedef short int MFCC_IN_TYPE;
#endif

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

#define WAVRAM 110*2*16000

// measurement
pi_gpio_e gpio_pin_measurement;
unsigned int gpio_pin_measurement_id = 89;


/* 
     global variables
*/
struct pi_device DefaultRam; 
struct pi_device* ram = &DefaultRam;

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
    if (sfu_out_buffer_cnt*DOUBLE_BUFF_SIZE*sizeof(int32_t) >= BUFF_SIZE) { // one seccond is added to the main buffer
        sfu_buffer_filled = 1;
    }
    if (sfu_out_buffer_cnt == BUFF_SIZE/DOUBLE_BUFF_SIZE/sizeof(int32_t)){
        sfu_out_buffer_cnt = 0;
    }
    if (sfu_buffer_filled){
        pi_evt_push(&inference_task);
    }
    sfu_out_buffer_idx ^= 1;
}

static void handle_in_transfer_end(void *arg)
{
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

    memout_port = pi_sfu_mem_port_get(sfu_graph, SFU_Name(Graph, MemOut1));
    if (memout_port == NULL)
        printf("Failed to get memout_port references\n");

    // Prepare buffer transfer callbacks
    pi_evt_callback_irq_init(&sfu_out_task, handle_out_transfer_end, NULL);
    
    // Enqueue first two buffers on each side
    for (int i = 0; i < NB_BUF_IN_RING; i++)
    {
        sfu_out_buffers[i].task = &sfu_out_task;
        pi_sfu_enqueue(sfu_graph, memout_port, &sfu_out_buffers[i]);
    }

    pi_sfu_graph_load(sfu_graph);
    pi_i2s_ioctl(&sai_dev_rx, PI_I2S_IOCTL_START, NULL);

    // pi_time_wait_us(2000000);
    // pi_time_wait_us(100000);

    // pi_i2s_ioctl(&sai_dev_rx, PI_I2S_IOCTL_STOP, NULL);
    // // pi_i2s_ioctl(&sai_dev_tx, PI_I2S_IOCTL_STOP, NULL);

    printf("Finish rec!\n");

}



int application(){
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