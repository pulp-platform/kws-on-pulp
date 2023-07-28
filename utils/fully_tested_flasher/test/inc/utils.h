/*
 * utils.h
 * Angelo Garofalo <angelo.garofalo@unibo.it>
 * Nazareno Bruschi <nazareno.bruschi@studio.unibo.it>
 * Francesco Conti <f.conti@unibo.it>
 * 
 * Copyright (C) 2019 ETH Zurich, University of Bologna.
 * All rights reserved.
 */

uint8_t __attribute__((always_inline)) pulp_nn_add_quant_u8 (
  uint8_t pix1,            
  uint8_t pix2,
  int16_t m1,
  int16_t m2,
  int8_t  d
); 

void pulp_nn_compare_and_replace_if_larger_int8(
	uint8_t * base,
	uint8_t * target,
	uint16_t  length
);

void pulp_zero_mem(
	uint8_t * pBuffer,
	int       size
);

void pulp_nn_im2col_int8(
	uint8_t * pInput,
	uint8_t * pOutput,
	unsigned int blockSize
);

void pulp_nn_im2col_int8_dmafree(
	uint8_t * pInput, 
	uint8_t * pOutput, 
	unsigned int blockSize
);
void pulp_nn_avg_and_replace_int8(
  int8_t * base,           // baseline for comparison
  int8_t * target,         // compare target
  uint16_t length          // data size
);
