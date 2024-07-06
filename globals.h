// Definitions

#ifndef __GLOBALS_H__
#define __GLOBALS_H__

#define SCALE_IN denoiser_dns_Input_1_OUT_SCALE
#define SCALE_OUT denoiser_dns_Output_1_OUT_SCALE

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



#endif /* GLOBALS_H */