// Test utterances
#include "tinytest.h"

#include "testing.h"

// L2
#include "input.h"

// Peripherals
#include "Gap.h"
#include "bsp/ram.h"
#include <bsp/fs/hostfs.h>
#include "gaplib/wavIO.h" 
#include "Graph_L2_Descr.h" // pdm_in_test
#include "localutil.h"

enum source {
  ONLINE,
  OFLINE
}; 

void evaluate_validation(int was_trained, int mfcc_src);
void evaluate_tinytest(int was_trained, int utter_eval_src, int noise_eval_src, int mfcc_src);
