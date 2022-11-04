//#include "hyperram_aligned.h"
#include <pulp.h>
#include "mem_controller.h"
#include "network.h"
#include "pulp.h"
#include "dory.h"
#include "rt/rt_api.h"
#include "utils.h"
#include "kernels.h"
#include "layer_init.h"
#include "layerConvBNRelu8.h"
#include "layerConvBNRelu4.h"
#include "layerConvDWBNRelu3.h"
#include "layerConvBNRelu2.h"
#include "layerConvDWBNRelu9.h"
#include "layerConvDWBNRelu1.h"
#include "layerConvBNRelu6.h"
#include "layerConvBNRelu0.h"
#include "layerConvDWBNRelu5.h"
#include "layerConvDWBNRelu7.h"
#include "/usr/scratch/wetterhorn/cioflanc/kws_on_fpga/local_deployment/fully_tested_flasher/test/inc/hyperbus_test.h"
//#include "../src/hyperbus_test.c"

#define FLASH_BUFF_SIZE 128
#define VERBOSE 0
/*const char * L3_weights_files[] = {
  "ConvBNRelu0_weights.hex", "ConvDWBNRelu1_weights.hex", "ConvBNRelu2_weights.hex", "ConvDWBNRelu3_weights.hex", "ConvBNRelu4_weights.hex", "ConvDWBNRelu5_weights.hex", "ConvBNRelu6_weights.hex", "ConvDWBNRelu7_weights.hex", "ConvBNRelu8_weights.hex", "ConvDWBNRelu9_weights.hex"
};*/
int __rt_fpga_fc_frequency = 20000000; // e.g. 20000000 for 20MHz;
int __rt_fpga_periph_frequency = 10000000; // e.g. 10000000 for 10MHz;
unsigned int __rt_iodev_uart_baudrate = 115200;


int L3_weights_size[10];
int L3_weights_sa[10];
static int L3_weights;
static int activations_input;
static rt_hyperram_t* hyperram;
unsigned int flash_weight_size[11]={1056, 480, 2432, 960, 8960, 1920, 17152, 1920, 34304, 3840, 49152};



#ifdef VERBOSE
static void check_layer(char *output, int check_sum_true, int dim) {
  int checksum = 0;
  char *ptr = (char *) output;
  for(int j=0; j<dim; j++) {
    checksum += ptr[j];
  }

  if(check_sum_true == checksum)
    printf("Checksum in/out Layer :\tOk\n");
  else 
    printf("Checksum in/out Layer :\tFailed [%u vs. %u]\n", checksum, check_sum_true);
}


static void check_layer_weight(char *weight, int check_sum_true, int dim) {
  int checksum = 0;
  char *ptr = (char *) weight;
  for(int j=0; j<dim; j++) {
    checksum += ptr[j];
  }

  if(check_sum_true == checksum)
    printf("Checksum weight/bias Layer :\tOk\n");
  else 
    printf("Checksum weight/bias Layer :\tFailed [%u vs. %u]\n", checksum, check_sum_true);
}
#endif 

/* Moves the weights and the biases from hyperflash to hyperram */
int network_setup()
{

  unsigned int flash_start_addr = 0;
  unsigned int ram_start_addr = 0;
  unsigned int read_size = 0; 
  
  //L3_weights = (char *) rt_hyperram_alloc(hyperram, 5000000);
  unsigned int rdDone = 0;
  //l3_start_addr = (unsigned int) L3_weights;
  
  int flashBuffSize = FLASH_BUFF_SIZE * sizeof(char);
  void *flashBuffer = rt_alloc(RT_ALLOC_PERIPH, flashBuffSize);
  void *data_check  = rt_alloc(RT_ALLOC_PERIPH, flashBuffSize);
  
  for (int i=0;i<11;i++)
  {
    // loop on chunk in file
    while(rdDone < (flash_weight_size[i] / sizeof(char)))
    { 
      if( rdDone + flashBuffSize > flash_weight_size[i])
          read_size = flash_weight_size[i] - rdDone;
      else
          read_size = flashBuffSize;
          
      // read from HyperFlash
      udma_hyper_flash_setup();
      udma_hyper_dread(read_size, rdDone+flash_start_addr, (unsigned int) flashBuffer, 0, 0);
      udma_hyper_wait(0);

      // write to HyperRam
      udma_hyper_setup();
      udma_hyper_dwrite(read_size, rdDone+ram_start_addr, (unsigned int) flashBuffer, 0, 0);
      udma_hyper_dread(read_size,  rdDone+ram_start_addr, (unsigned int) data_check, 0, 0);
      udma_hyper_wait(0);
      for(int j=0; j < read_size; j++){
         if(*((char *)data_check+j) != *((char *)(flashBuffer+j)))
              printf("%d th data chunk %d th wrong %x vs %x \n", i, j+rdDone,*((char *) flashBuffer+j), *((char *) data_check+j));
         }
         
      rdDone += read_size / sizeof(char);
    }
    flash_start_addr += flash_weight_size[i];
    ram_start_addr   += flash_weight_size[i];
    rdDone = 0;
  }

  for(int i=0; i<11; i++){
     if(i==0) L3_weights_size[0]=0;
     for(int j=0; j<i; j++){
        L3_weights_sa[i] += flash_weight_size[j];
     }
  }
    activations_input = ram_start_addr;
    rt_free(RT_ALLOC_PERIPH, flashBuffer, flashBuffSize);
    rt_free(RT_ALLOC_PERIPH, data_check , flashBuffSize);


  return 1;
}

