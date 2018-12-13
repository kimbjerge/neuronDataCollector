///////////////////////////////////////////////////////////
//  NXCOR.h
//  Header:          Normalized Cross Correlation functions.
//  Created on:      23-10-2017
//  Original author: MB
///////////////////////////////////////////////////////////
#ifndef NXCOR_H
#define NXCOR_H

#include "stdint.h"
#include "math.h"

#include "NXCOR.h"

/*----------------------------------------------------------------------------*/
/**
* @brief Calculates the mean for a given window
*
* @param T* inputPatch:    Pointer to the supplied data
* @param uint32_t r:       Start index couloum to start from
* @param uint32_t c:       Start index row to start from
* @param uint32_t stride : The size of the row (channel width)
* @param uint32_t wt :     Indicates the width of the template
* @param uint32_t ht :     Indicates the length or height of the template
*
* @retval T : The mean value
*/
T mean(T* inputPatch, uint32_t r, uint32_t c, uint32_t stride, uint32_t wt, uint32_t ht)
{
	T average = 0;
	mean_label0:for (uint32_t i = r; i<ht + r; i++)
		for (uint32_t j = c; j<wt + c; j++) {
			average += inputPatch[(i * stride) + j]; // Computes average
		}
	average = average / (wt*ht);
	return average;
}

/*----------------------------------------------------------------------------*/
/**
* @brief Performs NXCOR computation between the signal and a template
*   @note This Implementation assumes that the signal width and
*         the template width is the same
*
* @param T* result :                 Pointer to the array in which the NXCOR output is stored
*				                     - The resulting array is (signalLength-templateLength + 1) long.
* @param T* signal :                 Pointer to the input signal array - 1D
* @param T* template_ :              Pointer to the input template array - 1D
* @param uint32_t templateLength :   Indicates of the template length
* @param uint32_t templateChannels : Indicates the number og channels in the template and the signal
* @param uint32_t signalLength :     Indicates the length of the signal
* @param uint32_t signalLowerIndex : Indicates the start channel of where the template should be applied from
*
* @retval void : none
*/
void runNXCOR(T result[SIGNAL_LENGTH], T input[SIGNAL_LENGTH*DATA_CHANNELS], T template_[TEMPLATE_CROPPED_LENGTH*TEMPLATE_CROPPED_WIDTH],
		      uint32_t templateLength, uint32_t templateChannels, uint32_t signalLength, uint32_t signalLowerIndex)
{
	//const uint32_t wt = templateChannels;
	//const uint32_t ht = templateLength;
	//const uint32_t w = 32;
	//const uint32_t h = signalLength;
#define wt TEMPLATE_CROPPED_WIDTH
#define ht TEMPLATE_CROPPED_LENGTH
#define w  32
#define h  SIGNAL_LENGTH

	T xcorr = 0;     // Cross correlation between template and pixel area
	T varSignal = 0; // Variance Signal area
	T varTemp = 0;   // Variance template
	T avgSignal = 0; // Average Signal area
	T avgTemp = 0;   // Average template

	// Clear result
	//for (int n = 0; n < (h- ht); n++) result[n] = 0;

	// Computes average of template for red and blue color
	avgTemp = mean(template_, 0, 0, wt, wt, ht);

	// Compute variance of template
	runNXCOR_label1:for (uint32_t k = 0; k < ht; k++) // Cross correlation with template
		for (uint32_t l = 0; l < wt; l++) {
			T temp = template_[k * wt + l];

			// Red color, cross correlation, variance pixel, variance template
			T tr = temp - avgTemp;
			varTemp = varTemp + (tr*tr);
		}

	runNXCOR_label3:for (uint32_t j = 0; j <= (h - ht); j++) { // For all coloums - assuming all rows every time

		// Computes mean of image area
		avgSignal = mean(input, j, signalLowerIndex, 32, wt, ht);

		// Clear variance and cross correlation
		xcorr = 0;
		varSignal = 0;

		// Computes cross correlation and variance
		runNXCOR_label2:for (uint32_t x = 0; x < ht; x++) // Cross correlation with template
			for (uint32_t y = 0; y < wt; y++) {
				T signalValue = input[(((x+j)*w) + y + signalLowerIndex)];
				T temp = template_[(x*wt) + y];

				T pr = signalValue - avgSignal;
				T tr = temp - avgTemp;
				xcorr += (pr * tr);
				varSignal = varSignal + (pr * pr);
			}

		// Computes normalized cross correlation
		result[j] = (T)(xcorr / sqrt(varSignal * varTemp));
	}
	
}


