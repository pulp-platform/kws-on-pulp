#include "evaluate.h"



void evaluate_validation(int was_trained, int mfcc_src){

    int n_classes = 12;
    for (int classidx=2; classidx<n_classes; classidx++){

        classidx = 7; // MEASUREMENT: hardcoded

        printf("Started evaluating class %i in pre=%i mode\n", classidx, was_trained);

        int sampleidx = 0;
        while (sampleidx < 350) {
            if (sampleidx%100 == 0){
                printf ("Now evaluting sample %i\n", sampleidx);
            }

            // Read .wav
            header_struct header_info;
            short int *inWav = NULL;
            inWav    = (short int *) pi_l2_malloc(AUDIO_BUFFER_SIZE * sizeof(short)); 
            if (inWav == NULL){
                printf("Failed allocating inWav.\n");
                pmsis_exit(-1);
            }

            // HARDCODED: test_class_n is hardcoded as test_class_7

            // Define validation class
            switch(classidx){
                case 2:
                    if (ReadWavFromFile(test_class_7[sampleidx], inWav, AUDIO_BUFFER_SIZE*sizeof(short), &header_info)){
                        printf("Error reading wav file\n");
                        pmsis_exit(1);
                    }
                    break;
                case 3:
                    if (ReadWavFromFile(test_class_7[sampleidx], inWav, AUDIO_BUFFER_SIZE*sizeof(short), &header_info)){
                        printf("Error reading wav file\n");
                        pmsis_exit(1);
                    }
                    break;
                case 4:
                    if (ReadWavFromFile(test_class_7[sampleidx], inWav, AUDIO_BUFFER_SIZE*sizeof(short), &header_info)){
                        printf("Error reading wav file\n");
                        pmsis_exit(1);
                    }
                    break;
                case 5:
                    if (ReadWavFromFile(test_class_7[sampleidx], inWav, AUDIO_BUFFER_SIZE*sizeof(short), &header_info)){
                        printf("Error reading wav file\n");
                        pmsis_exit(1);
                    }
                    break;
                case 6:
                    if (ReadWavFromFile(test_class_7[sampleidx], inWav, AUDIO_BUFFER_SIZE*sizeof(short), &header_info)){
                        printf("Error reading wav file\n");
                        pmsis_exit(1);
                    }
                    break;
                case 7:
                    if (ReadWavFromFile(test_class_7[sampleidx], inWav, AUDIO_BUFFER_SIZE*sizeof(short), &header_info)){
                        printf("Error reading wav file\n");
                        pmsis_exit(1);
                    }
                    break;
                case 8:
                    if (ReadWavFromFile(test_class_7[sampleidx], inWav, AUDIO_BUFFER_SIZE*sizeof(short), &header_info)){
                        printf("Error reading wav file\n");
                        pmsis_exit(1);
                    }
                    break;
                case 9:
                    if (ReadWavFromFile(test_class_7[sampleidx], inWav, AUDIO_BUFFER_SIZE*sizeof(short), &header_info)){
                        printf("Error reading wav file\n");
                        pmsis_exit(1);
                    }
                    break;
                case 10:
                    if (ReadWavFromFile(test_class_7[sampleidx], inWav, AUDIO_BUFFER_SIZE*sizeof(short), &header_info)){
                        printf("Error reading wav file\n");
                        pmsis_exit(1);
                    }
                    break;
                case 11:
                    if (ReadWavFromFile(test_class_7[sampleidx], inWav, AUDIO_BUFFER_SIZE*sizeof(short), &header_info)){
                        printf("Error reading wav file\n");
                        pmsis_exit(1);
                    }
                    break;
            }

            MFCC_IN_TYPE *MfccInSig = NULL;
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
                    MfccInSig[i]] = (MFCC_IN_TYPE) gap_fcip(((int) inWav[i]), 15);
                }
            #endif

            pi_l2_free(inWav, AUDIO_BUFFER_SIZE * sizeof(short));

            int noisesamplestart = 0;   
            for (int samplepos = 0; samplepos < AUDIO_BUFFER_SIZE; samplepos++){
                MfccInSig[samplepos] = MfccInSig[samplepos] + 1*RecordedNoise[noisesamplestart+samplepos];
            }

            compute_mfcc();  
            pi_l2_free(MfccInSig, AUDIO_BUFFER_SIZE * sizeof(MFCC_IN_TYPE)); 
            

            feat_char = (char*) pi_l2_malloc(49 * 10 * sizeof(char));
            int k = 0;
            for (int i = 0; i < 49 * N_MELS;i++){                
                
                // feat_char[k] = (char) ((int) floor(out_feat[i] * pow(2, -1) * sqrt(0.05)) + 128);
                feat_char[k] = (char) ((int) floor(out_feat[i] * 0.1118) + 128);

                if (N_MELS == 40){
                    // Select 10 MFCC per window
                    if (i == 40*(k/10) + 9){
                        i = 40*(k/10) + 39;
                    }
                }
                // Fill input buffer
                if (mfcc_src == OFFLINE){
                    ((uint8_t *)l2_buffer)[k] = L2_input_h[k]; // Precomputed MFCC
                }
                else {
                    ((uint8_t *)l2_buffer)[k] = feat_char[k]; // Online computed MFCC
                }

                k++;
            } 

            pi_l2_free(out_feat, 49 * N_MELS * sizeof(OUT_TYPE));
            pi_l2_free(feat_char, 49 * 10 * sizeof(char));

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
            unsigned int args_inference_classifier[6];
            args_inference_classifier[0] = (unsigned int) l2_buffer;
            args_inference_classifier[1] = (unsigned int) L2_FC_weights_float;
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

        break;   
    }
    return 0; 
}



