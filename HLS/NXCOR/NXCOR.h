///////////////////////////////////////////////////////////
//  NXCOR.h
//  Header:          Normalized Cross Correlation functions.
//  Created on:      23-10-2017
//  Original author: MB
///////////////////////////////////////////////////////////
//#ifndef NXCOR_H
//#define NXCOR_H
#define SC_INCLUDE_FX
//#define SC_FX_EXCLUDE_OTHER
#include <systemc.h>

#define 				T									float
//#define 				T									sc_fixed<32,8,SC_RND_ZERO,SC_SAT>
#define 				DATA_CHANNELS 						32
#define					TEMPLATE_CROPPED_LENGTH				17
#define					TEMPLATE_CROPPED_WIDTH				9
#define                 SIGNAL_LENGTH						20

/* NXCOR functions calls*/
void runNXCOR(T result[SIGNAL_LENGTH], T input[SIGNAL_LENGTH*DATA_CHANNELS], T template_[TEMPLATE_CROPPED_LENGTH*TEMPLATE_CROPPED_WIDTH], uint32_t templateLength, uint32_t templateChannels, uint32_t signalLength,  uint32_t signalLowerIndex);

//void runNXCOR_STD(T* result, T* signal, T* template_, uint32_t templateLength, uint32_t templateChannels, uint32_t signalLength, uint32_t signalLowerIndex, uint32_t numberOfChannelDrift);
/* Helper Functions */
//float performXTestReturnExecutionTime(T* result, T* signal, T* template_, uint32_t templateLength, uint32_t templateChannels, uint32_t signalLength, bool useOpenCV, uint32_t numberOfTest);

//#endif
