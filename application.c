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

// Peripherals
#include "Gap.h"
#include "bsp/ram.h"
#include <bsp/fs/hostfs.h>
#include "gaplib/wavIO.h" 

#include "localutil.h"


// DORY
#include "mem.h"
#include "network.h"

// PULP TrainLib
#include "net.h"

// Clean utterances
#include "utterances.h"

// Test utterances
#include "tinytest.h"

// Large validation set
// #include "validation.h"
#include "testing.h"

#include "noise_meeting.h"


// SIMPLIFICATION
#include "preprocess.h"
#include "acquire.h"

// measurement
// unsigned int GPIOs = PI_GPIO_A89;
// unsigned int GPIOs = 89;
// #define WRITE_GPIO(x) pi_gpio_pin_write(GPIOs,x)
pi_gpio_e gpio_pin_measurement;
unsigned int gpio_pin_measurement_id = 89;

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

// Load args
char *WavName = NULL;
int mfcc_src = NULL;
int noise_train_src = NULL;
int uttr_train_src = NULL;
int uttr_inf_src = NULL;


void *l2_buffer;
void *l2_buffer_wgt_upd;

/* Read button */
static const pi_gpio_e gpio_boot_pin_1 = PAD_GPIO_UPB;


#define tinytestsize 10
// #define tinytestsize 1
static MFCC_IN_TYPE *MfccInSig_buff[tinytestsize];

static void *L3_wavs = NULL;

static float ce_loss_pre;
static float ce_loss_post;
static float ce_loss_pre_val;
static float ce_loss_post_val;
static float correct_pre_val;
static float correct_post_val;


char yes[N_MFCC_MELS * N_MFCC_WINS];
char no[N_MFCC_MELS * N_MFCC_WINS]; 
char up[N_MFCC_MELS * N_MFCC_WINS];
char down[N_MFCC_MELS * N_MFCC_WINS];
char left[N_MFCC_MELS * N_MFCC_WINS];
char right[N_MFCC_MELS * N_MFCC_WINS];
char on[N_MFCC_MELS * N_MFCC_WINS];
char off[N_MFCC_MELS * N_MFCC_WINS];
char stop[N_MFCC_MELS * N_MFCC_WINS];
char go[N_MFCC_MELS * N_MFCC_WINS];