// on cluster
void cluster_main(void *arg) {
  int *real_arg = (int *) arg;
  network_run(
    (unsigned int) real_arg[0]
    //(unsigned int) real_arg[1]
    );
}

void pulp_parallel(void *arg)
{
  rt_team_fork(NUM_CORES, (void *)cluster_main, arg);
}

void network_run_FabricController()
{
  //int arg[2];
  int arg[2];
  arg[0] = (unsigned int) L3_weights_size;
  //arg[1] = (unsigned int) hyperram;

  //PMU_set_voltage(1000, 0);
 /* rt_time_wait_us(10000);
  rt_freq_set(RT_FREQ_DOMAIN_FC, 100000000);
  rt_time_wait_us(10000);
  rt_freq_set(RT_FREQ_DOMAIN_CL, 100000000);
  rt_time_wait_us(10000);*/
  rt_cluster_mount(1, 0, 0, NULL);
  rt_cluster_call(NULL, 0, pulp_parallel, arg, NULL,1024+1024, 1024+1024, rt_nb_pe(), NULL);
  rt_cluster_mount(0, 0, 0, NULL);
}


  int memId;
  char* L2_output;
  char* L2_input;
  char* L2_weights_1;
  char* L2_weights_2;
  char* L2_buffer_allocation;
  int L2_buffer_allocation_end;

   char *l1_buffer;

