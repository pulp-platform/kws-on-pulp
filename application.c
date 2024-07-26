// Copyright (C) 2023-2024 ETH Zurich
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


#include "application.h"
#include "localutil.h"
#include "preprocess.h"
#include "acquire.h"
#include "train.h"

// Peripherals
#include "Gap.h"
#include "bsp/ram.h"
#include <bsp/fs/hostfs.h>
#include "gaplib/wavIO.h" 

// DORY
#include "mem.h"
#include "network.h"

// PULP TrainLib
#include "net.h"

// Noise for testing
#include "noise.h"

// Measurement
pi_gpio_e gpio_pin_measurement;
unsigned int gpio_pin_measurement_id = 89;

// Load args
char *WavName = NULL;
int mfcc_src = NULL;
int noise_train_src = NULL;
int uttr_train_src = NULL;
int uttr_inf_src = NULL;

/* Read button */
static const pi_gpio_e gpio_boot_pin_1 = PAD_GPIO_UPB;
void read_button(int * button_pressed){
    pi_gpio_pin_read(gpio_boot_pin_1, button_pressed);
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

    /****
        Configure And Open the External Ram. 
    ****/
    struct pi_device DefaultRam; 
    struct pi_device* ram = &DefaultRam;
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
    pi_gpio_flags_e flags_upb = PI_GPIO_INPUT;
    pi_gpio_pin_configure(gpio_boot_pin_1, flags_upb);


    // Measurement preparation
    pi_pad_function_set(gpio_pin_measurement_id, 1);
    pi_gpio_pin_configure(gpio_pin_measurement_id, PI_GPIO_OUTPUT);
    pi_gpio_pin_write(gpio_pin_measurement_id, 0);
    pi_gpio_pin_write(gpio_pin_measurement_id, 0);

    // Measurement start
    pi_gpio_pin_write(gpio_pin_measurement_id, 1);

    // Dory init
    mem_init();
    network_initialize(); // Absent in L2-only
    pi_cluster_close(&cluster_dev);

    PRINTF ("----------------------------- Read WAVs from filesystem ---------------------------\n");

    L3_wavs = ram_malloc(WAVRAM);
    printf("\nL3_wavs alloc initial\t@ %d:\t%s\n", (unsigned int)L3_wavs, L3_wavs?"Ok":"Failed");

    int startwavreading = pi_time_get_us();
    for (int i = 0; i < 100; i++) {

        header_struct header_info;
        short int *inWav = (short int *) pi_l2_malloc(AUDIO_BUFFER_SIZE * sizeof(short)); 
        if (inWav == NULL){
            printf("Failed allocating inWav.\n");
            pmsis_exit(-1);
        }

        if (i < 10){
            if (ReadWavFromFile(class_2[i], inWav, AUDIO_BUFFER_SIZE*sizeof(short), &header_info)){
                printf("Error reading wav file\n");
                pmsis_exit(1);
            }
        }
        else if (i < 20){
            if (ReadWavFromFile(class_3[i%10], inWav, AUDIO_BUFFER_SIZE*sizeof(short), &header_info)){
                printf("Error reading wav file\n");
                pmsis_exit(1);
            }
        }
        else if (i < 30){
            if (ReadWavFromFile(class_4[i%20], inWav, AUDIO_BUFFER_SIZE*sizeof(short), &header_info)){
                printf("Error reading wav file\n");
                pmsis_exit(1);
            }
        }
        else if (i < 40){
            if (ReadWavFromFile(class_5[i%30], inWav, AUDIO_BUFFER_SIZE*sizeof(short), &header_info)){
                printf("Error reading wav file\n");
                pmsis_exit(1);
            }
        }
        else if (i < 50){
            if (ReadWavFromFile(class_6[i%40], inWav, AUDIO_BUFFER_SIZE*sizeof(short), &header_info)){
                printf("Error reading wav file\n");
                pmsis_exit(1);
            }
        }
        else if (i < 60){
            if (ReadWavFromFile(class_7[i%50], inWav, AUDIO_BUFFER_SIZE*sizeof(short), &header_info)){
                printf("Error reading wav file\n");
                pmsis_exit(1);
            }
        }
        else if (i < 70){
            if (ReadWavFromFile(class_8[i%60], inWav, AUDIO_BUFFER_SIZE*sizeof(short), &header_info)){
                printf("Error reading wav file\n");
                pmsis_exit(1);
            }
        }
        else if (i < 80){
            if (ReadWavFromFile(class_9[i%70], inWav, AUDIO_BUFFER_SIZE*sizeof(short), &header_info)){
                printf("Error reading wav file\n");
                pmsis_exit(1);
            }
        } 
        else if (i < 90){
            if (ReadWavFromFile(class_10[i%80], inWav, AUDIO_BUFFER_SIZE*sizeof(short), &header_info)){
                printf("Error reading wav file\n");
                pmsis_exit(1);
            }
        } 
        else if (i < 100){
            if (ReadWavFromFile(class_11[i%90], inWav, AUDIO_BUFFER_SIZE*sizeof(short), &header_info)){
                printf("Error reading wav file\n");
                pmsis_exit(1);
            }
        }         

        ram_write(L3_wavs + i*AUDIO_BUFFER_SIZE*sizeof(short), inWav, AUDIO_BUFFER_SIZE*sizeof(short));
        
        // remove for measurements
        if (i%10 == 0){
            printf(" %i/110 samples read.\n", i);
        }

        pi_l2_free(inWav, AUDIO_BUFFER_SIZE*sizeof(short));

    }
    printf("100/110 samples read.\n");

    for (int i = 0; i < 10; i++) {

        short int *inWav = (short int *) pi_l2_malloc(AUDIO_BUFFER_SIZE * sizeof(short)); 
        if (inWav == NULL){
            printf("Failed allocating inWav.\n");
            pmsis_exit(-1);
        }
        header_struct header_info;
        if (ReadWavFromFile(tinytestutter[i], inWav, AUDIO_BUFFER_SIZE*sizeof(short), &header_info)){
            printf("Error reading wav file\n");
            pmsis_exit(1);
        }
        ram_write(L3_wavs + (100+i)*AUDIO_BUFFER_SIZE*sizeof(short), inWav, AUDIO_BUFFER_SIZE*sizeof(short));

        pi_l2_free(inWav, AUDIO_BUFFER_SIZE*sizeof(short));
    }

    int endwavreading = pi_time_get_us();
    printf("110/110 samples read, WAV reading is complete in %d us.\n", endwavreading - startwavreading);

    /* Backbone inference */
    l2_buffer = pi_l2_malloc(L2_MEMORY_SIZE);
    if (l2_buffer == NULL) {
        printf("failed to allocate memory for l2_buffer\n");
    }

    network_run(l2_buffer, L2_MEMORY_SIZE, l2_buffer, 0, 1); // L2_input_h extra-arg for L2-only

    /* Classifier preparation */
    pi_cluster_conf_init(&cl_conf);
    pi_open_from_conf(&cluster_dev, &cl_conf);
    if (pi_cluster_open(&cluster_dev))
    {
      return -1;
    }
    
    l2_buffer_wgt_upd = pi_l2_malloc (NCHANNELS * N_CLASSES * 4);
    if (l2_buffer_wgt_upd == NULL) {
        printf("failed to allocate memory for l2_buffer_wgt_upd\n");
    }

    /* Classifier inference */
    int predidx = -1;
    float ce_loss = 0.;
    unsigned int args_init_classifier[6];
    args_init_classifier[0] = (unsigned int) l2_buffer;
    args_init_classifier[1] = (unsigned int) l2_buffer_wgt_upd;
    args_init_classifier[2] = (unsigned int) 0; // initialize
    args_init_classifier[3] = (unsigned int) 0; // classidx (placeholder)
    args_init_classifier[4] = (float*) &ce_loss;
    args_init_classifier[5] = (int *) &predidx;

    pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, net_step, args_init_classifier));
    pi_cluster_close(&cluster_dev);

    BufferInList = (void*) pi_l2_malloc(BUFF_SIZE);
    if (BufferInList == NULL) return -1;

    /* Preparing recording */
    int button_pressed = 0;
    int sfu_out_buffer_cnt_prev = 0;
    int sfu_out_buffer_cnt_curr = 0;
    int buffer_len = 0;
    int buffer_upperlim = 0;
    int buffer_lowerlim = 0;
    pi_evt_sig_init(&inference_task);
    microphone_setup();

    /* Preparing preprocessing */
    MFCC_IN_TYPE * MfccInSig;
    MFCC_IN_TYPE * MfccInSig_prev;
    int16_t *MfccInSig_int16;
    OUT_TYPE *MfccOutSig;
    MfccInSig_prev = (MFCC_IN_TYPE *) pi_l2_malloc(1 * AUDIO_BUFFER_SIZE * sizeof (MFCC_IN_TYPE));

    // Manually handling silence
    int threshold_counter = 0;
    
    printf ("----------------------------- Starting application ---------------------------\n");

    while (1){
    
        
        if (uttr_inf_src == ONLINE){

            PRINTF ("----------------------------- Start acquisition ---------------------------\n");    

            int start_dataacq = pi_time_get_us();    
            

            MfccInSig = (MFCC_IN_TYPE *) pi_l2_malloc(1 * AUDIO_BUFFER_SIZE * sizeof (MFCC_IN_TYPE));
            for(int i=0;i<AUDIO_BUFFER_SIZE;i++){ 
                MfccInSig[i] = MfccInSig_prev[i];
            }  
            pi_evt_wait(&inference_task);

            sfu_out_buffer_cnt_curr = sfu_out_buffer_cnt;
            if (sfu_out_buffer_cnt_curr > sfu_out_buffer_cnt_prev){
                buffer_len = sfu_out_buffer_cnt_curr - sfu_out_buffer_cnt_prev;
                buffer_upperlim = sfu_out_buffer_cnt_curr * DOUBLE_BUFF_SIZE;
                buffer_lowerlim = 0;
            }
            else{
                buffer_len = (48 - sfu_out_buffer_cnt_prev) + sfu_out_buffer_cnt_curr;
                buffer_upperlim = BUFF_SIZE/sizeof(int32_t);
                buffer_lowerlim = sfu_out_buffer_cnt_curr * DOUBLE_BUFF_SIZE;
            }

            int outidx = 0;
            for (int i = sfu_out_buffer_cnt_prev*DOUBLE_BUFF_SIZE; i < buffer_upperlim; i+=3){
                // using MfccInSig_prev as buffer
                MfccInSig_prev[outidx] = (MFCC_IN_TYPE) (((float)((int32_t *)BufferInList)[i]) / (float)(1<<31 - 1));
                outidx++;
            }
            for (int i = 0; i < buffer_lowerlim; i+=3){
                // using MfccInSig_prev as buffer
                MfccInSig_prev[outidx] = (MFCC_IN_TYPE) (((float)((int32_t *)BufferInList)[i]) / (float)(1<<31 - 1));
                outidx++;
            }

            int j = 0;
            for (int i = DOUBLE_BUFF_SIZE*(buffer_len)/3; i < AUDIO_BUFFER_SIZE; i++){
                MfccInSig[j] = MfccInSig[i];    
                j++;
            }
            int k = 0;
            for (int i = AUDIO_BUFFER_SIZE - DOUBLE_BUFF_SIZE*(buffer_len)/3; i < AUDIO_BUFFER_SIZE; i++){
                MfccInSig[i] = MfccInSig_prev[k];
                k++;
            }

            for (int i = 0; i < AUDIO_BUFFER_SIZE; i++){
                MfccInSig_prev[i] = MfccInSig[i];
            }
            sfu_out_buffer_cnt_prev = sfu_out_buffer_cnt_curr;
            
            MfccInSig_int16 = (int16_t *) pi_l2_malloc(sizeof(int16_t) * AUDIO_BUFFER_SIZE);
            float mean = 0;
            for(int i=0;i<AUDIO_BUFFER_SIZE;i++){
                MfccInSig_int16[i] = (int16_t) (MfccInSig[i] * (1<<15));
                mean = mean + MfccInSig_int16[i]; 
            }
            mean = mean/AUDIO_BUFFER_SIZE;

            // TODO: FIGURE OUT THRESHOLD
            for(int i=0;i<AUDIO_BUFFER_SIZE;i++){
                if(MfccInSig_int16[i] < mean - 1000 || MfccInSig_int16[i] > mean + 1000){
                    threshold_counter++;
                }
            }

            // Commented out for measurements
            // if (threshold_counter < 200){
            //     printf("silence\n");
            //     threshold_counter = 0;
            //     pi_l2_free(MfccInSig_int16, sizeof(int16_t) * AUDIO_BUFFER_SIZE);
            //     pi_l2_free(MfccInSig, sizeof(int16_t) * AUDIO_BUFFER_SIZE);
            //     goto checkbutton;
            // }

            int end_dataacq = pi_time_get_us(); 
            PRINTF ("Data acquisition: %i\n", end_dataacq - start_dataacq);

        }
        else if (uttr_inf_src == OFFLINE){

            PRINTF ("***************************** Reading wav *****************************\n");
            
            int start_readwav = pi_time_get_us();
            
            // Read the 0th .wav saved in RAM
            short int *prepWav = NULL;
            prepWav = (short int *) pi_l2_malloc(AUDIO_BUFFER_SIZE * sizeof(short));

            // Read first sample in RAM (yes)
            ram_read(prepWav, L3_wavs + (0)*AUDIO_BUFFER_SIZE*sizeof(short), AUDIO_BUFFER_SIZE*sizeof(short));

            MfccInSig = (MFCC_IN_TYPE *) pi_l2_malloc(AUDIO_BUFFER_SIZE * sizeof(MFCC_IN_TYPE));
            if (MfccInSig == NULL){
                printf("Failed allocating MfccInSig.\n");
                pmsis_exit(-1);
            }
        
            #if (DATA_TYPE==2) || (DATA_TYPE==3)
                for (int i=0; i<AUDIO_BUFFER_SIZE; i++) { // BUFF_SIZE for MIC, AUDIO_BUFFER_SIZE for WAV
                    MfccInSig[i] = (MFCC_IN_TYPE) prepWav[i] / (1<<15);
                }
            #else
                for (int i=0; i<AUDIO_BUFFER_SIZE; i++) { // BUFF_SIZE for MIC, AUDIO_BUFFER_SIZE for WAV
                    MfccInSig[i]] = (MFCC_IN_TYPE) gap_fcip(((int) prepWav[i]), 15);
                }
            #endif

            pi_l2_free(prepWav, AUDIO_BUFFER_SIZE * sizeof(short int));

            // Used for .wav saving
            MfccInSig_int16 = (int16_t *) pi_l2_malloc(sizeof(int16_t) * AUDIO_BUFFER_SIZE);

            int end_readwav = pi_time_get_us();
            PRINTF("Reading .wav: %i\n", end_readwav - start_readwav);
        }

        #ifdef PERF
        gap_fc_starttimer();
        gap_fc_resethwtimer();
        int start_timer_mfcc = gap_fc_readhwtimer();        
        int start_readmfcc = pi_time_get_us();
        #endif

        preprocess(MfccInSig, l2_buffer, mfcc_src);

        for (int idx = 0; idx < N_MFCC_WINS * N_MFCC_MELS; idx++){
            PRINTF ("mfcc[%i] = %u, ", idx, ((uint8_t *)l2_buffer)[idx]);
        }
        PRINTF("\n");

        PRINTF ("***************************** Backbone inference **************************\n");
        int start_backbone = pi_time_get_us();
        

        // Extract backbone features
        network_run(l2_buffer, L2_MEMORY_SIZE, l2_buffer, 0, 1); // L2_input_h extra-arg for L2-only

        for (int idx = 0; idx < 64; idx++){
            PRINTF ("backbone[%i] = %u, ", idx, ((uint8_t *)l2_buffer)[idx]);
        }
        PRINTF("\n");

        #ifdef PERF
        int elapsed_timer_4 = gap_fc_readhwtimer() - start_timer_4;
        printf("Backbone inference: %d cycles\n", elapsed_timer_4);
        #endif

        int end_backbone = pi_time_get_us();
        PRINTF("Backbone: %i us\n", end_backbone - start_backbone);

        #ifdef PERF
        gap_fc_starttimer();
        gap_fc_resethwtimer();
        int start_timer_5 = gap_fc_readhwtimer();    
        #endif    

        PRINTF ("***************************** Classsifier inference **************************\n");

        int start_classif = pi_time_get_us();

        pi_cluster_conf_init(&cl_conf);
        pi_open_from_conf(&cluster_dev, &cl_conf);
        if (pi_cluster_open(&cluster_dev))
        {
          return -1;
        }

        int predidx = -1;
        float ce_loss = 0.;
        unsigned int args_inference_classifier[6];
        args_inference_classifier[0] = (unsigned int) l2_buffer;
        args_inference_classifier[1] = (unsigned int) l2_buffer_wgt_upd;
        args_inference_classifier[2] = (unsigned int) 1; // inference
        args_inference_classifier[3] = (unsigned int) 0; // classidx (placeholder)
        args_inference_classifier[4] = (float *) &ce_loss;
        args_inference_classifier[5] = (int *) &predidx;

        pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, net_step, args_inference_classifier));
        pi_cluster_close(&cluster_dev);

        pi_gpio_pin_write(gpio_pin_measurement_id, 0);

        PRINTF("***************************** Finished measurements *****************************\n");
       
        #ifdef PERF
        int elapsed_timer_5 = gap_fc_readhwtimer() - start_timer_5;
        printf("FC inference: %d cycles\n", elapsed_timer_5);
        #endif

        #ifdef PERF
            dump_wav_open("recording_utterance.wav", 16, 16000, 1, sizeof(int16_t) * AUDIO_BUFFER_SIZE);
            dump_wav_write(MfccInSig_int16, sizeof(int16_t) * AUDIO_BUFFER_SIZE);
            dump_wav_close();
        #endif

        // Measurements
        // pmsis_exit(-1);
        // return 0;

        pi_l2_free(MfccInSig_int16, sizeof(int16_t) * AUDIO_BUFFER_SIZE);

        int end_classif = pi_time_get_us();
        PRINTF("Classifier inference: %i us\n", end_classif - start_classif);
        
        checkbutton:
        button_pressed = 0;
        read_button(&button_pressed);
        button_pressed = 1; // measurement

        if (button_pressed){
            if (noise_train_src == ONLINE){

                printf ("----------------------------- Button pressed, recording noise ---------------------------\n");
                
                // wait 1s (for the previous non-noise content to be cleaned)
                pi_time_wait_us (1000000);

                MFCC_IN_TYPE * RecordedNoise = (MFCC_IN_TYPE *) pi_l2_malloc(NOISE_LEN_S*AUDIO_BUFFER_SIZE * sizeof(MFCC_IN_TYPE));
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
            else if (noise_train_src == OFFLINE){

                printf ("----------------------------- Button pressed, loading noise ---------------------------\n");

                MFCC_IN_TYPE * RecordedNoise = (MFCC_IN_TYPE *) pi_l2_malloc(NOISE_LEN_S * AUDIO_BUFFER_SIZE * sizeof(MFCC_IN_TYPE));
                wav_to_array(WavName, RecordedNoise, 1, 0); // NoiseName, noise, save
            }

            pi_gpio_pin_write(gpio_pin_measurement_id, 1);

            int evaluationtime = pi_time_get_us();

            ce_loss_pre = 0;
            ce_loss_post = 0;
            ce_loss_pre_val = 0;
            ce_loss_post_val = 0;
            correct_pre_val = 0;
            correct_post_val = 0;

            printf ("----------------------------- Pre-ODDA evaluation -----------------------------\n");
            
            // TODO: Add online/offline decision
            evaluate_tinytest(0);

            int endevaluationtime = pi_time_get_us();
            PRINTF("Evaluation time: %i\n", endevaluationtime - evaluationtime);

            printf("***************************** Finished pre-ODDA evaluation, now training... *****************************\n");

            int traintime = pi_time_get_us();
            train();
            int endtraintime = pi_time_get_us();
            PRINTF("Train time: %i\n", endtraintime - traintime);

            printf ("----------------------------- Post-ODDA evaluation -----------------------------\n");

            // TODO: Add online/offline decision
            evaluate_tinytest(1);

            pmsis_exit(0);
            return; // breaking loop early

        }

        // // block until next input audio frame is ready
        // #ifdef  AUDIO_EVK
        //     pi_gpio_pin_write(gpio_pin_o, 0);
        // #endif

        // TODO: Trigger inference every 250 ms
        pi_time_wait_us(250);

    }

    printf ("----------------------------- Application completed ---------------------------\n");

    pmsis_exit(0);
    return 0;
}

int main()
{
    WavName = __XSTR(WAV_FILE); 
    mfcc_src = __XSTR(MFCC) == "0" ? 0 : 1;
    noise_train_src = __XSTR(NOISE_EVAL) == "0" ? 0 : 1;
    uttr_train_src = __XSTR(UTTR_EVAL) == "0" ? 0 : 1;
    uttr_inf_src = __XSTR(APPL) == "0" ? 0 : 1;

    return application();
}