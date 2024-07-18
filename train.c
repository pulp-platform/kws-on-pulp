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


#include "train.h"

// Large validation set
// #include "validation.h"
#include "testing.h"

// DORY
#include "mem.h"
#include "network.h"

// PULP TrainLib
#include "net.h"


void train(){

    int sampleidx;
    int classidx;

    for (int epidx = 0; epidx < TRAIN_EPS; epidx++) {

        printf ("Epoch %i\n", epidx);

        for (int uttridx = 0; uttridx < 100; uttridx++){

        sampleidx = uttridx / 10;
        classidx = uttridx % 10 + 2; // no SILENCE, no UNKNOWN

        // printf ("sampleidx: %i\n", sampleidx);
        // printf ("classidx: %i\n", classidx);
        // printf ("position: %i\n", ((classidx-2)*10+sampleidx));

        // buggy sample ?!?!?!?
        if (((classidx-2)*10+sampleidx) == 34){
            continue; 
        }

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

        // Load utterance directly reading .wav
        // wav_to_array(utterance, MfccInSig, 0, 0);  // utterance, noise, save

        // Load utterance from RAM
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

        int noisesamplestart = 0;
        for (int samplepos = 0; samplepos < AUDIO_BUFFER_SIZE; samplepos++){
            // MfccInSig[samplepos] = MfccInSig[samplepos] + 1*RecordedNoise[noisesamplestart+samplepos];
            MfccInSig[samplepos] = MfccInSig[samplepos]; // CIOFLANC: Add RecordedNoise
        }

        preprocess(MfccInSig, l2_buffer, 0);

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
        // CIOFLANC: Should we INITIALIZE the weights again?
        // if (uttridx == 0 && epidx == 0)
        //     args_train_classifier[2] = (unsigned int) 1; // init = 1
        // else
        //     args_train_classifier[2] = (unsigned int) 0; // init = 0   
        args_train_classifier[3] = (unsigned int) classidx;
        args_train_classifier[4] = (float*) &ce_loss;
        args_train_classifier[5] = (int *) &predidx;

        pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, net_step, args_train_classifier));
        pi_cluster_close(&cluster_dev);
        }
    }
}


void evaluate_online(int was_trained){

    if (was_trained == 0){
        for (int tinytestidx = 0; tinytestidx < N_TINYTEST; tinytestidx++){
            MfccInSig_buff[tinytestidx] = (MFCC_IN_TYPE *) pi_l2_malloc(AUDIO_BUFFER_SIZE * sizeof(MFCC_IN_TYPE));
        }
    }
    for (int tinytestidx = 0; tinytestidx < N_TINYTEST; tinytestidx++){ // only non-unknown
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
            pi_evt_wait(&inference_task);

            // read recording
            printf("Filling input buffer...\n");  
            int mfccidx = 0;
            for (int i = 0; i < BUFF_SIZE; i+=3){
                MfccInSig[mfccidx] = (MFCC_IN_TYPE) (((float)((int32_t *)BufferInList)[i]) / (float)(1<<31 - 1));
                MfccInSig_buff[tinytestidx][mfccidx] = MfccInSig[mfccidx];
                mfccidx++;
            }
        }
        else {
            for (int i = 0; i < AUDIO_BUFFER_SIZE; i++){
                MfccInSig[i] = MfccInSig_buff[tinytestidx][i];
            }
        }
        // #ifdef AUDIO_EVK
        //             pi_gpio_pin_write(gpio_pin_o, 1);
        // #endif

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

        preprocess(MfccInSig, l2_buffer, 0);

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

    if ( was_trained == 1){
        for (int tinytestidx = 0; tinytestidx < N_TINYTEST; tinytestidx++){
            pi_l2_free(MfccInSig_buff[tinytestidx], AUDIO_BUFFER_SIZE * sizeof(MFCC_IN_TYPE));
        }
    }

    if (was_trained == 1) {
        if (ce_loss_pre_val > ce_loss_post_val){
            printf("\x1B[32m *** Successfully reduced loss by %f from %f to %f *** \x1B[0m\n", ce_loss_pre_val-ce_loss_post_val, ce_loss_pre_val, ce_loss_post_val);
        }
        else{
            printf ("\x1B[31m *** Unsuccessful adaptation, try again. Loss increased from %f to %f *** \x1B[0m\n", ce_loss_pre_val, ce_loss_post_val);
        }

        if (correct_pre_val * 100 < correct_post_val*100){
            printf("\x1B[32m *** Successfully increased accuracy by %f%% from %f%% to %f%% *** \x1B[0m\n", correct_post_val*10-correct_pre_val * 10, correct_pre_val * 10, correct_post_val*10);
        }
        else{
            printf ("\x1B[31m *** Unsuccessful adaptation, try again. Accuracy decreased from %f%% to %f%% *** \x1B[0m\n", correct_pre_val * 10, correct_post_val*10);
        }
    }


}


