///////////////////////////////////////////////////////////
//  FIRFilter.h
//  Header:          FIR Filter
//  Created on:      22-12-2018
//  Original author: Kim Bjerge
///////////////////////////////////////////////////////////
#ifndef FIRFiler_H
#define FIRFiler_H

#include "stdint.h"

#define 				DATA_CHANNELS 						8
#define					NUM_TAPS							60
#define                 INIT_VALUE                          0x00000000
#define 				ALGO_BITS							23  // 15 or 23 bits resolution of coefficients

void initFIR(void);

// Processing number of DATA_CHANNELS in parallel NUM_TAPS FIR filters
void FIRFilter (int32_t results[DATA_CHANNELS],
				int32_t samples[DATA_CHANNELS],
				int32_t coeff[NUM_TAPS],
				int32_t operation);
#endif
