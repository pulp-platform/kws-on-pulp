/*
 * Copyright (C) 2022 GreenWaves Technologies
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 *
 */

/* 
    include files
*/

// #include "mram.h"
// #define pi_default_flash_conf pi_mram_conf

#include "application.h"

int application(){
    return 0;
}

int main()
{
    PRINTF("\n\n\t *** Application ***\n\n");

    #define __XSTR(__s) __STR(__s)
    #define __STR(__s) #__s
    WavName = __XSTR(WAV_FILE); 
    mfcc = __XSTR(MFCC);
    noise_eval_input = __XSTR(NOISE_EVAL);
    uttr_eval_input = __XSTR(UTTR_EVAL);
    appl_input = __XSTR(APPL);

    return application();
}