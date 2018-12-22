///////////////////////////////////////////////////////////
//  NXCORController.h
//  Header:			 The class controlling the multiple instances of NXCOR.
//  Created on:      30-10-2017
//  Original author: MB
///////////////////////////////////////////////////////////
#ifndef NXCOR_CONTROLLER_H
#define NXCOR_CONTROLLER_H

#include "stdio.h"
#include "NXCOR.h"
#include "math.h"

T meanTemp(int32_t* inputPatch, uint32_t r, uint32_t c, uint32_t stride) //, uint32_t wt, uint32_t ht)
{
	T average = 0;
	mean_label0:for (uint32_t i = r; i<TEMPLATE_CROPPED_LENGTH + r; i++)
		for (uint32_t j = c; j<TEMPLATE_CROPPED_WIDTH + c; j++) {
			average += inputPatch[(i * stride) + j]; // Computes average
		}
	average = average / (TEMPLATE_CROPPED_WIDTH*TEMPLATE_CROPPED_LENGTH);
	return average;
}

int main() {
	T result;
	int32_t signalTest[DATA_CHANNELS];
	int32_t tempA[TEMPLATE_CROPPED_LENGTH*TEMPLATE_CROPPED_WIDTH];
	T avgTemp = 0;   // Average template
	T varTemp = 0;   // Variance template
	T varSignal = 0; // Variance signal

	//initNXCOR();

	// Set test values in signal
	for (uint16_t j = 0; j < DATA_CHANNELS; j++) {
		signalTest[j] = INIT_VALUE;
	}
	// Set test values in template
	for (uint16_t i = 0; i < TEMPLATE_CROPPED_LENGTH; i++) // Cross correlation with template
		for (uint16_t j = 0; j < TEMPLATE_CROPPED_WIDTH; j++) {
			tempA[i * TEMPLATE_CROPPED_WIDTH + j] = INIT_VALUE;
		}

	// Computes average of template for red and blue color
	avgTemp = meanTemp(tempA, 0, 0, TEMPLATE_CROPPED_WIDTH);

	// Compute variance of template
	for (uint16_t i = 0; i < TEMPLATE_CROPPED_LENGTH; i++) // Cross correlation with template
		for (uint16_t j = 0; j < TEMPLATE_CROPPED_WIDTH; j++) {
			T temp = tempA[i * TEMPLATE_CROPPED_WIDTH + j];

			// Red color, cross correlation, variance pixel, variance template
			T tr = temp - avgTemp;
			varTemp = varTemp + (tr*tr);
		}

	for (uint16_t i = 0; i < TEMPLATE_CROPPED_LENGTH*2; i++)
		NXCOR(&result, &varSignal, &signalTest[4], tempA, avgTemp);

	float xcorr = result;
	float varS = varSignal;
	float varT = varTemp;
	if (varS > 0 && varT > 0)
		printf("Result NXCOR: %f\r\n", xcorr / sqrt(varS * varT));
	else {
		printf("Result XCOR: %f\r\n", xcorr);
		printf("Result VarT: %f\r\n", varT);
		printf("Result VarS: %f\r\n", varS);
	}

	return 0;
}


#endif
