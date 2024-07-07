/*
 * Copyright (C) 2022 GreenWaves Technologies
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 *
 */


#pragma once
#include <stdint.h>

/*
 * \brief set up the AK4332 (PDM DAC)
 *
 * \return 0 if successful, an error code otherwise
 */
int setup_dac(uint8_t id);
int fxl6408_setup(void);