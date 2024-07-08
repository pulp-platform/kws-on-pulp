/**
 * INCLUDES
**/

#include "pulp_train.h"
#include "net.h"
#include "stats.h"

#include "init-defines.h"
#include "io_data.h"



/**
 * DATA
**/

// Define loss
PI_L1 float loss = 0;

// Define DNN blobs
PI_L1 struct blob layer0_in, layer0_wgt, layer0_out;

// Define DNN layer structures
PI_L1 struct vect_sum_args vect_sum_args;
PI_L1 struct vect_sum_args_fp16 vect_sum_args_fp16;
PI_L1 struct Linear_args l0_args;

// Define kernel tensors
PI_L1 float l0_ker[Tin_C_l0 * Tout_C_l0 * Tker_H_l0 * Tker_W_l0];

// Define kernel grad tensors
PI_L1 float l0_ker_diff[Tin_C_l0 * Tout_C_l0 * Tker_H_l0 * Tker_W_l0];

// Define I/O tensors
PI_L1 float l0_in[Tin_C_l0 * Tin_H_l0 * Tin_W_l0];
PI_L1 float l0_out[Tout_C_l0 * Tout_H_l0 * Tout_W_l0];
PI_L1 float bt_buffer[1];

// Define error propagation tensors
PI_L1 float l0_out_diff[Tout_C_l0 * Tout_H_l0 * Tout_W_l0];

// Define running parameters for normalization layers

// Loss function configuration structure
PI_L1 struct loss_args loss_args;


int predict_float_local (void * array, int n_classes){

  // Declare word list, determine recognized keyword
  // 'silence,unknown,yes,no,up,down,left,right,on,off,stop,go,'

  int idx;
  float max_val = -100.0;
  int max_idx = 0;
  char prediction[10];
  for (int i = 0; i < n_classes; i++){

    #ifdef VERBOSE
    printf ("d[%i] = %f\n", i, ((float*) array)[i]);
    #endif

    if (((float *) array)[i] > max_val){
      max_val = ((float*) array)[i];
      max_idx = i;
    }
  }

  idx = max_idx;
  switch (idx){
    case 0:
      strncpy(prediction, "silence", 10);
      // printf("The uttered keyword was: %s (%i).\n", "silence", idx);
      break;
    case 1:
      strncpy(prediction, "unknown", 10);
      // printf("The uttered keyword was: %s (%i).\n", "unknown", idx);
      break;
    case 2:
      strncpy(prediction, "yes", 10);
      // printf("The uttered keyword was: %s (%i).\n", "yes", idx);
      break;
    case 3:
      strncpy(prediction, "no", 10);
      // printf("The uttered keyword was: %s (%i).\n", "no", idx);
      break;
    case 4:
      strncpy(prediction, "up", 10);
      // printf("The uttered keyword was: %s (%i).\n", "up", idx);
      break;
    case 5:
      strncpy(prediction, "down", 10);
      // printf("The uttered keyword was: %s (%i).\n", "down", idx);
      break;
    case 6:
      strncpy(prediction, "left", 10);
      // printf("The uttered keyword was: %s (%i).\n", "left", idx);
      break;
    case 7:
      strncpy(prediction, "right", 10);
      // printf("The uttered keyword was: %s (%i).\n", "right", idx);
      break;
    case 8:
      strncpy(prediction, "on", 10);
      // printf("The uttered keyword was: %s (%i).\n", "on", idx);
      break;
    case 9:
      strncpy(prediction, "off", 10);
      // printf("The uttered keyword was: %s (%i).\n", "off", idx);
      break;
    case 10:
      strncpy(prediction, "stop", 10);
      // printf("The uttered keyword was: %s (%i).\n", "stop", idx);
      break;
    case 11:
      strncpy(prediction, "go", 10);
      // printf("The uttered keyword was: %s (%i).\n", "go", idx);
      break;
    default:
      printf ("Undefined class!\n");
  }

  printf("%s\n", prediction);
  return idx;
}


/**
 * DNN BACKEND FUNCTIONS
**/

// DNN initialization function
void DNN_init(float * weights)
{
  // Layer 0
  for(int i=0; i<Tin_C_l0*Tin_H_l0*Tin_W_l0; i++)			l0_in[i] = INPUT[i];
  for(int i=0; i<Tin_C_l0*Tout_C_l0*Tker_H_l0*Tker_W_l0; i++)		l0_ker[i] = weights[i];

  // Connect tensors to blobs


//Connecting linear
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
  l0_args.skip_wg_grad = 0;
  l0_args.skip_in_grad = 1;
  l0_args.opt_matmul_type_fw = MATMUL_TYPE_FW_L0;
  l0_args.opt_matmul_type_wg = MATMUL_TYPE_WG_L0;
  l0_args.opt_matmul_type_ig = MATMUL_TYPE_IG_L0;
  l0_args.use_biases = 0;
}


