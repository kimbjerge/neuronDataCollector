///////////////////////////////////////////////////////////
//  IIRFilter.h
//  Header:          IIR Filter
//  Created on:      02-05-2019
//  Original author: Kim Bjerge
///////////////////////////////////////////////////////////
#ifndef IIRFiler_H
#define IIRFiler_H

#include "stdint.h"

#define 				DATA_CHANNELS 						8
#define					NUM_TAPS							3 // Number of b or a coefficients
#define                 NUM_SOS								6 // Number of second order sections (IIR)
#define                 INIT_VALUE                          0x00000000
#define 				ALGO_BITS							17  // 23 bits resolution of coefficients
#define                 sigType                             int32_t
//#define                 sigType                             int16_t

void initIIR(void);

// Processing number of DATA_CHANNELS in parallel NUM_TAPS IIR filters
void IIRFilter (sigType results[DATA_CHANNELS],
				sigType samples[DATA_CHANNELS],
				int32_t coeff[NUM_TAPS*2*NUM_SOS], // First b coefficients then a coefficients
				int32_t operation);
#endif