/*----------------------------------------------------------------------------*/
/**
* @brief Performs NXCOR computation between the signal and a template with drift handling.
*   @note This Implementation assumes that the signal width and
*         the template width is the same. STD stands for Single Threaded Drift Handling		  
*
* @param T* result :                     Pointer to the array in which the NXCOR output is stored
*				                         - The resulting array is (signalLength-templateLength + 1) long.
* @param T* signal :                     Pointer to the input signal array - 1D
* @param T* template_ :                  Pointer to the input template array - 1D
* @param uint32_t templateLength :       Indicates of the template length
* @param uint32_t templateChannels :     Indicates the number og channels in the template and the signal
* @param uint32_t signalLength :         Indicates the length of the signal
* @param uint32_t signalLowerIndex :     Indicates the start channel of where the template should be applied from
* @param uint32_t numberOfChannelDrift : Indicates the amount of channels drift can be handled for.
*
* @retval void : none
*
void NXCOR::runNXCOR_STD(T* result, T* signal, T* template_, uint32_t templateLength, uint32_t templateChannels, uint32_t signalLength, uint32_t signalLowerIndex, uint32_t numberOfChannelDrift)
{
	const uint32_t wt = templateChannels;
	const uint32_t ht = templateLength;
	const uint32_t w = 32;
	const uint32_t h = signalLength;
	uint32_t driftIterations = (numberOfChannelDrift * 2) + 1;

	T xcorr = 0; // Cross correlation between template and pixel area
	T varSignal = 0; // Variance Signal area
	T varTemp = 0; // Variance template
	T avgSignal = 0; // Average Signal area
	T avgTemp = 0; // Average template
	T bestXCorr = 0;
	int32_t signalLowerIndexOld = signalLowerIndex;

	for (uint32_t d = 0; d < driftIterations; d++)
	{
		int32_t dataOffset = d - numberOfChannelDrift;
		int32_t templateStartChannel = 0;
		int32_t templateEndChannel = templateChannels;
		int32_t dataEndChannel = templateChannels;

		if ( (signalLowerIndexOld + templateChannels + dataOffset) <= DATA_CHANNELS && // the data and template must be cropped !
			 (int32_t(signalLowerIndexOld) + dataOffset) >= 0 )
		{
			signalLowerIndex = signalLowerIndexOld + dataOffset;
		}
		else
		{
			if ((int32_t(signalLowerIndexOld) + dataOffset) < 0)
			{
				templateStartChannel -= dataOffset; // Increment
				dataEndChannel -= 1;
				signalLowerIndex = 0;
				//templateEndChannel += dataOffset; // This will decrement!!
			}
			else if ((int32_t(signalLowerIndexOld) + templateChannels + dataOffset) > DATA_CHANNELS)
			{
				//templateStartChannel -= dataOffset; // this will increment, as d will always be negative here!!
				signalLowerIndex = signalLowerIndexOld + dataOffset;
				dataEndChannel -= 1;
				templateEndChannel -= dataOffset; // This will decrement!!
			}
		}

		// Computes average of template for red and blue color
		avgTemp = mean(template_, 0, templateStartChannel, wt, (templateEndChannel- templateStartChannel), ht);

		// Compute variance of template
		varTemp = 0; // Variance template
		for (uint32_t k = 0; k < ht; k++) // Cross correlation with template
			for (uint32_t l = templateStartChannel; l < templateEndChannel; l++) {
				T temp = template_[k * wt + l];

				// Red color, cross correlation, variance pixel, variance template
				T tr = temp - avgTemp;
				varTemp = varTemp + (tr*tr);
			}

		for (uint32_t j = 0; j <= (h - ht); j++) { // For all coloums - assuming all rows every time

			// Computes mean of image area
			avgSignal = mean(signal, j, signalLowerIndex, DATA_CHANNELS, (templateEndChannel - templateStartChannel), ht);

			// Clear variance and cross correlation
			xcorr = 0;
			varSignal = 0;

			// Computes cross correlation and variance
			for (uint32_t x = 0; x < ht; x++) // Cross correlation with template
				for (uint32_t y = 0; y < dataEndChannel; y++) {
					T signalValue = signal[(((x + j)*w) + y + signalLowerIndex)];
					T temp = template_[(x*wt) + y + templateStartChannel];

					T pr = signalValue - avgSignal;
					T tr = temp - avgTemp;
					xcorr += (pr * tr);
					varSignal = varSignal + (pr * pr);
				}

			// Computes normalized cross correlation
			//T normxcorr = xcorr / sqrt(varSignal * varTemp);
			if (d > 0)
			{
				T currentData = (T)(xcorr / sqrt(varSignal * varTemp));
				if (currentData > result[j])
				{
					result[j] = currentData;
				}
			}
			else
			{
				result[j] = (T)(xcorr / sqrt(varSignal * varTemp));
			}
		}
	}
}
*/


/*----------------------------------------------------------------------------*/
/**
* @brief Returns the average execution time, when performing multiple tests.
*
* @retval float : The execution time in microseconds (us)
*
float NXCOR::performXTestReturnExecutionTime(T* result, T* signal, T* template_, uint32_t templateLength, uint32_t templateChannels, uint32_t signalLength, bool useOpenCV, uint32_t numberOfTest)
{
	float returnValue = 0;
	float* timeArray = new float[numberOfTest];

	for (uint32_t i = 0; i < numberOfTest; i++)
	{
		runNXCOR(result, signal, template_, templateLength, templateChannels, signalLength);
		timeArray[i] = getLatestExecutionTime();
	}

	float sum = 0;
	for (uint32_t j = 0; j < numberOfTest; j++)
	{
		sum += timeArray[j];
	}

	returnValue = sum / numberOfTest;

	delete timeArray;
	return returnValue;
}
*/

#endif
