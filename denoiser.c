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

// L2
#include "input.h"

#include "Gap.h"
#include "bsp/ram.h"
#include <bsp/fs/hostfs.h>
#include "gaplib/wavIO.h" 

// Autotiler NN functions
// #include "RFFTKernels.h"
// #include "WinLUT_f16.def"   //load the input audio signal and compute the STFT

#define DEMO 1 
#define GRU 1
#include "denoiser_dns.h"

#define DISABLE_NN_INFERENCE 1

#define PERF 1


#define WAV_HEADER_SIZE 44 //bytes

/* 
     global variables
*/
struct pi_device DefaultRam; 
struct pi_device* ram = &DefaultRam;

AT_DEFAULTFLASH_FS_EXT_ADDR_TYPE __PREFIX(_L3_Flash) = 0;

#ifdef AUDIO_EVK
    // GPIO defines
    struct pi_device gpio_port;
    struct pi_device gpio_in;
    pi_gpio_e gpio_pin_o; /* PI_GPIO_A02-PI_GPIO_A05 */
    int val_gpio;
#endif

//static struct pi_default_flash_conf flash_conf;
static pi_fs_file_t * file[1];
static struct pi_device fs;
static struct pi_device flash;

// datatype for computation
#define DATATYPE_SIGNAL     float16
#define DATATYPE_SIGNAL_INF float16
#define SqrtF16(a) __builtin_pulp_f16sqrt(a)

#define IS_INPUT_STFT 0

// defines for audio IOs

// allocate space to load the input signal
char *WavName = NULL;

// copy input data to L3
// static uint32_t temporary_carrier;

/* 
    static allocation of temporary buffers
*/


// #include "GraphINOUT_L2_Descr.h"
#include "Graph_L2_Descr.h" // pdm_in_test

// #include "SFU_RT.h"

// FIXME: to tune it!!
#define Q_BIT_IN 27
#define Q_BIT_OUT (Q_BIT_IN-3)

// #define BUFF_SIZE (FRAME_STEP*4)
#define BUFF_SIZE (256*1024)
// #define BUFF_SIZE (32*1024)
#define AUDIO_BUFFER_SIZE 16000

#define CHUNK_NUM (8)

//This should be equal to FRAME_SIZE/FRAME_STEP + 1
#define STRUCT_DELAY (1)

#define SAI1         (1)


#define SAI_ITF_IN         (SAI1)


#define SAI_ID               (48)
#define SAI_SCK(itf)         (48+(itf*4)+0)
#define SAI_WS(itf)          (48+(itf*4)+1)
#define SAI_SDI(itf)         (48+(itf*4)+2)
#define SAI_SDO(itf)         (48+(itf*4)+3)


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


#define  NORM           6

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


// #include "mfcc_offline.h"

short int *inWav;
MFCC_IN_TYPE *MfccInSig;
OUT_TYPE *out_feat;
char * feat_char;

// DORY
#include "mem.h"
#include "network.h"

SFU_uDMA_Channel_T *ChanOutCtxt_0;

void * BufferInList;

volatile int remaining_size;
volatile int sent_size;
volatile int done;
int nb_transfers;
int current_size[2];


static PI_L2 uint8_t header_buffer[WAV_HEADER_SIZE];
static struct pi_device fs_wav;
static void *wavfile;

