/**
 * INCLUDES
**/

#include "pulp_train.h"
#include "net.h"
#include "stats.h"

#include "initdefines.h"
#include "iodata.h"

#include "directional_allocator.h"


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
void DNN_init()
{
  // Layer 0
  for(int i=0; i<Tin_C_l0*Tin_H_l0*Tin_W_l0; i++)			l0_in[i] = IN_DATA[i];
  for(int i=0; i<Tin_C_l0*Tout_C_l0*Tker_H_l0*Tker_W_l0; i++)		l0_ker[i] = init_WGT_l0[i];

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
  pulp_MSELoss(&loss_args);
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

void net_step(void *args)
{

  // TODO: Move all in denoiser.c.
  // TODO: Add trainlib_example_dscnn back in .gitignore

  unsigned int * real_args = (unsigned int *) args;
  void * l2_buffer = (void *) real_args[0];
  void * L3_weights_curr = (void *) real_args[1];
  void * L2_weights_curr_updated = (void *) real_args[2];
  int update = (int) real_args[3]; // 1 - update
  int init = (int) real_args[4]; // 1 - initialize
  int classidx = (int) real_args[5];

  // TODO: Discuss sample management per epoch
  LABEL[0] = classidx;



  if (update == 1 || init == 1){

    // L2 Dory to L1 TrainLib manual weights movement
    // Weights size - 64 * 12 = WGT_SIZE_L0
    // Weights address - Wait for Dory to iterate and copy the data from there
    void *L2_weights = NULL;
    L2_weights = (uint8_t *) pi_l2_malloc(WGT_SIZE_L0 * sizeof(uint8_t));  
    cl_ram_read(L2_weights, L3_weights_curr, WGT_SIZE_L0);

    // L2 Dory to L1 TrainLib manual weights movement
    printf ("Training weights\n");
    for (int i = 0; i < WGT_SIZE_L0; i++){
        // printf ("d[%i] = %f\n", i, ((float) ((uint8_t  *) L2_weights)[i])/255 );
        // Dory operates INT8, must be converted to FLOAT
        init_WGT_l0[i] = ((float) ((uint8_t  *) L2_weights)[i])/255;
    }

    pi_l2_free(L2_weights, WGT_SIZE_L0 * sizeof(uint8_t));

  }
  else{
    for (int i = 0; i < WGT_SIZE_L0; i++){
        // printf ("d[%i] = %f\n", i, ((float) ((uint8_t  *) L2_weights)[i])/255 );
        // Dory operates INT8, must be converted to FLOAT
        init_WGT_l0[i] = ((float *) L2_weights_curr_updated)[i];
    }
  }

  // L2 Dory to L1 TrainLib manual feature movement
  printf ("Training features\n");
  int in_feat_classif = 64;
  for (int i = 0; i < in_feat_classif; i++){
      // printf ("d[%i] = %f\n", i, ((float) ((uint8_t  *) l2_buffer)[i])/255 );
      // Dory operates INT8, must be converted to FLOAT
      IN_DATA[i] = ((float) ((uint8_t  *) l2_buffer)[i])/255;
  }

#ifdef VERBOSE
  printf("Original weights:\n");
  for (int i = 0; i < 10; i++){
    printf("W[%i] %f\n", i, init_WGT_l0[i]);
  }
#endif
  printf("Initializing network..\n");
  DNN_init();

  if (update == 1){
    #ifdef PROF_NET
    INIT_STATS();
    PRE_START_STATS();
    START_STATS();
    #endif


    for (int epoch=0; epoch<EPOCHS; epoch++)
    {
      forward();
      compute_loss();
      backward();
      update_weights();
    }
#ifdef VERBOSE    
    printf("Adapted weights:\n");
    for (int i = 0; i < 10; i++){
      printf("W[%i] %f\n", i, layer0_wgt.data[i]);
    }
#endif

    for (int i = 0; i < WGT_SIZE_L0; i++){
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
  }
  else {
    printf("Testing DNN initialization forward..\n");
    forward();

  #ifdef VERBOSE 
    print_output();
  #endif
  }

  if (init == 1) {
    for (int i = 0; i < WGT_SIZE_L0; i++){
     ((float*)L2_weights_curr_updated)[i] = layer0_wgt.data[i];
    }
  }

}
