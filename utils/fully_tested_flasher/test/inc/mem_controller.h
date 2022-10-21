#include "rt/rt_api.h"

void dory_L2_alloc(unsigned int * L2_pointer_input_begin,
              unsigned int * L2_pointer_input_end,
              unsigned int * L2_pointer_output,
              int memory_to_allocate,
              int begin_end_n // begin is 1, end is 0
              );
void dory_L2_free(unsigned int * L2_pointer_input_begin,
            unsigned int * L2_pointer_input_end,
            int memory_to_free,
            int begin_end_n // begin is 1, end is 0
            );

void dory_L1_alloc(unsigned int * L2_pointer_input_begin,
              unsigned int * L2_pointer_output,
              int memory_to_allocate
              );


void dory_L1_free(unsigned int * L2_pointer_input_begin,
            int memory_to_free
            );




