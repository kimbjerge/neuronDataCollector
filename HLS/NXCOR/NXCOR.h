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
#define 				DATA_CHANNELS 						32
#define					TEMPLATE_CROPPED_LENGTH				16
#define					TEMPLATE_CROPPED_WIDTH				8
#define                 INIT_VALUE                          0x00007fff

void initNXCOR(void);

/* NXCOR function call */
void NXCOR(T *result, T *varSig, int32_t inputSignal[TEMPLATE_CROPPED_WIDTH],
		   int32_t templateData[TEMPLATE_CROPPED_WIDTH*TEMPLATE_CROPPED_LENGTH],
		   int32_t avgTemp);

#endif
