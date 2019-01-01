///////////////////////////////////////////////////////////
//  NXCOR.h
//  Header:          Normalized Cross Correlation functions.
//  Created on:      20-12-2018
//  Original author: Kim Bjerge
///////////////////////////////////////////////////////////

//#include <systemc.h>
#include "NXCOR.h"


/*----------------------------------------------------------------------------*/
/**
* @brief Calculates the mean for a given window
*
* @param int32_t* inputPatch:    Pointer to the supplied data
* @param uint32_t r:       Start index column to start from
* @param uint32_t c:       Start index row to start from
* @param uint32_t stride : The size of the row (channel width)
*
* @retval T : The mean value
*
T mean(int32_t* inputPatch, uint16_t r, uint16_t c, uint16_t stride)
{
	T average = 0;
	mean_label0:for (uint16_t i = 0; i < r; i++)
		for (uint16_t j = c; j < TEMPLATE_CROPPED_WIDTH + c; j++) {
			average += inputPatch[(i * stride) + j]; // Computes average
		}
	average = average / (TEMPLATE_CROPPED_WIDTH*TEMPLATE_CROPPED_LENGTH);
	return average;
}
*/
static sigType signalBuffer[TEMPLATE_CROPPED_WIDTH*TEMPLATE_CROPPED_LENGTH];

void initNXCOR(void)
{
	uint16_t i, j;
	// Shift delay line and compute average of signal
	for (i = 0; i < TEMPLATE_CROPPED_LENGTH; i++)
		for (j = 0; j < TEMPLATE_CROPPED_WIDTH; j++)
		{
			signalBuffer[j+(i*TEMPLATE_CROPPED_WIDTH)] = INIT_VALUE;
		}

}
/*----------------------------------------------------------------------------*/
/**
* @brief Performs NXCOR computation between the signal and a template
*   @note This Implementation do not calculate the final normalization
*         relative to variance of template and signal
*
* @param T* result :                 	Pointer to the NXCOR output is stored
* @param uint32_t *varSignal : 			Pointer to calculated variance of signal
* @param T* signalData :                Pointer to the input signal array - 1D
* @param T* templateData :              Pointer to the input template array - 1D
* @param uint32_t avgTemp :             Mean value of template
* @param uint32_t signalLowerIndex : 	Indicates the start channel of where the template should be applied from
*
* @retval void : none
*/
void NXCOR(T *result, T *varSig,
		   sigType signalData[TEMPLATE_CROPPED_WIDTH],
		   sigType templateData[TEMPLATE_CROPPED_WIDTH*TEMPLATE_CROPPED_LENGTH],
		   sigType avgTemp)
{
	int64_t xcorr;     // Cross correlation between template and pixel area
	int64_t varSignal; // Variance signal buffer
	//sc_int<48> xcorr;     // Cross correlation between template and pixel area
	//sc_int<48> varSignal; // Variance signal buffer
	int32_t avgSignal; // Average signal buffer
	uint16_t i, j;

	// Clear average calculation
	avgSignal = 0;
	// Shift delay line and compute average of signal
	NXCOR_label0: for (i = TEMPLATE_CROPPED_LENGTH-1; i > 0; i--)
	{
		NXCOR_label1: for (j = 0; j < TEMPLATE_CROPPED_WIDTH; j++)
		{
			int32_t signal = signalBuffer[j + ((i-1)*TEMPLATE_CROPPED_WIDTH)];
			signalBuffer[j + (i*TEMPLATE_CROPPED_WIDTH)] = signal;
		    avgSignal += signal;
		}
	}

	// Insert newest samples
	NXCOR_label2: for (j = 0; j < TEMPLATE_CROPPED_WIDTH; j++)
	{
		int32_t signal = signalData[j];
		signalBuffer[j] = signal;
	    avgSignal += signal;
	}

	// Compute average of signal - TEMPLATE_CROPPED_WIDTH*TEMPLATE_CROPPED_LENGTH best to be a power of 2
	avgSignal = avgSignal / (TEMPLATE_CROPPED_WIDTH*TEMPLATE_CROPPED_LENGTH);

	// Computes mean of image area
	//avgSignal = mean(signalBuffer, TEMPLATE_CROPPED_LENGTH, signalLowerIndex, TEMPLATE_CROPPED_WIDTH);

	// Clear variance and cross correlation
	xcorr = 0.0;
	varSignal = 0.0;

	// Computes cross correlation and variance
	NXCOR_label3: for (i = 0; i < TEMPLATE_CROPPED_LENGTH; i++) // Cross correlation with template
		NXCOR_label4: for (j = 0; j < TEMPLATE_CROPPED_WIDTH; j++)
		{
			uint16_t idx = j + (i*TEMPLATE_CROPPED_WIDTH);
			int32_t temp = templateData[idx];
			int32_t signalValue = signalBuffer[idx];

			int32_t pr = signalValue - avgSignal;
			int32_t tr = temp - avgTemp;
			xcorr += (pr * tr);
			varSignal += (pr * pr);
		}

	// Computes normalized cross correlation - should be done in software
	//*result = (T)(xcorr / sqrt(varSignal * varTemp));
	*result = (T)xcorr;
	*varSig = (T)varSignal;
}

