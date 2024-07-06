// Definitions

#ifndef __GLOBALS_H__
#define __GLOBALS_H__

#define SCALE_IN denoiser_dns_Input_1_OUT_SCALE
#define SCALE_OUT denoiser_dns_Output_1_OUT_SCALE

#define WAVRAM 110*2*16000
#define DOUBLE_BUFF_SIZE (1000)
// #define BUFF_SIZE (48*1024*4)
#define BUFF_SIZE (48*1000*4)
#define AUDIO_BUFFER_SIZE 16000 // (32*1024)
#define CHUNK_NUM (8)

// SAI Setup
#define STRUCT_DELAY (1)
#define SAI1         (1)
#define SAI_ID               (48)
#define SAI_SCK(itf)         (48+(itf*4)+0)
#define SAI_WS(itf)          (48+(itf*4)+1)
#define SAI_SDI(itf)         (48+(itf*4)+2)
#define SAI_SDO(itf)         (48+(itf*4)+3)

// #define L2_MEMORY_SIZE MODEL_L2_MEMORY // TODO: Read from CMake
#define L2_MEMORY_SIZE 150000 // TODO: Read from CMake
// #define L2_MEMORY_SIZE 1404000

#define DATA_TYPE 2 // TODO: Understand why this works
#if (DATA_TYPE==2)
typedef float16 MFCC_IN_TYPE;
typedef float16 OUT_TYPE;
#elif (DATA_TYPE==3)
typedef float MFCC_IN_TYPE;
typedef float OUT_TYPE;
#else
typedef short int OUT_TYPE;  // Save MFCCs works 
typedef short int MFCC_IN_TYPE; // Save MFCCs works
#endif

#define NORM 6

// User Push Button
#define PAD_GPIO_UPB    (PI_PAD_086)

#ifdef SILENT
# define PRINTF(...) ((void) 0)
#else
# define PRINTF printf
#endif  /* DEBUG */

// Preprocessing defines
#include "MFCC_params.h"
#define N_MFCC_MELS 10
#define N_MFCC_WINS 49

#define N_CLASSES 12
#define N_TINYTEST 10


#define NOISE_LEN_S 1

enum source {
  ONLINE,
  OFFLINE
}; 

// PMSIS SFU
#include "sfu_pmsis_runtime.h"
#include "Graph_L2_Descr.h" // pdm_in_test

#define NB_BUF_IN_RING 2
#define FREQ_PDM_BIT (3072000)
#define FREQ_PCM (48000)
#define SAI_RX (1)
#define SAI_TX (0)


// Global declaration for cluster setup
struct pi_device cluster_dev;
struct pi_cluster_conf cl_conf;
struct pi_cluster_task cl_task;

// Triggering event for recording
pi_event_t inference_task;

// Global declaration for microphone recording
/* Global variables for microphone recording */
void * BufferInList;
// PMSIS SFU
pi_sfu_graph_t *sfu_graph;
// SAI used for receiving and sending PDM
pi_device_t sai_dev_rx;
// Audio buffers
pi_sfu_buffer_t sfu_out_buffers[NB_BUF_IN_RING]; // Buffers for SFU(MEM_OUT) -> L2 transfers
int sfu_out_buffer_idx;
int sfu_out_buffer_cnt;
pi_evt_t sfu_out_task;
pi_sfu_mem_port_t * memout_port;
int sfu_buffer_filled;

void *L3_wavs;
void *l2_buffer;
void *l2_buffer_wgt_upd;

MFCC_IN_TYPE *MfccInSig_buff[N_TINYTEST];

// tinytest samples
static char tinytestutter[40][200] = {
"/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/tinytest/yes_e49428d9_nohash_3.wav",
"/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/tinytest/no_e49428d9_nohash_3.wav",
"/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/tinytest/up_0cb74144_nohash_2.wav",
"/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/tinytest/down_3659fc1c_nohash_1.wav",
"/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/tinytest/left_e1469561_nohash_1.wav",
"/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/tinytest/right_e49428d9_nohash_3.wav",
"/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/tinytest/on_e49428d9_nohash_3.wav",
"/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/tinytest/off_3659fc1c_nohash_1.wav",
"/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/tinytest/stop_3659fc1c_nohash_1.wav",
"/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/tinytest/go_b7e9f841_nohash_1.wav",
};

