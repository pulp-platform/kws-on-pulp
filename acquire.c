#include "acquire.h"

#include "gaplib/wavIO.h" 

// Configure PDM RX interface
int configure_pdm()
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

// Configure PMSIS SFU transfer
void handle_out_transfer_end(void *arg)
{
    pi_sfu_enqueue(sfu_graph, memout_port, &sfu_out_buffers[sfu_out_buffer_idx]);
    /*
     * Buffer received from MEM_OUT.
     * Here we just do a simple copy to the MEM_IN buffer that is not currently being transferred.
     */
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

// Set up microphone for recording
void microphone_setup(){

    // Open SFU with default frequency
    pi_sfu_conf_t conf = { .sfu_frequency=0 };
    if (pi_sfu_open(&conf))
        printf("SFU device open failed\n");
    printf("SFU activated\n");

    sfu_graph = pi_sfu_graph_open(&SFU_RTD(Graph));
    if (sfu_graph == NULL)
        printf("SFU graph open failed\n");
    printf("Graph opened\n");


    sfu_out_buffer_idx = 0;
    sfu_out_buffer_cnt = 0;
    sfu_buffer_filled = 0;

    // Allocate IO buffers
    for (int i = 0; i < NB_BUF_IN_RING; i++)
    {
        void *data_out = pi_l2_malloc(DOUBLE_BUFF_SIZE * sizeof(int));
        if (data_out == NULL) return -1;
        pi_sfu_buffer_init(&sfu_out_buffers[i], data_out, DOUBLE_BUFF_SIZE, sizeof(int));

    }

    // Configure interfaces
    int err = configure_pdm();
    if (err != 0)
        printf("PDM interface init failed\n");
    printf("PDM Rx interface configured\n");

    // Get port refs
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
}



void wav_to_array(char* wavfile, MFCC_IN_TYPE* buffer, int noise, int save){
    
    // Allocate L3 buffers for audio IN     
    header_struct header_info;

    short int *inWav = (short int *) pi_l2_malloc(AUDIO_BUFFER_SIZE * sizeof(short)); 
    if (inWav == NULL){
        printf("Failed allocating inWav.\n");
        pmsis_exit(-1);
    }
    if (ReadWavFromFile(wavfile, inWav, AUDIO_BUFFER_SIZE*sizeof(short), &header_info)){
        printf("Error reading wav file\n");
        pmsis_exit(1);
    }
    for (int i = 0; i < 5; i++){
        PRINTF("inWav[%i] = %i, ", i, inWav[i]);
    }
    PRINTF("\n");

    if (noise){
        buffer = (MFCC_IN_TYPE *) pi_l2_malloc(NOISE_LEN_S*AUDIO_BUFFER_SIZE * sizeof(MFCC_IN_TYPE));
        if (buffer == NULL){
            printf("Failed allocating buffer.\n");
            pmsis_exit(-1);
        }
        #if (DATA_TYPE==2) || (DATA_TYPE==3)
            for (int i=0; i<NOISE_LEN_S*AUDIO_BUFFER_SIZE; i++) { // BUFF_SIZE for MIC, AUDIO_BUFFER_SIZE for WAV
                // READ WAV
                buffer[i] = (MFCC_IN_TYPE) inWav[i] / (1<<15);
            }
        #else
            for (int i=0; i<NOISE_LEN_S*AUDIO_BUFFER_SIZE; i++) { // BUFF_SIZE for MIC, AUDIO_BUFFER_SIZE for WAV
                // READ WAV
                buffer[i] = (MFCC_IN_TYPE) gap_fcip(((int) inWav[i]), 15);
            }
        #endif

    }
    else {
        buffer = (MFCC_IN_TYPE *) pi_l2_malloc(AUDIO_BUFFER_SIZE * sizeof(MFCC_IN_TYPE));
        if (buffer == NULL){
            printf("Failed allocating buffer.\n");
            pmsis_exit(-1);
        }
    
        #if (DATA_TYPE==2) || (DATA_TYPE==3)
            for (int i=0; i<AUDIO_BUFFER_SIZE; i++) { // BUFF_SIZE for MIC, AUDIO_BUFFER_SIZE for WAV
                buffer[i] = (MFCC_IN_TYPE) inWav[i] / (1<<15);
            }
        #else
            for (int i=0; i<AUDIO_BUFFER_SIZE; i++) { // BUFF_SIZE for MIC, AUDIO_BUFFER_SIZE for WAV
                buffer[i] = (MFCC_IN_TYPE) gap_fcip(((int) inWav[i]), 15);
            }
        #endif
    }
    
    if (save){
        // Log WAV 
        // TODO: use *_int16 for saving
        if (noise){
            dump_wav_open("noise_file.wav", 16, 16000, 1, NOISE_LEN_S*AUDIO_BUFFER_SIZE*sizeof(short));
            dump_wav_write(inWav, NOISE_LEN_S*AUDIO_BUFFER_SIZE*sizeof(short));
            dump_wav_close();
            PRINTF("Writing wav file to noise_file.wav completed successfully\n"); 
        }
        else{   
            dump_wav_open("utter_file.wav", 16, 16000, 1, AUDIO_BUFFER_SIZE*sizeof(short));
            dump_wav_write(inWav, AUDIO_BUFFER_SIZE*sizeof(short));
            dump_wav_close();
            PRINTF("Writing wav file to utter_file.wav completed successfully\n");
        }
    }
    
    if (noise){
        pi_l2_free(inWav, NOISE_LEN_S*AUDIO_BUFFER_SIZE * sizeof(short));
    }
    else{
        pi_l2_free(inWav, AUDIO_BUFFER_SIZE * sizeof(short));
    }
}