// Forward pass function
void forward()
{
  pulp_linear_fp32_fw_cl(&l0_args);
}

// Backward pass function
void backward()
{
  loss_args.output = &layer0_out;
  loss_args.target = LABEL;
  loss_args.wr_loss = &loss;
  pulp_CrossEntropyLoss_backward(&loss_args);
  pulp_linear_fp32_bw_param_grads_cl(&l0_args);
}

// Compute loss and output gradient
void compute_loss()
{
  loss_args.output = &layer0_out;
  loss_args.target = LABEL;
  loss_args.wr_loss = &loss;
  pulp_CrossEntropyLoss(&loss_args);
}

// Function to update the network
void update_weights()
{
  struct optim_args opt_l0;
  opt_l0.weights = &layer0_wgt;
  opt_l0.use_biases = 0;
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
void net_step(void *args)
{


  unsigned int * real_args = (unsigned int *) args;
  void * l2_buffer = (void *) real_args[0];
  void * upd_WGT_l0 = (void *) real_args[1];
  enum mode op_mode = (int) real_args[2];
  int classidx = (int) real_args[3];
  float *loss_ptr = (float*) real_args[4];
  int *predidx_ptr = (int *) real_args[5];

  // TODO: Discuss sample management per epoch
  float *L2_weights = (float *) pi_l2_malloc (WGT_SIZE_L0 * sizeof(float));
  if (op_mode == INITIALIZE) {        
    for (int i = 0; i < WGT_SIZE_L0; i++){
      L2_weights[i] = init_WGT_l0[i];
    }
  }
  else {
    for (int i = 0; i < WGT_SIZE_L0; i++){
      L2_weights[i] = (((float *) upd_WGT_l0)[i]);
    }  
  }

  float eps_in = 0.1802; // TODO: CMake argument
  // float eps_in = 1; // DEBUG
  // float eps_in = 0.01;
  for (int i = 0; i < IN_SIZE; i++){
      INPUT[i] = ((float) (((uint8_t *) l2_buffer)[i])) * eps_in;
  }


  // printf("Initializing network..\n");
  DNN_init(L2_weights);
  pi_l2_free(L2_weights, WGT_SIZE_L0 * sizeof(float));
  // printf("Testing DNN initialization forward..");

  if (op_mode == INFERENCE){

    forward();
    #ifdef VERBOSE
    printf("Forward\n");
    for (int i = 0; i < 12; i++){
      printf ("d[%i] = %f\n", i, ((float*) layer0_out.data)[i]);
    }
    #endif

    pulp_1dsoftmax_fp32_fw(&layer0_out);
    #ifdef VERBOSE
    printf("Softmax\n");
    for (int i = 0; i < 12; i++){
      printf ("d[%i] = %f\n", i, ((float*) layer0_out.data)[i]);
    }
    #endif
    *predidx_ptr = predict_float_local(layer0_out.data, 12);
  }


  if (op_mode == EVALUATE){

    forward();
    #ifdef VERBOSE
    printf("Forward\n");
    for (int i = 0; i < 12; i++){
      printf ("d[%i] = %f\n", i, ((float*) layer0_out.data)[i]);
    }
    #endif

    pulp_1dsoftmax_fp32_fw(&layer0_out);
    #ifdef VERBOSE
    printf("Softmax\n");
    for (int i = 0; i < 12; i++){
      printf ("d[%i] = %f\n", i, ((float*) layer0_out.data)[i]);
    }
    #endif
    *predidx_ptr = predict_float_local(layer0_out.data, 12);
    for (int i=0; i < layer0_out.dim; i++){
      if(layer0_out.data[i] == 0){
        layer0_out.data[i] += 1e-10;
      }
    }
    compute_loss();
    *loss_ptr = loss;

  }

  if (op_mode == TRAIN){

    // onehot encoding
    for (int labelidx = 0; labelidx<OUT_SIZE; labelidx++){
        LABEL[labelidx] = 0.;
    } 
    LABEL[classidx] = 1.;

    for (int epoch=0; epoch<EPOCHS; epoch++){
      forward();
      pulp_1dsoftmax_fp32_fw(&layer0_out);
      for (int i=0; i < layer0_out.dim; i++){
        if(layer0_out.data[i] == 0){
          layer0_out.data[i] += 1e-10;
        }
      }
      compute_loss();
      *loss_ptr = loss;
      backward();
      update_weights();
    }

    // Return updated weights
    for (int i = 0; i < WGT_SIZE_L0; i++) {
      // printf ("layer0_wgt.data[%i] = %f, ", i, layer0_wgt.data[i]);
      ((float*)upd_WGT_l0)[i] = layer0_wgt.data[i];
    }
    // printf ("\n");

  }
  // TODO: REMOVE???
  if (op_mode == INITIALIZE) {
    for (int i = 0; i < WGT_SIZE_L0; i++){
      ((float*)upd_WGT_l0)[i] = layer0_wgt.data[i];
    }
  }


}
