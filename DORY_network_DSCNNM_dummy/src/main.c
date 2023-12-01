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
#include "mem.h"
#include "network.h"

#include "pmsis.h"

#define VERBOSE 1



int main () {
  //PMU_set_voltage(1000, 0);
  pi_time_wait_us(10000);
  pi_freq_set(PI_FREQ_DOMAIN_FC, 100000000);
  pi_time_wait_us(10000);
  pi_freq_set(PI_FREQ_DOMAIN_CL, 100000000);
  pi_time_wait_us(10000);

/*
    Opening of Filesystem and Ram
*/
  mem_init();
  network_initialize();
  /*
    Allocating space for input
  */
  void *l2_buffer = pi_l2_malloc(380000);
#ifdef VERBOSE
  printf("\nL2 Buffer alloc initial\t@ 0x%08x:\t%s\n", (unsigned int)l2_buffer, l2_buffer?"Ok":"Failed");
#endif
  size_t l2_input_size = 490;
  size_t input_size = 1000000;

  void *ram_input = ram_malloc(input_size);
      load_file_to_ram(ram_input, "inputs.hex");
      ram_read(l2_buffer, ram_input, l2_input_size);
      network_run(l2_buffer, 380000, l2_buffer, 0);

  ram_free(ram_input, input_size);
  network_terminate();
  pi_l2_free(l2_buffer, 380000);
}
