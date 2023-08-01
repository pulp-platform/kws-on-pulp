/**
 * INCLUDES
**/

#include "pulp_train.h"
#include "net.h"
#include "stats.h"

#include "initdefines.h"
#include "iodata.h"

#include "directional_allocator.h"


#include "AutoTilerLibTypes.h"
#include "DSP_Lib.h"



int predict_unsigned_local (void * array, int n_classes){

        // Declare word list, determine recognized keyword
        // 'silence,unknown,yes,no,up,down,left,right,on,off,stop,go,'

        int idx;
        unsigned int max_val = 0;
        int max_idx = 0;
        char prediction[10];
        for (int i = 0; i < n_classes; i++){
                printf ("d[%i] = %u\n", i, ((unsigned int*) array)[i]);

                if (((unsigned int *) array)[i] > max_val){
                        max_val = ((unsigned int*) array)[i];
                        max_idx = i;
                }
        }

        idx = max_idx;

        switch (idx){
                case 0:
                        strncpy(prediction, "silence", 10);
                        break;
                case 1:
                        strncpy(prediction, "unknown", 10);
                        break;
                case 2:
                        strncpy(prediction, "yes", 10);
                        break;
                case 3:
                        strncpy(prediction, "no", 10);
                        break;
                case 4:
                        strncpy(prediction, "up", 10);
                        break;
                case 5:
                        strncpy(prediction, "down", 10);
                        break;
                case 6:
                        strncpy(prediction, "left", 10);
                        break;
                case 7:
                        strncpy(prediction, "right", 10);
                        break;
                case 8:
                        strncpy(prediction, "on", 10);
                        break;
                case 9:
                        strncpy(prediction, "off", 10);
                        break;
                case 10:
                        strncpy(prediction, "stop", 10);
                        break;
                case 11:
                        strncpy(prediction, "go", 10);
                        break;
                default:
                        printf ("Undefined class!\n");
        }

        printf("The uttered keyword was: %s (%i).\n", prediction, idx);
        return idx;
}

int predict_float_local (void * array, int n_classes){

        // Declare word list, determine recognized keyword
        // 'silence,unknown,yes,no,up,down,left,right,on,off,stop,go,'

        int idx;
        float max_val = -100.0;
        int max_idx = 0;
        char prediction[10];
        for (int i = 0; i < n_classes; i++){
                printf ("d[%i] = %f\n", i, ((float*) array)[i]);

                if (((float *) array)[i] > max_val){
                        max_val = ((float*) array)[i];
                        max_idx = i;
                }
        }

        idx = max_idx;

        switch (idx){
                case 0:
                        strncpy(prediction, "silence", 10);
                        break;
                case 1:
                        strncpy(prediction, "unknown", 10);
                        break;
                case 2:
                        strncpy(prediction, "yes", 10);
                        break;
                case 3:
                        strncpy(prediction, "no", 10);
                        break;
                case 4:
                        strncpy(prediction, "up", 10);
                        break;
                case 5:
                        strncpy(prediction, "down", 10);
                        break;
                case 6:
                        strncpy(prediction, "left", 10);
                        break;
                case 7:
                        strncpy(prediction, "right", 10);
                        break;
                case 8:
                        strncpy(prediction, "on", 10);
                        break;
                case 9:
                        strncpy(prediction, "off", 10);
                        break;
                case 10:
                        strncpy(prediction, "stop", 10);
                        break;
                case 11:
                        strncpy(prediction, "go", 10);
                        break;
                default:
                        printf ("Undefined class!\n");
        }

        printf("The uttered keyword was: %s (%i).\n", prediction, idx);
        return idx;
}


/**
 * DATA
**/

// Define loss
PI_L1 float loss = 0;

// Define DNN blobs
PI_L1 struct blob layer0_in, layer0_wgt, layer0_out;

// Define DNN layer structures
PI_L1 struct Linear_args l0_args;

// Define kernel tensors
PI_L1 float l0_ker[Tin_C_l0 * Tout_C_l0 * Tker_H_l0 * Tker_W_l0];

// Define kernel grad tensors
PI_L1 float l0_ker_diff[Tin_C_l0 * Tout_C_l0 * Tker_H_l0 * Tker_W_l0];

// Define I/O tensors
PI_L1 float l0_in[Tin_C_l0 * Tin_H_l0 * Tin_W_l0];
PI_L1 float l0_out[Tout_C_l0 * Tout_H_l0 * Tout_W_l0];

// Define error propagation tensors
PI_L1 float l0_out_diff[Tout_C_l0 * Tout_H_l0 * Tout_W_l0];

// Loss function configuration structure
PI_L1 struct loss_args loss_args;



/**
 * DNN BACKEND FUNCTIONS
**/