void evaluate_tinytest(int was_trained){

    for (int tinytestidx = 0; tinytestidx < N_TINYTEST; tinytestidx++){ // only non-unknown
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

        ram_read(prepWav, L3_wavs + (100 + tinytestidx)*AUDIO_BUFFER_SIZE*sizeof(short), AUDIO_BUFFER_SIZE*sizeof(short));

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

        
        
        int noisesamplestart = 0;
        for (int samplepos = 0; samplepos < AUDIO_BUFFER_SIZE; samplepos++){
            // MfccInSig[samplepos] = MfccInSig[samplepos] + 1*RecordedNoise[noisesamplestart+samplepos];
            MfccInSig[samplepos] = MfccInSig[samplepos]; // CIOFLANC: Add RecordedNoise
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

        preprocess(MfccInSig, l2_buffer, 0);

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
        args_inference_classifier[2] = (int) 3; // evaluate
        args_inference_classifier[3] = (int) 2 + tinytestidx; // tinytest already ordered
        args_inference_classifier[4] = (float *) &ce_loss;
        args_inference_classifier[5] = (int *) &predidx;

        pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, net_step, args_inference_classifier));
        pi_cluster_close(&cluster_dev);

        printf("Loss on tinytest: %f\n", ce_loss);

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

    if (was_trained == 1) {
        if (ce_loss_pre_val > ce_loss_post_val){
            printf("\x1B[32m *** Successfully reduced loss by %f from %f to %f *** \x1B[0m\n", ce_loss_pre_val-ce_loss_post_val, ce_loss_pre_val, ce_loss_post_val);
        }
        else{
            printf ("\x1B[31m *** Unsuccessful adaptation, try again. Loss increased from %f to %f *** \x1B[0m\n", ce_loss_pre_val, ce_loss_post_val);
        }

        if (correct_pre_val * 100 < correct_post_val*100){
            printf("\x1B[32m *** Successfully increased accuracy by %f%% from %f%% to %f%% *** \x1B[0m\n", correct_post_val*10-correct_pre_val * 10, correct_pre_val * 10, correct_post_val*10);
        }
        else{
            printf ("\x1B[31m *** Unsuccessful adaptation, try again. Accuracy decreased from %f%% to %f%% *** \x1B[0m\n", correct_pre_val * 10, correct_post_val*10);
        }
    }
}


void evaluate_largetest(int was_trained){

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

            preprocess(MfccInSig, l2_buffer, 0);

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
    }
    

    if (was_trained == 1) {
        if (ce_loss_pre_val > ce_loss_post_val){
            printf("\x1B[32m *** Successfully reduced loss by %f from %f to %f *** \x1B[0m\n", ce_loss_pre_val-ce_loss_post_val, ce_loss_pre_val, ce_loss_post_val);
        }
        else{
            printf ("\x1B[31m *** Unsuccessful adaptation, try again. Loss increased from %f to %f *** \x1B[0m\n", ce_loss_pre_val, ce_loss_post_val);
        }

        if (correct_pre_val * 100 < correct_post_val*100){
            printf("\x1B[32m *** Successfully increased accuracy by %f%% from %f%% to %f%% *** \x1B[0m\n", correct_post_val*10-correct_pre_val * 10, correct_pre_val * 10, correct_post_val*10);
        }
        else{
            printf ("\x1B[31m *** Unsuccessful adaptation, try again. Accuracy decreased from %f%% to %f%% *** \x1B[0m\n", correct_pre_val * 10, correct_post_val*10);
        }
    }
}