void evaluate_tinytest(int was_trained, int utter_eval_src, int noise_eval_src, int mfcc_src){

    if (was_trained == 0){
        for (int tinytestidx = 0; tinytestidx < tinytestsize; tinytestidx++){
            MfccInSig_buff[tinytestidx] = (MFCC_IN_TYPE *) pi_l2_malloc(AUDIO_BUFFER_SIZE * sizeof(MFCC_IN_TYPE));
        }
    }

    // Iterate through samples
    for (int tinytestidx = 0; tinytestidx < tinytestsize; tinytestidx++){ // only non-unknown

    	#ifdef MEASURE
        int start_readeval = pi_time_get_us();
        #endif

        #ifdef DEBUG
        int save_rec = 0; 
        #endif

        if (utter_eval_src == OFFLINE) {
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
            n_utter_train = 100; // HARDCODED
            ram_read(prepWav, L3_wavs + (n_utter_train+tinytestidx)*AUDIO_BUFFER_SIZE*sizeof(short), AUDIO_BUFFER_SIZE*sizeof(short));

            // Prepare MFCC input from .wav
            MFCC_IN_TYPE *MfccInSig = NULL;
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
        else if (utter_eval_src == ONLINE) {

            // Read from MIC
            MFCC_IN_TYPE *MfccInSig = NULL;
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
            else {
                for (int i = 0; i < AUDIO_BUFFER_SIZE; i++){
                    MfccInSig[i] = MfccInSig_buff[tinytestidx][i];
                }
            }


// #ifdef AUDIO_EVK
//             pi_gpio_pin_write(gpio_pin_o, 1);
// #endif
        }

        int noisesamplestart = 0; // TODO: select random start
        if (uttr_eval_input == OFFLINE) {
            for (int samplepos = 0; samplepos < AUDIO_BUFFER_SIZE; samplepos++){
                MfccInSig[samplepos] = MfccInSig[samplepos] + 1*RecordedNoise[noisesamplestart+samplepos];
            }
        }

        if (save_rec){

            int16_t *MfccInSig_int16 = (int16_t *) pi_l2_malloc (sizeof(int16_t) * AUDIO_BUFFER_SIZE);
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

        #ifdef MEASURE
        int end_readeval = pi_time_get_us();
        printf ("Loaded and scaled data for evaluation: %i us\n", end_readeval - start_readeval);
        #endif

        // MFCC computation
        compute_mfcc();

        feat_char = (char*) pi_l2_malloc(49 * 10 * sizeof(char));
        int k = 0;
        for (int i = 0; i < 49 * N_MELS;i++){                
            
            // feat_char[k] = (char) ((int) floor(out_feat[i] * pow(2, -1) * sqrt(0.05)) + 128); // 23.883617 QSNR w/ float
            feat_char[k] = (char) ((int) floor(out_feat[i] * 0.1118) + 128);

            if (N_MELS == 40){
                // Select 10 MFCC per window
                if (i == 40*(k/10) + 9){
                    i = 40*(k/10) + 39;
                }
            }
            // Fill input buffer
            if (mfcc_src == OFFLINE){
                ((uint8_t *)l2_buffer)[k] = L2_input_h[k]; // Precomputed MFCC
            }
            else {
                ((uint8_t *)l2_buffer)[k] = feat_char[k]; // Online computed MFCC
            }
            k++;
        } 

        pi_l2_free(out_feat, 49 * N_MELS * sizeof(OUT_TYPE));
        pi_l2_free(feat_char, 49 * 10 * sizeof(char));

        for (int k = 0; k < 49 * 10; k++){
            // Data saving to elude re-recording the evaluation samples. TODO: organize workflow
            if (uttr_eval_input == ONLINE) {
                if (was_trained == 0){
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
                else {
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
        unsigned int args_inference_classifier[6];
        args_inference_classifier[0] = (unsigned int) l2_buffer;
        args_inference_classifier[1] = (unsigned int) L2_FC_weights_float;
        args_inference_classifier[2] = (unsigned int) 3; // evaluate
        args_inference_classifier[3] = (unsigned int) tinytestidx + 2; // tinytest already ordered
        args_inference_classifier[4] = (float *) &ce_loss;
        args_inference_classifier[5] = (int *) &predidx;

        pi_cluster_send_task_to_cl(&cluster_dev, pi_cluster_task(&cl_task, net_step, args_inference_classifier));
        pi_cluster_close(&cluster_dev);

        int classidx = tinytestidx + 2;
        if (was_trained == 0){
            ce_loss_pre_val += (ce_loss < 0) ? -ce_loss : ce_loss;
            correct_pre_val += (predidx == classidx);
        }
        else{
            ce_loss_post_val += (ce_loss < 0) ? -ce_loss : ce_loss;
            correct_post_val += (predidx == classidx);
        }

    // #ifdef  AUDIO_EVK
    //     // block until next input audio frame is ready
    //     pi_gpio_pin_write(gpio_pin_o, 0);
    // #endif

    }

    if (was_trained == 1){
        for (int tinytestidx = 0; tinytestidx < tinytestsize; tinytestidx++){
            pi_l2_free(MfccInSig_buff[tinytestidx], AUDIO_BUFFER_SIZE * sizeof(MFCC_IN_TYPE));
        }
    }
}