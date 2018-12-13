///////////////////////////////////////////////////////////
//  NXCORController.h
//  Header:			 The class controlling the multiple instances of NXCOR.
//  Created on:      30-10-2017
//  Original author: MB
///////////////////////////////////////////////////////////
#ifndef NXCOR_CONTROLLER_H
#define NXCOR_CONTROLLER_H

//#include "stdint.h"
//#include "math.h"
#include "NXCOR.h"

int main() {
	T result[SIGNAL_LENGTH];
	T signal[SIGNAL_LENGTH*DATA_CHANNELS];
	T tempA[TEMPLATE_CROPPED_LENGTH*TEMPLATE_CROPPED_WIDTH];
	runNXCOR(result, signal, tempA, TEMPLATE_CROPPED_LENGTH, TEMPLATE_CROPPED_WIDTH, SIGNAL_LENGTH, 1);
}


#endif