void evaluate_validation(int was_trained){

    for (int classidx=2; classidx<N_CLASSES; classidx++){

        classidx = 7; // CIOFLANC: Tests

        printf("Started evaluating class %i in pre=%i mode\n", classidx, was_trained);

        int sampleidx = 0;
        while (sampleidx < 350) {

            if (sampleidx % 100 == 0){
                printf ("Now evaluating sample %i\n", sampleidx);
            }
            MFCC_IN_TYPE * MfccInSig = (MFCC_IN_TYPE *) pi_l2_malloc(AUDIO_BUFFER_SIZE * sizeof(MFCC_IN_TYPE));
            if (MfccInSig == NULL){
                printf("Failed allocating MfccInSig.\n");
                pmsis_exit(-1);
            }
            // validation.h: val_class_2
            // testing.h: test_class_2
            switch(classidx){
                case 2:
                    wav_to_array(test_class_2, MfccInSig, 0, 0);
                    break;
                case 3:
                    wav_to_array(test_class_3, MfccInSig, 0, 0);
                    break;
                case 4:
                    wav_to_array(test_class_4, MfccInSig, 0, 0);
                    break;
                case 5:
                    wav_to_array(test_class_5, MfccInSig, 0, 0);
                    break;
                case 6:
                    wav_to_array(test_class_6, MfccInSig, 0, 0);
                    break;
                case 7:
                    wav_to_array(test_class_7, MfccInSig, 0, 0);
                    break;
                case 8:
                    wav_to_array(test_class_8, MfccInSig, 0, 0);
                    break;
                case 9:
                    wav_to_array(test_class_9, MfccInSig, 0, 0);
                    break;
                case 10:
                    wav_to_array(test_class_10, MfccInSig, 0, 0);
                    break;
                case 11:
                    wav_to_array(test_class_11, MfccInSig, 0, 0);
                    break;
            }

            int noisesamplestart = 0;                
            for (int samplepos = 0; samplepos < AUDIO_BUFFER_SIZE; samplepos++){
                // MfccInSig[samplepos] = MfccInSig[samplepos] + 1*RecordedNoise[noisesamplestart+samplepos]; // CIOFLANC: Add RecordedNoise
                MfccInSig[samplepos] = 0;
            }

            preprocess(MfccInSig, l2_buffer, mfcc_src);

            // Extract backbone features
            void *dump; // dump to copy FC weights, won't be used; TODO: Parametrize DORY
            network_run(l2_buffer, L2_MEMORY_SIZE, l2_buffer, &dump, 0, 1); // L2_input_h extra-arg for L2-only

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

            int predidx = -1;
            float ce_loss = 0.;

            unsigned int args_inference_classifier[6];
            args_inference_classifier[0] = (unsigned int) l2_buffer;
            args_inference_classifier[1] = (unsigned int) l2_buffer_wgt_upd;
            args_inference_classifier[2] = (unsigned int) 3; // evaluate
            args_inference_classifier[3] = (unsigned int) classidx; // tinytest already ordered
            args_inference_classifier[4] = (float *) &ce_loss;
            args_inference_classifier[5] = (int *) &predidx;

            pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, net_step, args_inference_classifier));
            pi_cluster_close(&cluster_dev);

            // printf("CE loss: %f\n", ce_loss);

            if (was_trained == 0){
                ce_loss_pre_val += (ce_loss < 0) ? -ce_loss : ce_loss;
                correct_pre_val += (predidx == classidx);
            }
            else{
                ce_loss_post_val += (ce_loss < 0) ? -ce_loss : ce_loss;
                correct_post_val += (predidx == classidx);
            }

            sampleidx++;

        }

        printf("ce_loss_pre_val=%f, correct_pre_val=%f\n", ce_loss_pre_val, correct_pre_val);
        printf("ce_loss_post_val=%f, correct_post_val=%f\n", ce_loss_post_val, correct_post_val);

        break;
        
    }


    return 0; 
}