// train samples
static char class_0[12][130] = { "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/",};
static char class_1[12][130] = {};
static char class_2[12][130] = { "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/yes_51055bda_nohash_2.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/yes_cd671b5f_nohash_2.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/yes_73f20b00_nohash_4.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/yes_6fb3d5a7_nohash_0.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/yes_bfaf2000_nohash_1.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/yes_e0c782d5_nohash_2.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/yes_ffbb695d_nohash_0.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/yes_27b03931_nohash_0.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/yes_21cbe292_nohash_0.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/yes_44dad20e_nohash_0.wav",};
static char class_3[12][130] = { "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/no_9448c397_nohash_0.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/no_f0ae7203_nohash_0.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/no_ab46af55_nohash_4.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/no_cc6ee39b_nohash_2.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/no_da15e796_nohash_0.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/no_f2e9b610_nohash_1.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/no_b69fe0e2_nohash_2.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/no_8b25410a_nohash_1.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/no_0a196374_nohash_1.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/no_f5341341_nohash_4.wav",};
static char class_4[12][130] = { "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/up_12c206ea_nohash_0.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/up_cb62dbf1_nohash_4.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/up_1acc97de_nohash_2.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/up_87014d40_nohash_0.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/up_ce49cb60_nohash_0.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/up_b66f4f93_nohash_1.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/up_3e31dffe_nohash_1.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/up_c1b7c224_nohash_1.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/up_ce0cb033_nohash_2.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/up_6347b393_nohash_0.wav",};
static char class_5[12][130] = { "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/down_15c563d7_nohash_2.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/down_ccca5655_nohash_4.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/down_211b928a_nohash_1.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/down_87070229_nohash_0.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/down_0ff728b5_nohash_1.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/down_3777c08e_nohash_0.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/down_784e281a_nohash_0.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/down_03431e13_nohash_0.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/down_f68160c0_nohash_1.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/down_f5733968_nohash_0.wav",};
static char class_6[12][130] = { "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/left_ec74a8a5_nohash_0.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/left_07089da9_nohash_1.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/left_1a73dcd1_nohash_0.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/left_f5626af6_nohash_3.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/left_226537ab_nohash_1.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/left_7ff8e367_nohash_0.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/left_45692b02_nohash_0.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/left_2a0b413e_nohash_0.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/left_79b37d3a_nohash_0.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/left_73f20b00_nohash_0.wav",};
static char class_7[12][130] = { "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/right_7192fddc_nohash_1.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/right_f798ac78_nohash_4.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/right_28ed6bc9_nohash_0.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/right_9190045a_nohash_0.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/right_210f3aa9_nohash_0.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/right_0ea0e2f4_nohash_0.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/right_8134f43f_nohash_2.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/right_7dc50b88_nohash_0.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/right_8e05039f_nohash_0.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/right_fde2dee7_nohash_0.wav",};
static char class_8[12][130] = { "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/on_890cc926_nohash_3.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/on_51f7a034_nohash_1.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/on_18a8f03f_nohash_1.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/on_ad6a46f1_nohash_1.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/on_cae62f38_nohash_0.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/on_2197f41c_nohash_0.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/on_977a3be4_nohash_0.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/on_6a2fb9a5_nohash_0.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/on_324210dd_nohash_3.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/on_c245d3d7_nohash_0.wav",};
static char class_9[12][130] = { "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/off_173ce2be_nohash_2.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/off_7d6b4b10_nohash_1.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/off_e6327279_nohash_2.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/off_2dc4f05d_nohash_3.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/off_8dc18a75_nohash_2.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/off_b83c1acf_nohash_2.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/off_d070ea86_nohash_1.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/off_332d33b1_nohash_2.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/off_9fac5701_nohash_1.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/off_9dc1889e_nohash_0.wav",};
static char class_10[12][130] = { "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/stop_e11fbc6e_nohash_1.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/stop_48e8b82a_nohash_1.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/stop_cf87b736_nohash_0.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/stop_893705bb_nohash_12.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/stop_62b7c848_nohash_0.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/stop_eeaf97c3_nohash_1.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/stop_01bb6a2a_nohash_2.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/stop_2fcb6397_nohash_0.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/stop_f68160c0_nohash_3.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/stop_c351e611_nohash_4.wav",};
static char class_11[12][130] = { "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/go_f8f60f59_nohash_0.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/go_fb2f3242_nohash_0.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/go_29dce108_nohash_0.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/go_06a79a03_nohash_0.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/go_88120683_nohash_0.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/go_d874a786_nohash_0.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/go_46a153d8_nohash_2.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/go_34ba417a_nohash_0.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/go_61e2f74f_nohash_0.wav", "/usr/scratch/wetterhorn/cioflanc/kws_on_gap9/tiny_denoiser/res/wavsrc/go_017c4098_nohash_3.wav",};


#endif /* GLOBALS_H */