void dump_wav_open(char *filename, int width, int sampling_rate, int nb_channels, int size)
{
    unsigned int idx = 0;
    unsigned int sz = WAV_HEADER_SIZE + size;

    // 4 bytes "RIFF"
    header_buffer[idx++] = 'R';
    header_buffer[idx++] = 'I';
    header_buffer[idx++] = 'F';
    header_buffer[idx++] = 'F';

    // 4 bytes File size - 8bytes 32kS 0x10024 - 65408S 0x1ff24
    //header_buffer[idx++] = 0x24;
    //header_buffer[idx++] = 0xff;
    //header_buffer[idx++] = 0x01;
    //header_buffer[idx++] = 0x00;
    header_buffer[idx++] = (unsigned char) (sz & 0x000000ff);
    header_buffer[idx++] = (unsigned char)((sz & 0x0000ff00) >> 8);
    header_buffer[idx++] = (unsigned char)((sz & 0x00ff0000) >> 16);
    header_buffer[idx++] = (unsigned char)((sz & 0xff000000) >> 24);

    // 4 bytes file type: "WAVE"
    header_buffer[idx++] = 'W';
    header_buffer[idx++] = 'A';
    header_buffer[idx++] = 'V';
    header_buffer[idx++] = 'E';

    // 4 bytes format chunk: "fmt " last char is trailing NULL
    header_buffer[idx++] = 'f';
    header_buffer[idx++] = 'm';
    header_buffer[idx++] = 't';
    header_buffer[idx++] = ' ';

    // 4 bytes length of format data below, until data part
    header_buffer[idx++] = 0x10;
    header_buffer[idx++] = 0x00;
    header_buffer[idx++] = 0x00;
    header_buffer[idx++] = 0x00;

    // 2 bytes type of format: 1 (PCM)
    header_buffer[idx++] = 0x01;
    header_buffer[idx++] = 0x00;

    // 2 bytes nb of channels: 1 or 2
    //header_buffer[idx++] = 0x02;
    //header_buffer[idx++] = 0x01;
    header_buffer[idx++] = nb_channels;
    header_buffer[idx++] = 0x00;

    // 4 bytes sample rate in Hz:
    header_buffer[idx++] = (sampling_rate >> 0) & 0xff;
    header_buffer[idx++] = (sampling_rate >> 8) & 0xff;
    header_buffer[idx++] = (sampling_rate >> 16) & 0xff;
    header_buffer[idx++] = (sampling_rate >> 24) & 0xff;

    // 4 bytes (Sample Rate * BitsPerSample * Channels) / 8:
    // (8000*16*1)/8=0x3e80 * 2
    // (16000*16*1)/8=32000 or 0x6F00
    // (22050*16*1)/8=0xac44
    // (22050*16*2)/8=0x15888
    int rate = (sampling_rate * width * nb_channels) / 8;
    header_buffer[idx++] = (rate >> 0) & 0xff;
    header_buffer[idx++] = (rate >> 8) & 0xff;
    header_buffer[idx++] = (rate >> 16) & 0xff;
    header_buffer[idx++] = (rate >> 24) & 0xff;

    // 2 bytes (BitsPerSample * Channels) / 8:
    // 16*1/8=2 - 16b mono
    // 16*2/8=4 - 16b stereo
    rate = (width * nb_channels) / 8;
    header_buffer[idx++] = (rate >> 0) & 0xff;
    header_buffer[idx++] = (rate >> 8) & 0xff;

    // 2 bytes bit per sample:
    header_buffer[idx++] = width;
    header_buffer[idx++] = 0x00;

    // 4 bytes "data" chunk
    header_buffer[idx++] = 'd';
    header_buffer[idx++] = 'a';
    header_buffer[idx++] = 't';
    header_buffer[idx++] = 'a';

    // 4 bytes size of data section in bytes:
    header_buffer[idx++] = (unsigned char) (size & 0x000000ff);
    header_buffer[idx++] = (unsigned char)((size & 0x0000ff00) >> 8);
    header_buffer[idx++] = (unsigned char)((size & 0x00ff0000) >> 16);
    header_buffer[idx++] = (unsigned char)((size & 0xff000000) >> 24);

    struct pi_hostfs_conf conf;
    pi_hostfs_conf_init(&conf);

    pi_open_from_conf(&fs_wav, &conf);

    if (pi_fs_mount(&fs_wav))
     return;

    wavfile = pi_fs_open(&fs_wav, filename, PI_FS_FLAGS_WRITE);
    if (wavfile == 0)
    {
        printf("Failed to open file, %s\n", filename);
        return;
    }

    pi_fs_write(wavfile, header_buffer, WAV_HEADER_SIZE);
}

