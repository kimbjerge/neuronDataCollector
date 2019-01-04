///////////////////////////////////////////////////////////
//  NXCOR.h
//  Header:          Normalized Cross Correlation function
//  Created on:      20-12-2018
//  Original author: Kim Bjerge
///////////////////////////////////////////////////////////
#ifndef NXCOR_H
#define NXCOR_H

#include "stdint.h"

//#define SC_INCLUDE_FX
//#define SC_FX_EXCLUDE_OTHER
//#include <systemc.h>
//#define 				T									sc_fixed<64,62,SC_RND_ZERO,SC_SAT>
#define 				T									int64_t
//#define 				T									float

//#define 				sigType 							int32_t
#define 				sigType 							int16_t

#define 				DATA_CHANNELS 						32
#define					TEMPLATE_CROPPED_LENGTH				17 // 16/17 best thesis solution 1
#define					TEMPLATE_CROPPED_WIDTH				9  // 8/9 best thesis solution 1
//#define                 INIT_VALUE                          0x00007fff
#define                 INIT_VALUE                          0x00000000
#define					TEMPLATE_SIZE 						(TEMPLATE_CROPPED_LENGTH*TEMPLATE_CROPPED_WIDTH)

void initNXCOR(void);

/* NXCOR function call */
void NXCOR(T *result, T *varSig, sigType inputSignal[TEMPLATE_CROPPED_WIDTH],
			sigType templateData[TEMPLATE_CROPPED_WIDTH*TEMPLATE_CROPPED_LENGTH],
			sigType avgTemp, short width, short length); //, int avgMultiplier); // Actual width and length

#endif