// DNN initialization function
void DNN_init(float * weights)
{
    // Layer 0
    for(int i=0; i<Tin_C_l0*Tin_H_l0*Tin_W_l0; i++)			l0_in[i] = IN_DATA[i];
    for(int i=0; i<Tin_C_l0*Tout_C_l0*Tker_H_l0*Tker_W_l0; i++)		l0_ker[i] = weights[i];

    // Connect tensors to blobs
    layer0_in.data = l0_in;
    layer0_in.dim = Tin_C_l0*Tin_H_l0*Tin_W_l0;
    layer0_in.C = Tin_C_l0;
    layer0_in.H = Tin_H_l0;
    layer0_in.W = Tin_W_l0;
    layer0_wgt.data = l0_ker;
    layer0_wgt.diff = l0_ker_diff;
    layer0_wgt.dim = Tin_C_l0*Tout_C_l0*Tker_H_l0*Tker_W_l0;
    layer0_wgt.C = Tin_C_l0;
    layer0_wgt.H = Tker_H_l0;
    layer0_wgt.W = Tker_W_l0;
    layer0_out.data = l0_out;
    layer0_out.diff = l0_out_diff;
    layer0_out.dim = Tout_C_l0*Tout_H_l0*Tout_W_l0;
    layer0_out.C = Tout_C_l0;
    layer0_out.H = Tout_H_l0;
    layer0_out.W = Tout_W_l0;

    // Configure layer structures
    // Layer 0
    l0_args.input = &layer0_in;
    l0_args.coeff = &layer0_wgt;
    l0_args.output = &layer0_out;
    l0_args.skip_in_grad = 1;
    l0_args.opt_matmul_type_fw = MATMUL_TYPE_FW_L0;
    l0_args.opt_matmul_type_wg = MATMUL_TYPE_WG_L0;
    l0_args.opt_matmul_type_ig = MATMUL_TYPE_IG_L0;
}


// Forward pass function
void forward()
{
    pulp_linear_fp32_fw_cl(&l0_args);
}

// Backward pass function
void backward()
{
    pulp_linear_fp32_bw_cl(&l0_args);
}

// Compute loss and output gradient
void compute_loss()
{
    loss_args.output = &layer0_out;
    loss_args.target = LABEL;
    loss_args.wr_loss = &loss;
    // pulp_MSELoss(&loss_args);
    pulp_CrossEntropyLoss(&loss_args);
}

// Function to update the network
void update_weights()
{
    struct optim_args opt_l0;
    opt_l0.weights = &layer0_wgt;
    opt_l0.learning_rate = LEARNING_RATE;
    pi_cl_team_fork(NUM_CORES, pulp_gradient_descent_fp32, &opt_l0);
}



/**
 * DATA VISUALIZATION AND CHECK TOOLS
**/

// Function to print FW output
void print_output()
{
    printf("\nLayer 0 output:\n");

    for (int i=0; i<Tout_C_l0*Tout_H_l0*Tout_W_l0; i++)
    {
        printf("%f ", l0_out[i]);
        // Newline when an output row ends
        // if(!(i%Tout_W_l0)) printf("\n");
        // Newline when an output channel ends
        if(!(i%Tout_W_l0*Tout_H_l0)) printf("\n");
    }
}

// Function to check post-training output wrt Golden Model (GM)
void check_post_training_output()
{
    int integrity_check = 0;
    integrity_check = verify_tensor(l0_out, REFERENCE_OUTPUT, Tout_C_l0*Tout_H_l0*Tout_W_l0, TOLERANCE);
    if (integrity_check > 0)
        printf("\n*** UPDATED OUTPUT NOT MATCHING GOLDEN MODEL ***\n");
}



/**
 * DNN MODEL TRAINING
**/

// Call for a complete training step
// void net_step(void *l2_buffer, void *L3_weights_curr)