void dump_wav_write(void *data, int size)
{
    pi_fs_write(wavfile, data, size);
}


void dump_wav_close()
{
    pi_fs_close(wavfile);

    pi_fs_unmount(&fs_wav);
}


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

/*
    STFT computation
        argument parameters are manually set based on STFT configuration
*/

static void RunMFCC()
{
    #ifdef PERF
        gap_cl_starttimer();
        gap_cl_resethwtimer();
        int start = gap_cl_readhwtimer();
    #endif

    // Compute MFCC following Tensorflow settings
    #if (N_DCT == 0)
        printf("DCT is 0 \n");
        #if (DATA_TYPE==2) || (DATA_TYPE==3)
        Tensorflow_MFCC(MfccInSig, out_feat, FFTTwiddles, RFFTTwiddles, SwapTable, WindowLUT, MelFBSparsity, MelFBCoeff);
        #elif (DATA_TYPE==1)
        Tensorflow_MFCC(MfccInSig, out_feat, FFTTwiddles, SwapTable, WindowLUT, MelFBSparsity, MelFBCoeff, NORM);
        #else
        Tensorflow_MFCC(MfccInSig, out_feat, FFTTwiddles, RFFTTwiddles, SwapTable, WindowLUT, MelFBSparsity, MelFBCoeff, NORM);
        #endif
    #else
        printf("DCT is 1 \n");
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


    // // Instead of listening from microphone, we read from WAV.
    // // Allocate L3 buffers for audio IN


    inWav    = (short int *) pi_l2_malloc(AUDIO_BUFFER_SIZE * sizeof(short));   
    header_struct header_info;
    if (ReadWavFromFile(WavName, inWav, AUDIO_BUFFER_SIZE*sizeof(short), &header_info)){
        printf("Error reading wav file\n");
        pmsis_exit(1);
    }
    int num_samples = header_info.DataSize * 8 / (header_info.NumChannels * header_info.BitsPerSample);


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

    MfccInSig = (MFCC_IN_TYPE *) pi_l2_malloc(AUDIO_BUFFER_SIZE * sizeof(MFCC_IN_TYPE));
    // MfccInSig = (MFCC_IN_TYPE *) pi_l2_malloc(BUFF_SIZE);

    
    // /****
    //     Setup the SFU for PDM in/out
    // ****/
    // struct pi_device i2s_sai1;

    // // Configure PDM
    // if (open_i2s_PDM(&i2s_sai1, SAI1,   3072000, 3, 0)) return -1;

    // StartSFU(FREQ_SFU*1000*1000, 1);
    // ChanOutCtxt_0  = (SFU_uDMA_Channel_T *) pi_l2_malloc(sizeof(SFU_uDMA_Channel_T));


    // BufferInList = (void*) pi_l2_malloc(BUFF_SIZE);
        
    // // Get uDMA channels for Graph
    // SFU_Allocate_uDMA_Channel(ChanOutCtxt_0, 0, &SFU_RTD(Graph));

    // //Next API will have a value to replace this high number with -1
    // //To be able to 
    // SFU_Enqueue_uDMA_Channel(ChanOutCtxt_0, BufferInList, BUFF_SIZE);
    
    // // Connect Channels to SFU for Mic IN (PDM IN)
    // SFU_GraphConnectIO(SFU_Name(Graph, Out1), ChanOutCtxt_0->ChannelId, 0, &SFU_RTD(Graph));
    // SFU_GraphConnectIO(SFU_Name(Graph, In1), SAI_ITF_IN, 2, &SFU_RTD(Graph));

    // pi_l2_free(ChanOutCtxt_0, sizeof(SFU_uDMA_Channel_T));

    // fxl6408_setup();

    // printf("Recording!\n");

    // //Starting In and Out Graphs
    // pi_i2s_ioctl(&i2s_sai1, PI_I2S_IOCTL_START, NULL);
    // // Let the microphone start
    // pi_time_wait_us(30000); 

    // chunk_in_cnt=0;
    // SFU_StartGraph(&SFU_RTD(Graph));
    // pi_time_wait_us(2000000);

    // pi_i2s_ioctl(&i2s_sai1, PI_I2S_IOCTL_STOP, NULL);
    printf("Finished!\n");
    // Dory init
    // TODO: Remove flash init and/or ram init duplicates
    mem_init();
    network_initialize(); // Absent in L2

    while(1){

#ifdef AUDIO_EVK
        pi_gpio_pin_write(gpio_pin_o, 1);
#endif

        // int round = (chunk_in_cnt%CHUNK_NUM);
        // int round_out = (chunk_in_cnt>(STRUCT_DELAY-1))? ((chunk_in_cnt-(STRUCT_DELAY-1))%CHUNK_NUM):0;


        // printf ("Scale data\n");
        // int outidx = 0;
        // for(int i=0;i<BUFF_SIZE;i+=3){
        //     // We for now assume that no rescaling is needed
            
        //     // MfccInSig[outidx] = ((MFCC_IN_TYPE *)BufferInList)[i];
        //     // MfccInSig[outidx] = (MFCC_IN_TYPE) gap_clip( ( (int *) BufferInList)[i], 15);
        //     // MfccInSig[outidx] = (MFCC_IN_TYPE)(((float)((int32_t*)BufferInList)[i]) /((int)(1<<Q_BIT_IN)));

        //     // WORKING
        //     MfccInSig[outidx] = (MFCC_IN_TYPE)(((float)((int32_t*)BufferInList)[i]) /((int)(1<<12)));
        //     outidx++;
        //     if (outidx == AUDIO_BUFFER_SIZE){
        //         break;
        //     }
            
        // }
        // pi_l2_free(BufferInList, BUFF_SIZE);

        // TODO: Measure error here versus only copying
        #if (DATA_TYPE==2) || (DATA_TYPE==3)
            for (int i=0; i<AUDIO_BUFFER_SIZE; i++) { // BUFF_SIZE for MIC, AUDIO_BUFFER_SIZE for WAV
                MfccInSig[i] = (MFCC_IN_TYPE) inWav[i] / (1<<15);
            }
        #else
            for (int i=0; i<AUDIO_BUFFER_SIZE; i++) { // BUFF_SIZE for MIC, AUDIO_BUFFER_SIZE for WAV
                MfccInSig[i] = (MFCC_IN_TYPE) gap_clip(((int) inWav[i]), 15);
                // MfccInSig[i] = (MFCC_IN_TYPE) gap_clip(((int) inWav[i]), 10); // TODO: 10 or 9 give absurdly better results
            }
        #endif
        // for (int i=0; i<AUDIO_BUFFER_SIZE; i++) { // BUFF_SIZE for MIC, AUDIO_BUFFER_SIZE for WAV
        //     MfccInSig[i] = (MFCC_IN_TYPE) inWav[i];
        // }

        // pi_l2_free(inWav, AUDIO_BUFFER_SIZE * sizeof(short));
        
        out_feat = (OUT_TYPE *) pi_l2_malloc(49 * 10 * 4 * sizeof(OUT_TYPE));    
        feat_char = (char*) pi_l2_malloc(49 * 10 * sizeof(char));

        // MFCC generation - KWS on PULP

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
        // pi_l2_free(MfccInSig, AUDIO_BUFFER_SIZE * sizeof (MFCC_IN_TYPE));
        // pi_l2_free(MfccInSig, BUFF_SIZE);

        pi_cluster_close(&cluster_dev);

        printf("MFCC Computation complete. Rescaling data\n");
        int k = 0;
        for (int i = 0; i < 1960;i++){
            
            // Rescale MFCCs to match Tensorflow-generated ones
            // pow(2, -5): Checking L2 output: Checksum Failed: true [104159] vs. calculated [104953]
            // pow(2, -4): Checking L2 output: Checksum Failed: true [104159] vs. calculated [118521]
            feat_char[k] = (char) (((int) floor(out_feat[i] * pow(2, -4) * sqrt(0.2))) + 128);
            // Select 10 MFCC per window
            if (i == 40*(k/10) + 9){
                i = 40*(k/10) + 39;
            }
            k++;
        } 

        // TEST

        #if (DATA_TYPE==2) || (DATA_TYPE==3)
        float QSNR_THR = 40;
        #else
        float QSNR_THR = 38;
        #endif
        int N_FRAME = 49;
        int frame_size = 10;
        float MSE = 0.0, SUM = 0.0;
            for (int i=0; i<N_FRAME; i++) {
                for (int j=0; j<frame_size; j++) {
                    #if (DATA_TYPE==2) || (DATA_TYPE==3)
                          MSE += (L2_input_h[i*frame_size+j] - feat_char[i*frame_size+j])*(L2_input_h[i*frame_size+j] - feat_char[i*frame_size+j]);
                    #else
                          int QMFCC = 15 - NORM - 7;
                          MSE += (L2_input_h[i*frame_size+j] - FIX2FP(feat_char[i*frame_size+j], QMFCC)) * (L2_input_h[i*frame_size+j] - FIX2FP(feat_char[i*frame_size+j], QMFCC));
                    #endif
                    SUM += (L2_input_h[i*frame_size+j])*(L2_input_h[i*frame_size+j]);
                }
            }
            float QSNR = 10*log10(SUM / MSE);
            printf("QSNR: %f (thr: %f) --> ", QSNR, QSNR_THR);
            if (QSNR < QSNR_THR) {
                printf("Test NOT PASSED\n");
                pmsis_exit(-1);
            } else {
                printf("Test PASSED\n");
            }


        pi_l2_free(out_feat, 49*10*4*sizeof(OUT_TYPE));

        void *l2_buffer;
        l2_buffer = pi_l2_malloc(80000);
        if (l2_buffer == NULL) {
            printf("failed to allocate memory for l2_buffer\n");
        }

        for (int i = 0; i < 490; i++){
            // ((uint8_t *)l2_buffer)[i] = L2_input_h[i]; // Precomputed MFCC
            ((uint8_t *)l2_buffer)[i] = feat_char[i]; // Online computed MFCC
            // printf("%i\n", feat_char[i]); // Online computed MFCC
        }

        printf("Memory allocated.\n");
        // L3
        network_run(l2_buffer, 80000, l2_buffer, 0);

        // L2
        // network_run(L2_input, 380000, l2_buffer, 0, L2_input_h);
        // network_run(L2_input, 380000, l2_buffer, 0, L2_input);

        pi_l2_free(l2_buffer, 80000);

        break;


        // block until next input audio frame is ready
#ifdef  AUDIO_EVK
        pi_gpio_pin_write(gpio_pin_o, 0);
#endif
        chunk_in_cnt++;
    }


    dump_wav_open("test_gap.wav", 16, 16000, 1, sizeof(short)*AUDIO_BUFFER_SIZE);
    dump_wav_write(MfccInSig, sizeof(short)*AUDIO_BUFFER_SIZE);
    dump_wav_close();

    // ORIGINAL
    // dump_wav_open("test_gap.wav", 32, 48000, 1, BUFF_SIZE);
    // dump_wav_write(MfccInSig, BUFF_SIZE);
    // dump_wav_close();

    printf("Writing wav file to test_gap.wav completed successfully\n");

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

    return denoiser();
}


// 'yes,no,up,down,left,right,on,off,stop,go,'

