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

#define DATA_FORMAT 15    // Data format of template
#define NUM_SAMPLES 1000 // Filter test on real neuron signals

float m_data[NUM_SAMPLES][DATA_CHANNELS];
int m_idx;
float m_template[TEMPLATE_CROPPED_LENGTH][TEMPLATE_CROPPED_WIDTH];

void readDataSamples(void)
{
	FILE *fp;
	fp = fopen("DATA.bin", "r+b");
	size_t sz = fread(m_data, sizeof(float), NUM_SAMPLES*DATA_CHANNELS, fp);
	if (sz != NUM_SAMPLES*DATA_CHANNELS) {
		printf("Error reading DATA.bin file\r\n");
	}
	fclose(fp);
	m_idx = 0;
}

void getNextSample(int32_t *signal)
{
	int ch;
	for (ch = 0; ch < DATA_CHANNELS; ch++) {
		signal[ch] = m_data[m_idx][ch];
	}
	m_idx++;
}

void readTemplate(int32_t *temp)
{
	FILE *fp;
	fp = fopen("T11_01.bin", "r+b");
	size_t sz = fread(m_template, sizeof(float), TEMPLATE_SIZE, fp);
	if (sz != TEMPLATE_SIZE) {
		printf("Error reading T11_01.bin file\r\n");
	}
	fclose(fp);

	for (int i = 0; i < TEMPLATE_SIZE; i++) {
		int n = i/TEMPLATE_CROPPED_WIDTH;
		int c = i%TEMPLATE_CROPPED_WIDTH;
		temp[i] = m_template[n][c]*pow(2, DATA_FORMAT);
	}

}

// Compute mean value of template
float meanTemp(int32_t* temp)
{
	float average = 0;
	mean_label0:for (uint32_t i = 0; i<TEMPLATE_CROPPED_LENGTH; i++)
		for (uint32_t j = 0; j<TEMPLATE_CROPPED_WIDTH; j++) {
			average += temp[i * TEMPLATE_CROPPED_WIDTH + j]; // Computes average
		}
	average = average / TEMPLATE_SIZE;
	return average;
}

// Compute variance of template
float varianceTemp(int32_t *temp, T avgTemp)
{
	float varTemp = 0;
	for (uint16_t i = 0; i < TEMPLATE_CROPPED_LENGTH; i++) // Cross correlation with template
		for (uint16_t j = 0; j < TEMPLATE_CROPPED_WIDTH; j++) {
			float tmp = temp[i * TEMPLATE_CROPPED_WIDTH + j];

			// Red color, cross correlation, variance pixel, variance template
			float tr = tmp - avgTemp;
			varTemp = varTemp + (tr*tr);
		}
	return varTemp;
 }

int main()
{
	FILE   *fp;
	int32_t signalTest[DATA_CHANNELS];
	int32_t tempA[TEMPLATE_CROPPED_LENGTH*TEMPLATE_CROPPED_WIDTH];
	float avgTemp;   // Average template
	float varTemp;   // Variance template
	T result; // Result from NXCOR
	T varSignal; // Variance signal

    initNXCOR();
#if 0
	// Set test values in signal
	for (uint16_t j = 0; j < DATA_CHANNELS; j++) {
		signalTest[j] = INIT_VALUE;
	}
	// Set test values in template
	for (uint16_t i = 0; i < TEMPLATE_CROPPED_LENGTH; i++) // Cross correlation with template
		for (uint16_t j = 0; j < TEMPLATE_CROPPED_WIDTH; j++) {
			tempA[i * TEMPLATE_CROPPED_WIDTH + j] = INIT_VALUE;
		}
#else
	readDataSamples();
	readTemplate(tempA);
#endif

	// Computes average of template for red and blue color
	avgTemp = meanTemp(tempA); // Tested OK
    varTemp = varianceTemp(tempA, avgTemp); // Tested OK

	fp=fopen("nxcorr.dat","w");

	for (uint16_t i = 0; i < NUM_SAMPLES; i++) {
		getNextSample(signalTest);
		NXCOR(&result, &varSignal, &signalTest[1], tempA, round(avgTemp));
		float xcorr = result;
		float varS = varSignal;
		if (varS > 0)
			xcorr = xcorr / sqrt(varS * varTemp);
		else
			xcorr = 0;
		printf("%04i %lld %lld %f\n", i, result, varSignal, xcorr);
		fprintf(fp,"%04i %lld %lld %f\r\n", i, result, varSignal, xcorr);
	}
	fclose(fp);

	return 0;
}


#endif