void evaluate_tinytest(int was_trained){

    if (uttr_train_src == ONLINE && was_trained == 0){
        for (int tinytestidx = 0; tinytestidx < tinytestsize; tinytestidx++){
            MfccInSig_buff[tinytestidx] = (MFCC_IN_TYPE *) pi_l2_malloc(AUDIO_BUFFER_SIZE * sizeof(MFCC_IN_TYPE));
        }
    }

    for (int tinytestidx = 0; tinytestidx < tinytestsize; tinytestidx++){ // only non-unknown
        // printf ("-----------------------------Loop evaluation (itteration %i)-------------------------\n", tinytestidx);

        #ifdef MEASURE
        int start_readeval = pi_time_get_us();
        #endif

        int savewav = 0;
        #ifdef DEBUG
        savewav = 1; 
        #endif

        MFCC_IN_TYPE *MfccInSig;
        OUT_TYPE *MfccOutSig;

        if (uttr_train_src == OFFLINE) {

            // Read from WAV       
            switch(tinytestidx){
                case 0:
                    // printf("Ground truth: yes\n");
                    break;
                case 1:
                    // printf("Ground truth: no\n");
                    break;
                case 2:
                    // printf("Ground truth: up\n");
                    break;
                case 3:
                    // printf("Ground truth: down\n");
                    break;
                case 4:
                    // printf("Ground truth: left\n");
                    break;
                case 5:
                    // printf("Ground truth: right\n");
                    break;
                case 6:
                    // printf("Ground truth: on\n");
                    break;    
                case 7:
                    // printf("Ground truth: off\n");
                    break;    
                case 8:
                    // printf("Ground truth: stop\n");
                    break;    
                case 9:
                    // printf("Ground truth: go\n");
                    break;                                                                                                                                                            
            }

            short int *prepWav = NULL;
            prepWav = (short int *) pi_l2_malloc(AUDIO_BUFFER_SIZE * sizeof(short));

            ram_read(prepWav, L3_wavs + (100+tinytestidx)*AUDIO_BUFFER_SIZE*sizeof(short), AUDIO_BUFFER_SIZE*sizeof(short));

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

        }
        else if (uttr_train_src == ONLINE) {

            // Read from MIC
            MfccInSig = (MFCC_IN_TYPE *) pi_l2_malloc(AUDIO_BUFFER_SIZE * sizeof(MFCC_IN_TYPE));

            if (MfccInSig == NULL){
                printf("Failed allocating MfccInSig in evaluate_tinytest.\n");
                pmsis_exit(-1);
            }            

            if (was_trained == 0) {

                switch(tinytestidx){
                    case 0:
                        printf("---------------- Pronounce yes --------------\n");
                        break;
                    case 1:
                        printf("---------------- Pronounce no --------------\n");
                        break;
                    case 2:
                        printf("---------------- Pronounce up --------------\n");
                        break;
                    case 3:
                        printf("---------------- Pronounce down --------------\n");
                        break;
                    case 4:
                        printf("---------------- Pronounce left --------------\n");
                        break;
                    case 5:
                        printf("---------------- Pronounce right --------------\n");
                        break;
                    case 6:
                        printf("---------------- Pronounce on --------------\n");
                        break;    
                    case 7:
                        printf("---------------- Pronounce off --------------\n");
                        break;    
                    case 8:
                        printf("---------------- Pronounce stop --------------\n");
                        break;    
                    case 9:
                        printf("---------------- Pronounce go --------------\n");
                        break;                                                                                                                                                            
                }

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
            // #ifdef AUDIO_EVK
            //             pi_gpio_pin_write(gpio_pin_o, 1);
            // #endif
        }

        
        if (uttr_train_src == OFFLINE) {
            int noisesamplestart = 0;
            for (int samplepos = 0; samplepos < AUDIO_BUFFER_SIZE; samplepos++){
                // MfccInSig[samplepos] = MfccInSig[samplepos] + 1*RecordedNoise[noisesamplestart+samplepos];
                MfccInSig[samplepos] = MfccInSig[samplepos]; // CIOFLANC: Add RecordedNoise
            }
        }

        // if (savewav){
        //     int16_t *MfccInSig_int16 = (int16_t *) pi_l2_malloc (sizeof(int16_t) * AUDIO_BUFFER_SIZE);
        //     int16_t *RecordedNoise_int16 = (int16_t *) pi_l2_malloc (sizeof(int16_t) * AUDIO_BUFFER_SIZE);
        //     for (int i = 0; i < AUDIO_BUFFER_SIZE; i++){
        //         MfccInSig_int16[i] = (int16_t) (MfccInSig[i] * (1<<15));
        //         RecordedNoise_int16[i] = (int16_t) (RecordedNoise[i] * (1<<15));
        //     }
        //     // Save the noise-augmented recording
        //     dump_wav_open("utter.wav", 16, 16000, 1, sizeof(int16_t) * AUDIO_BUFFER_SIZE);
        //     dump_wav_write(MfccInSig_int16, sizeof(int16_t) * AUDIO_BUFFER_SIZE);
        //     dump_wav_close();

        //     PRINTF("Writing wav file to utter.wav completed successfully\n");
        //     for (int samplepos = 0; samplepos < AUDIO_BUFFER_SIZE; samplepos++){
        //         MfccInSig_int16[samplepos] = (int16_t)(MfccInSig[samplepos] * (1<<15)) + 1*RecordedNoise_int16[noisesamplestart+samplepos];
        //     }
        //     // Save the noise-augmented recording
        //     dump_wav_open("utter_noise.wav", 16, 16000, 1, sizeof(int16_t) * AUDIO_BUFFER_SIZE);
        //     dump_wav_write(MfccInSig_int16, sizeof(int16_t) * AUDIO_BUFFER_SIZE);
        //     dump_wav_close();
        //     pi_l2_free(MfccInSig_int16, sizeof(int16_t) * AUDIO_BUFFER_SIZE);
        //     pi_l2_free(RecordedNoise_int16, sizeof(int16_t) * AUDIO_BUFFER_SIZE);

        //     PRINTF("Writing wav file to utter_noise.wav completed successfully\n");
        // }

        #ifdef MEASURE
        int end_readeval = pi_time_get_us();
        printf ("Loaded and scaled data for evaluation: %i us\n", end_readeval - start_readeval);
        #endif

        preprocess(MfccInSig, l2_buffer, mfcc_src);

        if (uttr_train_src == ONLINE) {
            for (int k = 0; k < N_MFCC_WINS * N_MELS; k++){
                // Data saving to elude re-recording the evaluation samples. TODO: organize workflow
                if (was_trained == 0) {
                    switch (tinytestidx) {
                        case 0:
                            yes[k] = ((uint8_t *)l2_buffer)[k];
                            break;
                        case 1: 
                            no[k] = ((uint8_t *)l2_buffer)[k];
                            break;
                        case 2:
                            up[k] = ((uint8_t *)l2_buffer)[k];
                            break;
                        case 3:
                            down[k] = ((uint8_t *)l2_buffer)[k];
                            break;
                        case 4:
                            left[k] = ((uint8_t *)l2_buffer)[k];
                            break;
                        case 5:
                            right[k] = ((uint8_t *)l2_buffer)[k];
                            break;
                        case 6:
                            on[k] = ((uint8_t *)l2_buffer)[k];
                            break;
                        case 7:
                            off[k] = ((uint8_t *)l2_buffer)[k];
                            break;
                        case 8:
                            stop[k] = ((uint8_t *)l2_buffer)[k];
                            break;
                        case 9:
                            go[k] = ((uint8_t *)l2_buffer)[k];
                            break;
                    } 
                }
                else{
                    switch (tinytestidx) {
                        case 0:
                            ((uint8_t *)l2_buffer)[k] = yes[k];
                            break;
                        case 1: 
                            ((uint8_t *)l2_buffer)[k] = no[k];
                            break;
                        case 2:
                            ((uint8_t *)l2_buffer)[k] = up[k];
                            break;
                        case 3:
                            ((uint8_t *)l2_buffer)[k] = down[k];
                            break;
                        case 4:
                            ((uint8_t *)l2_buffer)[k] = left[k];
                            break;
                        case 5:
                            ((uint8_t *)l2_buffer)[k] = right[k];
                            break;
                        case 6:
                            ((uint8_t *)l2_buffer)[k] = on[k];
                            break;
                        case 7:
                            ((uint8_t *)l2_buffer)[k] = off[k];
                            break;
                        case 8:
                            ((uint8_t *)l2_buffer)[k] = stop[k];
                            break;
                        case 9:
                            ((uint8_t *)l2_buffer)[k] = go[k];
                            break;
                    } 
                }
            }
        }

        // Extract backbone features
        void *dump; // dump to copy FC weights, won't be used; TODO: Parametrize DORY
        network_run(l2_buffer, L2_MEMORY_SIZE, l2_buffer, &dump, 0, 1); // L2_input_h extra-arg for L2-only

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
        
        int predidx = 0;
        float ce_loss = 0.;

        unsigned int args_inference_classifier[6];
        args_inference_classifier[0] = (unsigned int) l2_buffer;
        args_inference_classifier[1] = (unsigned int) l2_buffer_wgt_upd;
        args_inference_classifier[2] = (unsigned int) 3; // evaluate
        args_inference_classifier[3] = (unsigned int) tinytestidx + 2; // tinytest already ordered
        args_inference_classifier[4] = (float *) &ce_loss;
        args_inference_classifier[5] = (int *) &predidx;

        pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, net_step, args_inference_classifier));
        pi_cluster_close(&cluster_dev);

        printf("EVALUATE loss: %f\n", ce_loss);

        if (was_trained == 0){
            ce_loss_pre_val += (ce_loss < 0) ? -ce_loss : ce_loss;
            correct_pre_val += (predidx == (tinytestidx + 2));
        }
        else{
            ce_loss_post_val += (ce_loss < 0) ? -ce_loss : ce_loss;
            correct_post_val += (predidx == (tinytestidx + 2));
        }
        // #ifdef  AUDIO_EVK
        //     // block until next input audio frame is ready
        //     pi_gpio_pin_write(gpio_pin_o, 0);
        // #endif
    }
    if (uttr_train_src == ONLINE && was_trained == 1){
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
        // for (int uttridx = 0; uttridx < 2; uttridx++){ // debug/measurement
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
        // wav_to_array(utterance, MfccInSig, 0, 0);  // utterance, noise, save

        short int *prepWav = NULL;
        prepWav = (short int *) pi_l2_malloc(AUDIO_BUFFER_SIZE * sizeof(short));

        ram_read(prepWav, L3_wavs + ((classidx-2)*10+sampleidx)*AUDIO_BUFFER_SIZE*sizeof(short), AUDIO_BUFFER_SIZE*sizeof(short));

        MFCC_IN_TYPE *MfccInSig = (MFCC_IN_TYPE *) pi_l2_malloc(AUDIO_BUFFER_SIZE * sizeof(MFCC_IN_TYPE));

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

        // Reading wav for training takes: 2312 us

        int localaddnoise = 1; // always true
        if (localaddnoise){
            samplestart = 0; // TODO: random sample between (0, len(wav)-16000)
            for (int samplepos = 0; samplepos < AUDIO_BUFFER_SIZE; samplepos++){
                // MfccInSig[samplepos] = MfccInSig[samplepos] + 1*RecordedNoise[samplestart+samplepos]; // CIOFLANC: Add RecordedNoise
                MfccInSig[samplepos] = MfccInSig[samplepos]; 
            }
        }

        preprocess(MfccInSig, l2_buffer, mfcc_src);


        PRINTF ("********** Run inferecene **********\n");
        // Extract backbone features
        void *dump;
        network_run(l2_buffer, L2_MEMORY_SIZE, l2_buffer, &dump, 0, 1); // L2_input_h extra-arg for L2-only


        pi_cluster_conf_init(&cl_conf);
        pi_open_from_conf(&cluster_dev, &cl_conf);
        if (pi_cluster_open(&cluster_dev))
        {
          return -1;
        }

        int predidx = -1;
        float ce_loss = 0.;
        unsigned int args_train_classifier[6];
        args_train_classifier[0] = (unsigned int) l2_buffer;
        args_train_classifier[1] = (unsigned int) l2_buffer_wgt_upd;
        args_train_classifier[2] = (unsigned int) 2; // train
        // if (uttridx == 0 && epidx == 0)
        //     args_train_classifier[2] = (unsigned int) 1; // init = 1
        // else
        //     args_train_classifier[2] = (unsigned int) 0; // init = 0   
        args_train_classifier[3] = (unsigned int) classidx;
        args_train_classifier[4] = (float*) &ce_loss;
        args_train_classifier[5] = (int *) &predidx;

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

    // gpio_pin_measurement = PI_GPIO_A89; /* PI_GPIO_A02-PI_GPIO_A05 */
    // pi_gpio_flags_e flags = PI_GPIO_OUTPUT;
    // pi_pad_function_set(gpio_pin_measurement, 1);
    // pi_gpio_pin_configure(gpio_pin_measurement, flags);
    // pi_gpio_pin_write(gpio_pin_measurement, 0);
    // pi_gpio_pin_write(gpio_pin_measurement, 0);

    pi_pad_function_set(gpio_pin_measurement_id, 1);
    pi_gpio_pin_configure(gpio_pin_measurement_id, PI_GPIO_OUTPUT);
    pi_gpio_pin_write(gpio_pin_measurement_id, 0);
    pi_gpio_pin_write(gpio_pin_measurement_id, 0);



    PRINTF ("----------------------------- Initializing backbone ---------------------------\n");

    // Measurement start
    pi_gpio_pin_write(gpio_pin_measurement_id, 1);

    // Dory init
    mem_init();
    network_initialize(); // Absent in L2-only
    pi_cluster_close(&cluster_dev);


    // removed for measurements
    // printf ("----------------------------- Read WAVs from filesystem ---------------------------\n");

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

        // RAM write takes 701 us, RAM read takes 690
        ram_write(L3_wavs + i*AUDIO_BUFFER_SIZE*sizeof(short), inWav, AUDIO_BUFFER_SIZE*sizeof(short));
        
        // remove for measurements
        if (i%10 == 0){
            // printf(" %i/110 samples read.\n", i);
        }

        pi_l2_free(inWav, AUDIO_BUFFER_SIZE*sizeof(short));

    }

    printf ("Finished train allocation\n");

    // remove for measurements
    // printf("100/110 samples read.\n");


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
    printf ("Finished tinytest allocation\n");


    // remove for measurements
    int endwavreading = pi_time_get_us();
    // printf("110/110 samples read, WAV reading is complete in %d us.\n", endwavreading - startwavreading);
    


    // /* Remove RAM memory */
    // void network_terminate() {
    //   ram_free(L3_weights, L3_WEIGHTS_SIZE);
    //   ram_free(L3_input, L3_INPUT_SIZE);
    //   ram_free(L3_output, L3_OUTPUT_SIZE);
    // }


    // TODO: Comment in
    // DORY - TrainLib FC weights copy
    l2_buffer = pi_l2_malloc(L2_MEMORY_SIZE);
    if (l2_buffer == NULL) {
        printf("failed to allocate memory for l2_buffer\n");
    }
    void *dump;
    network_run(l2_buffer, L2_MEMORY_SIZE, l2_buffer, &dump, 0, 1); // L2_input_h extra-arg for L2-only

    // Run classifier
    pi_cluster_conf_init(&cl_conf);
    pi_open_from_conf(&cluster_dev, &cl_conf);
    if (pi_cluster_open(&cluster_dev))
    {
      return -1;
    }
    
    PRINTF ("----------------------------- Initializing classifier ---------------------------\n");

    l2_buffer_wgt_upd = pi_l2_malloc (64 *N_CLASSES * 4); // DSCNN S
    // l2_buffer_wgt_upd = pi_l2_malloc (172 *N_CLASSES * 4); // DSCNN M
    // l2_buffer_wgt_upd = pi_l2_malloc (276 *N_CLASSES * 4); // DSCNN L
    if (l2_buffer_wgt_upd == NULL) {
        printf("failed to allocate memory for l2_buffer_wgt_upd\n");
    }

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

    pi_gpio_pin_write(gpio_pin_measurement_id, 0);

    int button_was_pressed = 0;


    pi_evt_sig_init(&inference_task);

    microphone_setup();

    int iterations = 0;
    int sfu_out_buffer_cnt_prev = 0;
    int sfu_out_buffer_cnt_curr = 0;

    MFCC_IN_TYPE * MfccInSig;
    MFCC_IN_TYPE * MfccInSig_prev;
    int16_t *MfccInSig_int16;
    OUT_TYPE *MfccOutSig;

    MfccInSig_prev = (MFCC_IN_TYPE *) pi_l2_malloc(1 * AUDIO_BUFFER_SIZE * sizeof (MFCC_IN_TYPE));
    int len = 0;
    int upperlim = 0;
    int lowerlim = 0;

    printf ("----------------------------- Starting application ---------------------------\n");

    
    while (1){
    
        pi_gpio_pin_write(gpio_pin_measurement_id, 1);

        
        if (uttr_inf_src == ONLINE){

            PRINTF ("----------------------------- Start acquisition ---------------------------\n");    

            int start_dataacq = pi_time_get_us();    

            int threshold_counter = 0;


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


            // TODO: FIGURE OUT SCALING 2^32 or 2^31???

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

            // printf ("threshold_counter is: %i\n", threshold_counter);
            // TODO: FIGURE OUT NUMBER OF SAMPLES

            // Commented out for measurements
            // if (threshold_counter < 200){
            //     printf("silence\n");

            //     pi_l2_free(MfccInSig_int16, sizeof(int16_t) * AUDIO_BUFFER_SIZE);
            //     pi_l2_free(MfccInSig, sizeof(int16_t) * AUDIO_BUFFER_SIZE);


            //     int end_dataacq = pi_time_get_us(); 
            //     // printf("Data acquisition1: %i\n", end_dataacq - start_dataacq);

            //     // continue;
            //     goto checkbutton;
            // }

            int end_dataacq = pi_time_get_us(); 
            // printf("Data acquisition2: %i\n", end_dataacq - start_dataacq);

        }
        else if (uttr_inf_src == OFFLINE){

            PRINTF ("***************************** Reading wav *****************************\n");
            
            int start_readwav = pi_time_get_us();
            
            // Read the 0th .wav saved in RAM
            short int *prepWav = NULL;
            prepWav = (short int *) pi_l2_malloc(AUDIO_BUFFER_SIZE * sizeof(short));
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
            // printf("Time spent reading wav: %i\n", end_readwav - start_readwav);
        }

        // pi_gpio_pin_write(gpio_pin_measurement_id, 0);

        #ifdef PERF
        gap_fc_starttimer();
        gap_fc_resethwtimer();
        int start_timer_mfcc = gap_fc_readhwtimer();        
        int start_readmfcc = pi_time_get_us();
        #endif

        preprocess(MfccInSig, l2_buffer, mfcc_src);

        PRINTF ("***************************** Backbone inference **************************\n");


        int start_backbone = pi_time_get_us();
        
        
        // pi_gpio_pin_write(gpio_pin_measurement_id, 1);
        // Extract backbone features
        void *dump; // dump to copy FC weights, won't be used; TODO: Parametrize DORY
        network_run(l2_buffer, L2_MEMORY_SIZE, l2_buffer, &dump, 0, 1); // L2_input_h extra-arg for L2-only
        // pi_gpio_pin_write(gpio_pin_measurement_id, 0);

        #ifdef PERF
        int elapsed_timer_4 = gap_fc_readhwtimer() - start_timer_4;
        printf("Backbone inference: %d cycles\n", elapsed_timer_4);
        #endif

        for (int i=0; i < 64; i++){
            PRINTF("%i, ", ((uint8_t *) l2_buffer)[i]);
        }
        PRINTF("\n");

        int end_backbone = pi_time_get_us();
        // printf("Backbone: %i us\n", end_backbone - start_backbone);


        #ifdef PERF
        gap_fc_starttimer();
        gap_fc_resethwtimer();
        int start_timer_5 = gap_fc_readhwtimer();    
        #endif    

        PRINTF ("***************************** Classsifier inference **************************\n");

        // pi_gpio_pin_write(gpio_pin_measurement_id, 1);


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


        // Finished measurement
        // printf("***************************** Finished measurements *****************************\n");
       
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
        // printf("Classifier: %i us\n", end_classif - start_classif);
        
        
        // #ifdef  AUDIO_EVK
        //     // block until next input audio frame is ready
        //     pi_gpio_pin_write(gpio_pin_o, 0);
        // #endif

        checkbutton:
        button_was_pressed = 0;
        // button_was_pressed = read_button();
        button_was_pressed = 1; // measurement

        if (button_was_pressed){
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
            // evaluate before training


            ce_loss_pre = 0;
            ce_loss_post = 0;
            ce_loss_pre_val = 0;
            ce_loss_post_val = 0;
            correct_pre_val = 0;
            correct_post_val = 0;


            printf ("----------------------------- Pre-ODDA evaluation -----------------------------\n");
            // evaluate_tinytest(0);
            evaluate_validation(0);
            int endevaluationtime = pi_time_get_us();
            printf("Evaluation time: %i\n", endevaluationtime - evaluationtime);

            // pi_gpio_pin_write(gpio_pin_measurement_id, 0);

            printf("***************************** Finished pre-ODDA evaluation, now training... *****************************\n");


            // pi_gpio_pin_write(gpio_pin_measurement_id, 1);

            int traintime = pi_time_get_us();
            // train model with noisy data
            train_wavsrc();
            int endtraintime = pi_time_get_us();
            printf("Train time: %i\n", endtraintime - traintime);

            // pi_gpio_pin_write(gpio_pin_measurement_id, 0);

            printf ("----------------------------- Post-ODDA evaluation -----------------------------\n");


            // pi_gpio_pin_write(gpio_pin_measurement_id, 1);
            // evaluate improvement
            // evaluate_tinytest(1);
            evaluate_validation(1);
            // pi_gpio_pin_write(gpio_pin_measurement_id, 0);

            // if (noise_train_src == ONLINE){
            //    pi_l2_free(RecordedNoise, NOISE_LEN_S*AUDIO_BUFFER_SIZE * sizeof(MFCC_IN_TYPE)); // CIOFLANC: Add RecordedNoise
            // }


            // if (ce_loss_pre > ce_loss_post){
            //     printf("\x1B[32m *** Successfully reduced loss by %f from %f to %f *** \x1B[0m\n", ce_loss_pre-ce_loss_post, ce_loss_pre, ce_loss_post);
            // }
            // else{
            //     printf ("\x1B[31m *** Unsuccessful adaptation, try again. Loss increased from %f to %f *** \x1B[0m\n", ce_loss_pre, ce_loss_post);
            // }

            if (ce_loss_pre_val > ce_loss_post_val){
                printf("\x1B[32m *** Successfully reduced loss by %f from %f to %f *** \x1B[0m\n", ce_loss_pre_val-ce_loss_post_val, ce_loss_pre_val, ce_loss_post_val);
            }
            else{
                printf ("\x1B[31m *** Unsuccessful adaptation, try again. Loss increased from %f to %f *** \x1B[0m\n", ce_loss_pre_val, ce_loss_post_val);
            }

            if (correct_pre_val/350 * 100 < correct_post_val/350*100){
                printf("\x1B[32m *** Successfully increased accuracy by %f from %f to %f *** \x1B[0m\n", correct_post_val/350*100-correct_pre_val/350 * 100, correct_pre_val/350 * 100, correct_post_val/350*100);
            }
            else{
                printf ("\x1B[31m *** Unsuccessful adaptation, try again. Accuracy decreased from %f to %f *** \x1B[0m\n", correct_pre_val/350 * 100, correct_post_val/350*100);
            }

            // printf ("***************************** ODDA complete *****************************\n");

            pmsis_exit(0);
            return; // breaking loop early

        }

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
    mfcc_src = __XSTR(MFCC) == "0" ? 0 : 1;
    noise_train_src = __XSTR(NOISE_EVAL) == "0" ? 0 : 1;
    uttr_train_src = __XSTR(UTTR_EVAL) == "0" ? 0 : 1;
    uttr_inf_src = __XSTR(APPL) == "0" ? 0 : 1;

    return application();
}




// Example :)
// Everytime a buffer is finished, you re-enqueue itself. THen you pingpong the buffer out index
// KConfig/Menuconfig for SFU for example
// Memout comes from the PDM, Memin sends to the I2S
// when buffer received: pi_evt_push (to send a new event) -> in while(1) you wait for the event (pi_even_wait)
// pi_even_init() when I finished my comp task
