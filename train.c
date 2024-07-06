#include "train.h"

// Large validation set
// #include "validation.h"
#include "testing.h"

// DORY
#include "mem.h"
#include "network.h"

// PULP TrainLib
#include "net.h"


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
    return 0; 
}
