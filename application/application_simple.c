/*
 * test_template.c
 * Alessio Burrello <alessio.burrello@unibo.it>
 *
 * Copyright (C) 2019-2020 University of Bologna
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License. 
 */

#include "application.h"


// Peripherals
#include "Gap.h"
#include "bsp/ram.h"
#include <bsp/fs/hostfs.h>
#include "gaplib/wavIO.h" 
#include "Graph_L2_Descr.h" // pdm_in_test
#include "localutil.h"

// PMSIS SFU
#include "sfu_pmsis_runtime.h"
#define NB_BUF_IN_RING 2
#define FREQ_PDM_BIT (3072000)
#define FREQ_PCM (48000)
#define SAI_RX (1)
#define SAI_TX (0)


// MFCC
#include "MFCC_params.h"
#include "MfccKernels.h"
#include "DCTTwiddles.def"
#include "MelFBSparsity.def"
#include "WindowLUT.def"
#include "FFTTwiddles.def"
#include "RFFTTwiddles.def"
#include "MelFBCoeff.def"
#include "SwapTable.def"

// DORY
#include "mem.h"
#include "network.h"


#define VERBOSE 1



void application(void * arg) {
/*
    Opening of Filesystem and Ram
*/
  mem_init();
  network_initialize();
  /*
    Allocating space for input
  */
  void *l2_buffer = pi_l2_malloc(1000000);
  if (NULL == l2_buffer) {
#ifdef VERBOSE
    printf("ERROR: L2 buffer allocation failed.");
#endif
    pmsis_exit(-1);
  }
#ifdef VERBOSE
  printf("\nL2 Buffer alloc initial\t@ 0x%08x:\tOk\n", (unsigned int)l2_buffer);
#endif
  size_t l2_input_size = 490;
  size_t input_size = 1000000;
  int initial_dir = 1;

  void *ram_input = ram_malloc(input_size);
  load_file_to_ram(ram_input, "inputs.hex");
  ram_read(l2_buffer, ram_input, l2_input_size);

  printf ("Starting network inference!\n");
  void * dump;
  network_run(l2_buffer, 1000000, l2_buffer, &dump, 0, initial_dir);

  ram_free(ram_input, input_size);
  network_terminate();
  pi_l2_free(l2_buffer, 1000000);
}

int main () {
#ifndef TARGET_CHIP_FAMILY_GAP9
  PMU_set_voltage(1000, 0);
#else
  // pi_pmu_voltage_set(PI_PMU_VOLTAGE_DOMAIN_CHIP, PI_PMU_VOLT_800); // GAP8
  pi_pmu_voltage_set(PI_PMU_VOLTAGE_DOMAIN_CHIP, PI_VOLT_800); // GAP9
#endif
  pi_time_wait_us(10000);
  pi_freq_set(PI_FREQ_DOMAIN_FC, 370000000);
  pi_time_wait_us(10000);
  pi_freq_set(PI_FREQ_DOMAIN_CL, 370000000);
  pi_time_wait_us(10000);
  pi_freq_set(PI_FREQ_DOMAIN_PERIPH, 370000000);
  pi_time_wait_us(10000);


  pmsis_kickoff((void*)application);
  return 0;
}