void network_run( 
  unsigned int L3_weights_size
  //unsigned int hyperram
  )
{   

  if (rt_core_id()==0)
  {
    L2_buffer_allocation = rt_alloc(RT_ALLOC_L2_CL_DATA, 400000);
    L2_buffer_allocation_end = L2_buffer_allocation + 400000;
    l1_buffer = rt_alloc(RT_ALLOC_CL_DATA,44000 );
#ifdef VERBOSE
    printf("L2 Buffer alloc initial\t@ 0x%08x:\t%s\n", (unsigned int)L2_buffer_allocation, L2_buffer_allocation?"Ok":"Failed");
#endif
    udma_hyper_setup();
    printf("L3_weights_sa:");
    for(int i=0; i<10; i++){
    printf("%d ", L3_weights_sa[i]);
    }
    printf("%d ", L3_weights_sa[10]);
    printf("\n");
  }


    uint16_t out_mult = 0;
    uint16_t out_shift = 0;
    uint16_t inmul1 = 0;
    uint16_t inmul2 = 0;
    int branch_active = 0;
    int counter = 0;
    int valid = 0;
    uint8_t * bypass_activations = 0;
    int bypass_dimension = 0;
    int d_buffering_weights_t = 0;
    int error_presence = 0;
    int d_buffering_weights_e = 0;
    int d_buffering_inputs = 0;
    int d_buffering_outputs = 0;
    int begin_end_n = 1;
#ifdef VERBOSE
  int check;
#endif
    char* exec_weights,*transfer_weights;


  transfer_weights = d_buffering_weights_t ? L2_weights_2 : L2_weights_1;
  exec_weights = d_buffering_weights_e ? L2_weights_2 : L2_weights_1;

  if(rt_core_id()==0)
  {
        dory_L2_alloc(&L2_buffer_allocation,
            &L2_buffer_allocation_end,
            &L2_input,
            49152,
            begin_end_n // begin is 1, end is 0
            );
    //rt_hyperram_cluster_read_mine(hyperram, L2_input, activations_input, 49152, &wait_L3_w);
    //rt_hyperram_cluster_wait(&wait_L3_w);
    udma_hyper_dread(49152, (unsigned int) L3_weights_sa[10], (unsigned int) L2_input ,128,0);
    udma_hyper_wait(0);
    printf("input data is transferred from Hyperram \n");

    dory_L2_alloc(&L2_buffer_allocation,
            &L2_buffer_allocation_end,
            &L2_weights_1,
            1056,
            begin_end_n // begin is 1, end is 0
            );
    begin_end_n = !begin_end_n;
    transfer_weights = L2_weights_1;
    exec_weights = L2_weights_1;  

    //rt_hyperram_cluster_read_mine(hyperram, transfer_weights, L3_weights_internal, 1056, &wait_L3_w);
    //rt_hyperram_cluster_wait(&wait_L3_w);
    udma_hyper_dread(1056, (unsigned int) L3_weights_sa[0], transfer_weights ,128, 0);
    udma_hyper_wait(0);
    printf("weight0 data is transferred from Hyperram \n");
    /* for all layers in a list, instantiate the layer.
    Instantiate the L3 copies again in a double-buffering fashion*/
    /* Instantiate the L3 first memory passage */
    dory_L2_alloc(&L2_buffer_allocation,
            &L2_buffer_allocation_end,
            &L2_output,
            131072,
            begin_end_n // begin is 1, end is 0
            );
    d_buffering_weights_t = !d_buffering_weights_t;
    if(L2_output == NULL) return -1;
    dory_L2_alloc(&L2_buffer_allocation,
            &L2_buffer_allocation_end,
            &L2_weights_2,
            1536- 1056,
            begin_end_n // begin is 1, end is 0
            );
    transfer_weights = d_buffering_weights_t ? L2_weights_2 : L2_weights_1;
    begin_end_n = !begin_end_n;
  }

    rt_perf_t perf2;
    rt_perf_init(&perf2);                       
    rt_perf_conf(&perf2, (1<<RT_PERF_CYCLES));          
    rt_perf_reset(&perf2);                      
    rt_perf_stop(&perf2);                       
    rt_perf_start(&perf2); 
  if(rt_core_id()==0)
  {
    
    //rt_hyperram_cluster_read_mine(hyperram, transfer_weights, L3_weights_internal + 1056, 480, &wait_L3_w);
    udma_hyper_dread(480, (unsigned int) L3_weights_sa[1], transfer_weights ,128,0);
    printf("weight1 data is transferred from Hyperram \n");
  }

#ifdef VERBOSE
  if(rt_core_id()==0)
  {
    check = 117576;
    check_layer_weight(exec_weights, check, 1056) ;
    check = 4227015;
    check_layer(L2_input, check, 49152);
  // printf("L2 input %d, L2 output %d, weights %d\n", L2_input, L2_output, exec_weights);
  // printf("L1 buffer %d\n", l1_buffer);
  }
#endif  
  out_mult = 31.0;
  out_shift = 19.0;
  rt_team_barrier();
  layerConvBNRelu0(
      L2_input,
      L2_output,
      exec_weights,
      l1_buffer,
  out_mult,
  out_shift
      );
  rt_team_barrier();


 if(rt_core_id()==0)
 	L2_output[3*32*64+6*32+28] +=1;

  if(rt_core_id()==0)
  {
#ifdef VERBOSE
    printf("Layer %d ended: \n", 0);  
    check = 2593838;
    check_layer(L2_output, check, 131072) ;
#endif 
  }
  if(rt_core_id()==0)
  {
  if (branch_active == 1)
    counter++;
    dory_L2_free(&L2_buffer_allocation,
      &L2_buffer_allocation_end,
      1056,
      begin_end_n // begin is 1, end is 0
      );
    //begin_end_n != begin_end_n;
    //rt_hyperram_cluster_wait(&wait_L3_w); 
    udma_hyper_wait(0);

    d_buffering_weights_e = !d_buffering_weights_e;
    exec_weights = d_buffering_weights_e ? L2_weights_2 : L2_weights_1;
    dory_L2_free(&L2_buffer_allocation,
      &L2_buffer_allocation_end,
      49152,
      begin_end_n // begin is 1, end is 0
      );
  if (valid == 0 && counter%2==1)
  {
    dory_L2_free(&L2_buffer_allocation,
      &L2_buffer_allocation_end,
      bypass_dimension,
      begin_end_n // begin is 1, end is 0
      );
  counter = 0;
  branch_active = 0;
  }
    L2_input = L2_output;
    dory_L2_alloc(&L2_buffer_allocation,
            &L2_buffer_allocation_end,
            &L2_output,
            131072,
            begin_end_n // begin is 1, end is 0
            );

    if (d_buffering_weights_e==1)
    {
      dory_L2_alloc(&L2_buffer_allocation,
              &L2_buffer_allocation_end,
              &L2_weights_1,
              2432,
              begin_end_n // begin is 1, end is 0
              );
    }
    else
    {
      dory_L2_alloc(&L2_buffer_allocation,
              &L2_buffer_allocation_end,
              &L2_weights_2,
              2432,
              begin_end_n // begin is 1, end is 0
              );
    }  
    d_buffering_weights_t = !d_buffering_weights_t;
    transfer_weights = d_buffering_weights_t ? L2_weights_2 : L2_weights_1;
    begin_end_n = !begin_end_n;
        //switching output and input.
  }
  if(rt_core_id()==0)
  {
    //rt_hyperram_cluster_read_mine(hyperram, transfer_weights, L3_weights_internal + 1536, 2432, &wait_L3_w);
    udma_hyper_dread(2432, (unsigned int) L3_weights_sa[2], transfer_weights ,128, 0);
    printf("weight2 data is transferred from Hyperram \n");
  }

#ifdef VERBOSE
  if(rt_core_id()==0)
  {
    check = 49561;
    check_layer_weight(exec_weights, check, 480) ;
    check = 2593838;
    check_layer(L2_input, check, 131072);
  // printf("L2 input %d, L2 output %d, weights %d\n", L2_input, L2_output, exec_weights);
  // printf("L1 buffer %d\n", l1_buffer);
  }
#endif  
  out_mult = 23.0;
  out_shift = 18.0;
  rt_team_barrier();
  layerConvDWBNRelu1(
      L2_input,
      L2_output,
      exec_weights,
      l1_buffer,
  out_mult,
  out_shift
      );
  rt_team_barrier();



  if(rt_core_id()==0)
  {
#ifdef VERBOSE
    printf("Layer %d ended: \n", 1);  
    check = 4653815;
    check_layer(L2_output, check, 131072) ;
#endif 
  }
  if(rt_core_id()==0)
  {
  if (branch_active == 1)
    counter++;
    dory_L2_free(&L2_buffer_allocation,
      &L2_buffer_allocation_end,
              480,
      begin_end_n // begin is 1, end is 0
      );
    //begin_end_n != begin_end_n;
    //rt_hyperram_cluster_wait(&wait_L3_w);
    udma_hyper_wait(0);

    d_buffering_weights_e = !d_buffering_weights_e;
    exec_weights = d_buffering_weights_e ? L2_weights_2 : L2_weights_1;
    dory_L2_free(&L2_buffer_allocation,
      &L2_buffer_allocation_end,
      131072,
      begin_end_n // begin is 1, end is 0
      );
  if (valid == 0 && counter%2==1)
  {
    dory_L2_free(&L2_buffer_allocation,
      &L2_buffer_allocation_end,
      bypass_dimension,
      begin_end_n // begin is 1, end is 0
      );
  counter = 0;
  branch_active = 0;
  }
    L2_input = L2_output;
    dory_L2_alloc(&L2_buffer_allocation,
            &L2_buffer_allocation_end,
            &L2_output,
            262144,
            begin_end_n // begin is 1, end is 0
            );

    if (d_buffering_weights_e==1)
    {
      dory_L2_alloc(&L2_buffer_allocation,
              &L2_buffer_allocation_end,
              &L2_weights_1,
              960,
              begin_end_n // begin is 1, end is 0
              );
    }
    else
    {
      dory_L2_alloc(&L2_buffer_allocation,
              &L2_buffer_allocation_end,
              &L2_weights_2,
              960,
              begin_end_n // begin is 1, end is 0
              );
    }  
    d_buffering_weights_t = !d_buffering_weights_t;
    transfer_weights = d_buffering_weights_t ? L2_weights_2 : L2_weights_1;
    begin_end_n = !begin_end_n;
        //switching output and input.
  }
  if(rt_core_id()==0)
  {
    //rt_hyperram_cluster_read_mine(hyperram, transfer_weights, L3_weights_internal + 3968, 960, &wait_L3_w);
    udma_hyper_dread(960, (unsigned int) L3_weights_sa[3], transfer_weights ,128, 0);
    printf("weight3 data is transferred from Hyperram \n");
    //udma_hyper_wait();

  }

#ifdef VERBOSE
  if(rt_core_id()==0)
  {
    check = 292789;
    check_layer_weight(exec_weights, check, 2432) ;
    check = 4653815;
    check_layer(L2_input, check, 131072);
  // printf("L2 input %d, L2 output %d, weights %d\n", L2_input, L2_output, exec_weights);
  // printf("L1 buffer %d\n", l1_buffer);
  }
#endif  

  out_mult = 23.0;
  out_shift = 21.0;
  rt_team_barrier();
  layerConvBNRelu2(
      L2_input,
      L2_output,
      exec_weights,
      l1_buffer,
  out_mult,
  out_shift
      );
  rt_team_barrier();

  if(rt_core_id()==0)
  {
#ifdef VERBOSE
    printf("Layer %d ended: \n", 2);  
    check = 3633007;
    check_layer(L2_output, check, 262144) ;
#endif 
  }
  if(rt_core_id()==0)
  {
  if (branch_active == 1)
    counter++;
    dory_L2_free(&L2_buffer_allocation,
      &L2_buffer_allocation_end,
              2432,
      begin_end_n // begin is 1, end is 0
      );
    //begin_end_n != begin_end_n;
    //rt_hyperram_cluster_wait(&wait_L3_w); 
    udma_hyper_wait(0);
    

    d_buffering_weights_e = !d_buffering_weights_e;
    exec_weights = d_buffering_weights_e ? L2_weights_2 : L2_weights_1;
    dory_L2_free(&L2_buffer_allocation,
      &L2_buffer_allocation_end,
      131072,
      begin_end_n // begin is 1, end is 0
      );
  if (valid == 0 && counter%2==1)
  {
    dory_L2_free(&L2_buffer_allocation,
      &L2_buffer_allocation_end,
      bypass_dimension,
      begin_end_n // begin is 1, end is 0
      );
  counter = 0;
  branch_active = 0;
  }
    L2_input = L2_output;
    dory_L2_alloc(&L2_buffer_allocation,
            &L2_buffer_allocation_end,
            &L2_output,
            65536,
            begin_end_n // begin is 1, end is 0
            );

    if (d_buffering_weights_e==1)
    {
      dory_L2_alloc(&L2_buffer_allocation,
              &L2_buffer_allocation_end,
              &L2_weights_1,
              8960,
              begin_end_n // begin is 1, end is 0
              );
    }
    else
    {
      dory_L2_alloc(&L2_buffer_allocation,
              &L2_buffer_allocation_end,
              &L2_weights_2,
              8960,
              begin_end_n // begin is 1, end is 0
              );
    }  
    d_buffering_weights_t = !d_buffering_weights_t;
    transfer_weights = d_buffering_weights_t ? L2_weights_2 : L2_weights_1;
    begin_end_n = !begin_end_n;
        //switching output and input.
  }
  if(rt_core_id()==0)
  {
    //rt_hyperram_cluster_read_mine(hyperram, transfer_weights, L3_weights_internal + 4928, 8960, &wait_L3_w);
    udma_hyper_dread(8960, (unsigned int) L3_weights_sa[4], transfer_weights ,128, 0);
    printf("weight4 data is transferred from Hyperram \n");
  }

#ifdef VERBOSE
  if(rt_core_id()==0)
  {
    check = 108125;
    check_layer_weight(exec_weights, check, 960) ;
    check = 3633007;
    check_layer(L2_input, check, 262144);
  // printf("L2 input %d, L2 output %d, weights %d\n", L2_input, L2_output, exec_weights);
  // printf("L1 buffer %d\n", l1_buffer);
  }
#endif  

// commented to test
  out_mult = 18.0;
  out_shift = 19.0;
  rt_team_barrier();
  layerConvDWBNRelu3(
      L2_input,
      L2_output,
      exec_weights,
      l1_buffer,
  out_mult,
  out_shift
      );
  rt_team_barrier();


  if(rt_core_id()==0)
  {
#ifdef VERBOSE
    //printf("Layer %d ended: \n", 3);  
    check = 1319132;
    check_layer(L2_output, check, 65536) ;
#endif 
  }
  if(rt_core_id()==0)
  {
  if (branch_active == 1)
    counter++;
    dory_L2_free(&L2_buffer_allocation,
      &L2_buffer_allocation_end,
              960,
      begin_end_n // begin is 1, end is 0
      );
    //begin_end_n != begin_end_n;
    //rt_hyperram_cluster_wait(&wait_L3_w); 
    udma_hyper_wait(0);

    d_buffering_weights_e = !d_buffering_weights_e;
    exec_weights = d_buffering_weights_e ? L2_weights_2 : L2_weights_1;
    dory_L2_free(&L2_buffer_allocation,
      &L2_buffer_allocation_end,
      262144,
      begin_end_n // begin is 1, end is 0
      );
  if (valid == 0 && counter%2==1)
  {
    dory_L2_free(&L2_buffer_allocation,
      &L2_buffer_allocation_end,
      bypass_dimension,
      begin_end_n // begin is 1, end is 0
      );
  counter = 0;
  branch_active = 0;
  }
    L2_input = L2_output;
    dory_L2_alloc(&L2_buffer_allocation,
            &L2_buffer_allocation_end,
            &L2_output,
            131072,
            begin_end_n // begin is 1, end is 0
            );

    if (d_buffering_weights_e==1)
    {
      dory_L2_alloc(&L2_buffer_allocation,
              &L2_buffer_allocation_end,
              &L2_weights_1,
              1920,
              begin_end_n // begin is 1, end is 0
              );
    }
    else
    {
      dory_L2_alloc(&L2_buffer_allocation,
              &L2_buffer_allocation_end,
              &L2_weights_2,
              1920,
              begin_end_n // begin is 1, end is 0
              );
    }  
    d_buffering_weights_t = !d_buffering_weights_t;
    transfer_weights = d_buffering_weights_t ? L2_weights_2 : L2_weights_1;
    begin_end_n = !begin_end_n;
        //switching output and input.
  }
  if(rt_core_id()==0)
  {
    //rt_hyperram_cluster_read_mine(hyperram, transfer_weights, L3_weights_internal + 13888, 1920, &wait_L3_w);
    //udma_hyper_dread(1920, transfer_weights, L3_weights_sa[5], 128);
    udma_hyper_dread(1920, (unsigned int) L3_weights_sa[5], transfer_weights ,128 ,0);
    printf("weight5 data is transferred from Hyperram \n");

  }

#ifdef VERBOSE
  if(rt_core_id()==0)
  {
    check = 1084080;
    check_layer_weight(exec_weights, check, 8960) ;
    check = 1319132;
    check_layer(L2_input, check, 65536);
  // printf("L2 input %d, L2 output %d, weights %d\n", L2_input, L2_output, exec_weights);
  // printf("L1 buffer %d\n", l1_buffer);
  }
#endif  
  out_mult = 21.0;
  out_shift = 21.0;
  rt_team_barrier();

  layerConvBNRelu4(
      L2_input,
      L2_output,
      exec_weights,
      l1_buffer,
  out_mult,
  out_shift
      );
  rt_team_barrier();


  if(rt_core_id()==0)
  {
#ifdef VERBOSE
    //printf("Layer %d ended: \n", 4);  
    check = 1663744;
    check_layer(L2_output, check, 131072) ;
#endif 
  }
  if(rt_core_id()==0)
  {
  if (branch_active == 1)
    counter++;
    dory_L2_free(&L2_buffer_allocation,
      &L2_buffer_allocation_end,
              8960,
      begin_end_n // begin is 1, end is 0
      );
    //begin_end_n != begin_end_n;
    //rt_hyperram_cluster_wait(&wait_L3_w); 
    udma_hyper_wait(0);

    d_buffering_weights_e = !d_buffering_weights_e;
    exec_weights = d_buffering_weights_e ? L2_weights_2 : L2_weights_1;
    dory_L2_free(&L2_buffer_allocation,
      &L2_buffer_allocation_end,
      65536,
      begin_end_n // begin is 1, end is 0
      );
  if (valid == 0 && counter%2==1)
  {
    dory_L2_free(&L2_buffer_allocation,
      &L2_buffer_allocation_end,
      bypass_dimension,
      begin_end_n // begin is 1, end is 0
      );
  counter = 0;
  branch_active = 0;
  }
    L2_input = L2_output;
    dory_L2_alloc(&L2_buffer_allocation,
            &L2_buffer_allocation_end,
            &L2_output,
            131072,
            begin_end_n // begin is 1, end is 0
            );

    if (d_buffering_weights_e==1)
    {
      dory_L2_alloc(&L2_buffer_allocation,
              &L2_buffer_allocation_end,
              &L2_weights_1,
              17152,
              begin_end_n // begin is 1, end is 0
              );
    }
    else
    {
      dory_L2_alloc(&L2_buffer_allocation,
              &L2_buffer_allocation_end,
              &L2_weights_2,
              17152,
              begin_end_n // begin is 1, end is 0
              );
    }  
    d_buffering_weights_t = !d_buffering_weights_t;
    transfer_weights = d_buffering_weights_t ? L2_weights_2 : L2_weights_1;
    begin_end_n = !begin_end_n;
        //switching output and input.
  }
  if(rt_core_id()==0)
  {
    //rt_hyperram_cluster_read_mine(hyperram, transfer_weights, L3_weights_internal + 15808, 17152, &wait_L3_w);
    //udma_hyper_dread(17152, transfer_weights, L3_weights_sa[6], 128);
    udma_hyper_dread(17152, (unsigned int) L3_weights_sa[6], transfer_weights ,128, 0);
    printf("weight6 data is transferred from Hyperram \n");

  }

#ifdef VERBOSE
  if(rt_core_id()==0)
  {
    check = 220694;
    check_layer_weight(exec_weights, check, 1920) ;
    check = 1663744;
    //check_layer(L2_input, check, 131072);
  // printf("L2 input %d, L2 output %d, weights %d\n", L2_input, L2_output, exec_weights);
  // printf("L1 buffer %d\n", l1_buffer);
  }
#endif  

  out_mult = 21.0;
  out_shift = 18.0;
  rt_team_barrier();
  layerConvDWBNRelu5(
      L2_input,
      L2_output,
      exec_weights,
      l1_buffer,
  out_mult,
  out_shift
      );
  rt_team_barrier();


  if(rt_core_id()==0)
  {
#ifdef VERBOSE
    printf("Layer %d ended: \n", 5);  
    check = 2150927;
    check_layer(L2_output, check, 131072) ;
#endif 
  }
  if(rt_core_id()==0)
  {
  if (branch_active == 1)
    counter++;
    dory_L2_free(&L2_buffer_allocation,
      &L2_buffer_allocation_end,
              1920,
      begin_end_n // begin is 1, end is 0
      );
    //begin_end_n != begin_end_n;
    //rt_hyperram_cluster_wait(&wait_L3_w); 
    udma_hyper_wait(0);

    d_buffering_weights_e = !d_buffering_weights_e;
    exec_weights = d_buffering_weights_e ? L2_weights_2 : L2_weights_1;
    dory_L2_free(&L2_buffer_allocation,
      &L2_buffer_allocation_end,
      131072,
      begin_end_n // begin is 1, end is 0
      );
  if (valid == 0 && counter%2==1)
  {
    dory_L2_free(&L2_buffer_allocation,
      &L2_buffer_allocation_end,
      bypass_dimension,
      begin_end_n // begin is 1, end is 0
      );
  counter = 0;
  branch_active = 0;
  }
    L2_input = L2_output;
    dory_L2_alloc(&L2_buffer_allocation,
            &L2_buffer_allocation_end,
            &L2_output,
            131072,
            begin_end_n // begin is 1, end is 0
            );

    if (d_buffering_weights_e==1)
    {
      dory_L2_alloc(&L2_buffer_allocation,
              &L2_buffer_allocation_end,
              &L2_weights_1,
              1920,
              begin_end_n // begin is 1, end is 0
              );
    }
    else
    {
      dory_L2_alloc(&L2_buffer_allocation,
              &L2_buffer_allocation_end,
              &L2_weights_2,
              1920,
              begin_end_n // begin is 1, end is 0
              );
    }  
    d_buffering_weights_t = !d_buffering_weights_t;
    transfer_weights = d_buffering_weights_t ? L2_weights_2 : L2_weights_1;
    begin_end_n = !begin_end_n;
        //switching output and input.
  }
  if(rt_core_id()==0)
  {
    //rt_hyperram_cluster_read_mine(hyperram, transfer_weights, L3_weights_internal + 32960, 1920, &wait_L3_w);
    udma_hyper_dread(1920, (unsigned int) L3_weights_sa[7], transfer_weights ,128, 0);
    printf("weight7 data is transferred from Hyperram \n");

  }

#ifdef VERBOSE
  if(rt_core_id()==0)
  {
    check = 2214519;
    check_layer_weight(exec_weights, check, 17152) ;
    check = 2150927;
    check_layer(L2_input, check, 131072);
  // printf("L2 input %d, L2 output %d, weights %d\n", L2_input, L2_output, exec_weights);
  // printf("L1 buffer %d\n", l1_buffer);
  }
#endif  

  out_mult = 22.0;
  out_shift = 21.0;
  rt_team_barrier();
  layerConvBNRelu6(
      L2_input,
      L2_output,
      exec_weights,
      l1_buffer,
  out_mult,
  out_shift
      );
  rt_team_barrier(); 



  if(rt_core_id()==0)
  {
#ifdef VERBOSE
    //printf("Layer %d ended: \n", 6);  
    check = 1086247;
    check_layer(L2_output, check, 131072) ;
#endif 
  }
  if(rt_core_id()==0)
  {
  if (branch_active == 1)
    counter++;
    dory_L2_free(&L2_buffer_allocation,
      &L2_buffer_allocation_end,
              17152,
      begin_end_n // begin is 1, end is 0
      );
    //begin_end_n != begin_end_n;
    //rt_hyperram_cluster_wait(&wait_L3_w); 
    udma_hyper_wait(0);

    d_buffering_weights_e = !d_buffering_weights_e;
    exec_weights = d_buffering_weights_e ? L2_weights_2 : L2_weights_1;
    dory_L2_free(&L2_buffer_allocation,
      &L2_buffer_allocation_end,
      131072,
      begin_end_n // begin is 1, end is 0
      );
  if (valid == 0 && counter%2==1)
  {
    dory_L2_free(&L2_buffer_allocation,
      &L2_buffer_allocation_end,
      bypass_dimension,
      begin_end_n // begin is 1, end is 0
      );
  counter = 0;
  branch_active = 0;
  }
    L2_input = L2_output;
    dory_L2_alloc(&L2_buffer_allocation,
            &L2_buffer_allocation_end,
            &L2_output,
            32768,
            begin_end_n // begin is 1, end is 0
            );

    if (d_buffering_weights_e==1)
    {
      dory_L2_alloc(&L2_buffer_allocation,
              &L2_buffer_allocation_end,
              &L2_weights_1,
              34304,
              begin_end_n // begin is 1, end is 0
              );
    }
    else
    {
      dory_L2_alloc(&L2_buffer_allocation,
              &L2_buffer_allocation_end,
              &L2_weights_2,
              34304,
              begin_end_n // begin is 1, end is 0
              );
    }  
    d_buffering_weights_t = !d_buffering_weights_t;
    transfer_weights = d_buffering_weights_t ? L2_weights_2 : L2_weights_1;
    begin_end_n = !begin_end_n;
        //switching output and input.
  }
  if(rt_core_id()==0)
  {
    //rt_hyperram_cluster_read_mine(hyperram, transfer_weights, L3_weights_internal + 34880, 34304, &wait_L3_w);
    udma_hyper_dread(34304, (unsigned int) L3_weights_sa[8], transfer_weights ,128,0);
    printf("weight8 data is transferred from Hyperram \n");

  }

#ifdef VERBOSE
  if(rt_core_id()==0)
  {
    check = 191350;
    check_layer_weight(exec_weights, check, 1920) ;
    check = 1086247;
    check_layer(L2_input, check, 131072);
  // printf("L2 input %d, L2 output %d, weights %d\n", L2_input, L2_output, exec_weights);
  // printf("L1 buffer %d\n", l1_buffer);
  }
#endif  

  out_mult = 17.0;
  out_shift = 19.0;
  rt_team_barrier();
  layerConvDWBNRelu7(
      L2_input,
      L2_output,
      exec_weights,
      l1_buffer,
  out_mult,
  out_shift
      );
  rt_team_barrier();


  if(rt_core_id()==0)
  {
#ifdef VERBOSE
    //printf("Layer %d ended: \n", 7);  
    check = 593980;
    check_layer(L2_output, check, 32768) ;
#endif 
  }
  if(rt_core_id()==0)
  {
  if (branch_active == 1)
    counter++;
    dory_L2_free(&L2_buffer_allocation,
      &L2_buffer_allocation_end,
              1920,
      begin_end_n // begin is 1, end is 0
      );
    //begin_end_n != begin_end_n;
    //rt_hyperram_cluster_wait(&wait_L3_w); 
    udma_hyper_wait(0);

    d_buffering_weights_e = !d_buffering_weights_e;
    exec_weights = d_buffering_weights_e ? L2_weights_2 : L2_weights_1;
    dory_L2_free(&L2_buffer_allocation,
      &L2_buffer_allocation_end,
      131072,
      begin_end_n // begin is 1, end is 0
      );
  if (valid == 0 && counter%2==1)
  {
    dory_L2_free(&L2_buffer_allocation,
      &L2_buffer_allocation_end,
      bypass_dimension,
      begin_end_n // begin is 1, end is 0
      );
  counter = 0;
  branch_active = 0;
  }
    L2_input = L2_output;
    dory_L2_alloc(&L2_buffer_allocation,
            &L2_buffer_allocation_end,
            &L2_output,
            65536,
            begin_end_n // begin is 1, end is 0
            );

    if (d_buffering_weights_e==1)
    {
      dory_L2_alloc(&L2_buffer_allocation,
              &L2_buffer_allocation_end,
              &L2_weights_1,
              3840,
              begin_end_n // begin is 1, end is 0
              );
    }
    else
    {
      dory_L2_alloc(&L2_buffer_allocation,
              &L2_buffer_allocation_end,
              &L2_weights_2,
              3840,
              begin_end_n // begin is 1, end is 0
              );
    }  
    d_buffering_weights_t = !d_buffering_weights_t;
    transfer_weights = d_buffering_weights_t ? L2_weights_2 : L2_weights_1;
    begin_end_n = !begin_end_n;
        //switching output and input.
  }
  if(rt_core_id()==0)
  {
    //rt_hyperram_cluster_read_mine(hyperram, transfer_weights, L3_weights_internal + 69184, 3840, &wait_L3_w);
    udma_hyper_dread(3840, (unsigned int) L3_weights_sa[9], transfer_weights ,128,0);
    printf("weight9 data is transferred from Hyperram \n");

  }

#ifdef VERBOSE
  if(rt_core_id()==0)
  {
    check = 4223678;
    check_layer_weight(exec_weights, check, 34304) ;
    check = 593980;
    check_layer(L2_input, check, 32768);
  // printf("L2 input %d, L2 output %d, weights %d\n", L2_input, L2_output, exec_weights);
  // printf("L1 buffer %d\n", l1_buffer);
  }
#endif  

  out_mult = 19.0;
  out_shift = 21.0;
  rt_team_barrier();
  layerConvBNRelu8(
      L2_input,
      L2_output,
      exec_weights,
      l1_buffer,
  out_mult,
  out_shift
      );
  rt_team_barrier();


  if(rt_core_id()==0)
  {
#ifdef VERBOSE
    printf("Layer %d ended: \n", 8);  
    check = 658371;
    check_layer(L2_output, check, 65536) ;
#endif 
  }
  if(rt_core_id()==0)
  {
  if (branch_active == 1)
    counter++;
    dory_L2_free(&L2_buffer_allocation,
      &L2_buffer_allocation_end,
              34304,
      begin_end_n // begin is 1, end is 0
      );
    //begin_end_n != begin_end_n;
    //rt_hyperram_cluster_wait(&wait_L3_w); 
    udma_hyper_wait(0);

    d_buffering_weights_e = !d_buffering_weights_e;
    exec_weights = d_buffering_weights_e ? L2_weights_2 : L2_weights_1;
    dory_L2_free(&L2_buffer_allocation,
      &L2_buffer_allocation_end,
      32768,
      begin_end_n // begin is 1, end is 0
      );
  if (valid == 0 && counter%2==1)
  {
    dory_L2_free(&L2_buffer_allocation,
      &L2_buffer_allocation_end,
      bypass_dimension,
      begin_end_n // begin is 1, end is 0
      );
  counter = 0;
  branch_active = 0;
  }
    L2_input = L2_output;
    dory_L2_alloc(&L2_buffer_allocation,
            &L2_buffer_allocation_end,
            &L2_output,
            65536,
            begin_end_n // begin is 1, end is 0
            );

    begin_end_n = !begin_end_n;
        //switching output and input.
  }
  if(rt_core_id()==0)
  {
  }

#ifdef VERBOSE
  if(rt_core_id()==0)
  {
    check = 500855;
    check_layer_weight(exec_weights, check, 3840) ;
    check = 658371;
    check_layer(L2_input, check, 65536);
  // printf("L2 input %d, L2 output %d, weights %d\n", L2_input, L2_output, exec_weights);
  // printf("L1 buffer %d\n", l1_buffer);
  }
#endif  
  out_mult = 18.0;
  out_shift = 18.0;
  rt_team_barrier();
  layerConvDWBNRelu9(
      L2_input,
      L2_output,
      exec_weights,
      l1_buffer,
  out_mult,
  out_shift
      );
  rt_team_barrier();



  if(rt_core_id()==0)
  {
#ifdef VERBOSE
    //printf("Layer %d ended: \n", 9);  
    check = 660670;
    int max = -100000;
    int index_max = -1;
    for(int class = 0; class < 65536; class ++)
    {
      if (*(L2_output + class) > max) 
      {
        index_max = class;
        max = *(L2_output + class);
      }
    }
    printf("Output class is: %d, Correct Class is %d\n", index_max, 1417);
#endif 
  }
/*rt_perf_stop(&perf2);          
rt_perf_save(&perf2);          
int cid = rt_core_id();   
int perf_cyc =  rt_perf_get(&perf2, RT_PERF_CYCLES) ; 
int MACs = 186400768;
float perf_MAC =  (float)MACs/perf_cyc;
if (cid == 0){
printf("[%d] : num_cycles: %d\n",cid,perf_cyc); 
printf("[%d] : MACs: %d\n",cid,MACs ); 
printf("[%d] : MAC/cycle: %f\n",cid,perf_MAC ); 
printf("[%d] : n. of Cores: %d\n",cid,NUM_CORES); 
}*/
}