void net_step(void *args) {

    // TODO: Move all in application.c.
    // TODO: Add trainlib_example_dscnn back in .gitignore

    unsigned int * real_args = (unsigned int *) args;
    void * l2_buffer = (void *) real_args[0];
    void * L3_weights_curr = (void *) real_args[1];
    void * L2_weights_curr_updated = (void *) real_args[2];
    int update = (int) real_args[3]; // 1 - update
    int init = (int) real_args[4]; // 1 - initialize
    int classidx = (int) real_args[5];

    // TODO: Discuss sample management per epoch
    
    // TODO: INIT - take pretrained weights or used updated ones
    float *L2_weights = (float *) pi_l2_malloc (WGT_SIZE_L0 * sizeof(float));
    if (init == 1){        
        for (int i = 0; i < WGT_SIZE_L0; i++){
            L2_weights[i] = init_WGT_l0[i];
        }
    }
    else{
        for (int i = 0; i < WGT_SIZE_L0; i++){
            L2_weights[i] = (((float    *) L2_weights_curr_updated)[i]);
        }
        
    }


    // L2 Dory to L1 TrainLib manual feature movement
    // Librosa
    // float eps_in = 0.1142;
    // Tensorflow
    float eps_in = 0.1247;

    for (int i = 0; i < IN_SIZE; i++){
        IN_DATA[i] = ((float) (((uint8_t    *) l2_buffer)[i])) * eps_in;
    }

#ifdef VERBOSE
    for (int i = 0; i < 64; i++){
        printf("IN_DATA[%i]=%f,\n ", i, IN_DATA[i]);
    }
#endif

#ifdef VERBOSE
    printf("Original weights:\n");
    for (int i = 0; i < 10; i++){
        printf("W[%i] %f\n", i, init_WGT_l0[i]);
    }
#endif


    DNN_init(L2_weights);
    pi_l2_free(L2_weights, WGT_SIZE_L0 * sizeof(float));


    // UPDATE
    if (update == 1) {

#ifdef PROF_NET
        INIT_STATS();
        PRE_START_STATS();
        START_STATS();
#endif

        // onehot encoding
        for (int labelidx = 0; labelidx<OUT_SIZE; labelidx++){
            LABEL[labelidx] = 0.;
        } 
        LABEL[classidx] = 1.;

        int start = 0;
        int elapsed = 0;
        for (int epoch=0; epoch<EPOCHS; epoch++) {

            gap_cl_starttimer();
            gap_cl_resethwtimer();
            start = gap_cl_readhwtimer();

            forward();
            elapsed = gap_cl_readhwtimer() - start;
            printf("forward: %d\n", elapsed);
            gap_cl_starttimer();
            gap_cl_resethwtimer();
            start = gap_cl_readhwtimer();

#ifdef VERBOSE
            for (int i = 0; i < OUT_SIZE; i++){
                printf("Out[%i]=%f\n", i, ((float *)layer0_out.data)[i]);
            }
#endif

            compute_loss();

            elapsed = gap_cl_readhwtimer() - start;
            printf("compute loss: %d\n", elapsed);
            gap_cl_starttimer();
            gap_cl_resethwtimer();
            start = gap_cl_readhwtimer();

            backward();

            elapsed = gap_cl_readhwtimer() - start;
            printf("backward: %d\n", elapsed);
            gap_cl_starttimer();
            gap_cl_resethwtimer();
            start = gap_cl_readhwtimer();

            update_weights();

            elapsed = gap_cl_readhwtimer() - start;
            printf("update_weights: %d\n", elapsed);

        }

#ifdef VERBOSE        
        printf("Adapted weights:\n");
        for (int i = 0; i < 10; i++){
            printf("W[%i] %f\n", i, layer0_wgt.data[i]);
        }
#endif

        // Return updated weights
        for (int i = 0; i < WGT_SIZE_L0; i++) {
            // printf ("((float*)L2_weights_curr_updated)[%i]: %f\n", i, ((float*)L2_weights_curr_updated)[i]);
            // printf ("layer0_wgt.data[%i]: %f\n", i, layer0_wgt.data[i]);

            ((float*)L2_weights_curr_updated)[i] = layer0_wgt.data[i];
        }
        
#ifdef PROF_NET
        STOP_STATS();
#endif

        // Check and print updated output
        forward();

#ifdef VERBOSE
        printf("Checking updated output..\n");
        check_post_training_output();
        print_output();
#endif
    } // update

    else {
#ifdef VERBOSE
        printf("Testing DNN initialization forward..\n");

        for (int i = 0; i < 10; i++){
            printf("W[%i] = %f, ", i, init_WGT_l0[i]);
        }
        printf("\n");

        for (int i = 0; i < 10; i++){
            printf("IN_DATA[%i] %f, ", i, IN_DATA[i]);
        }
        printf("\n");
#endif

        forward();

        for (int i = 0; i < OUT_SIZE; i++){
            // ((uint8_t    *) l2_buffer)[i] = (uint8_t) ((l0_out[i])*255.0); 
            // ((int *) l2_buffer)[i] = ((uint16_t *)l0_out)[i]; 
            ((float *) l2_buffer)[i] = ((float *)l0_out)[i]; 
        }

        // onehot encoding
        for (int labelidx = 0; labelidx<OUT_SIZE; labelidx++){
            LABEL[labelidx] = 0.;
        } 
        if (classidx != 100){
            LABEL[classidx] = 1.;     
        }
        

        compute_loss();
        printf("Predicting local output\n");
        predict_float_local(l0_out, OUT_SIZE);

        // #ifdef VERBOSE 
        //     print_output();
        // #endif
    }



    // // TODO: INIT
    // if (init == 1) {
    //     for (int i = 0; i < WGT_SIZE_L0; i++){
    //         ((float*)L2_weights_curr_updated)[i] = layer0_wgt.data[i];
    //     }
    // }
}
