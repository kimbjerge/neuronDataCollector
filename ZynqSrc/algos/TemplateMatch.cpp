/*
 * TemplateMatch.cpp
 *
 *  Created on: 25. dec. 2018
 *      Author: Kim Bjerge
 */

#include <math.h>
#include "TemplateMatch.h"
#include "xparameters.h"
#include "FIRFilter_coeffs.h"

#define DEBUG_FILES // When disabled doesn't save time

TemplateMatch::~TemplateMatch()
{
	if (mNumSamples > 0) {
		for (int i = 0; i < FIR_NUM; i++) {
			delete(pFirFilter[i]);
		}
		for (int i = 0; i < TEMP_NUM; i++) {
			delete(pNXCOR[i]);
			delete(pTemplate[i]);
		}
#ifdef DEBUG_FILES
		for (int i = 0; i < TEMP_NUM; i++)
			delete(pResultNXCOR[i]);
		delete(pResultFIR);
#endif
	}
}

int TemplateMatch::Init(string *pTempNames[TEMP_NUM], int numSamples, IRQ* pIrq)
{

	for (int i = 0; i < FIR_NUM; i++) {
		pFirFilter[i] = new FirFilter(XPAR_FIRFILTER_0_DEVICE_ID+i, FIR_SIZE, FIR_TAPS);
		pFirFilter[i]->Init(pIrq, XPAR_FABRIC_FIRFILTER_0_INTERRUPT_INTR+i);
	}
	for (int i = 0; i < TEMP_NUM; i++) {
		pNXCOR[i] = new NXCOR(XPAR_NXCOR_0_DEVICE_ID+i, TEMP_LENGTH, TEMP_WIDTH);
		pNXCOR[i]->Init(pIrq, XPAR_FABRIC_NXCOR_0_INTERRUPT_INTR+i);
		pTemplate[i] = new Template();
		// Load template from file defined by pTempNames
		pTemplate[i]->loadTemplate(*pTempNames[i]);
	}

#ifdef DEBUG_FILES
	for (int i = 0; i < TEMP_NUM; i++) {
		pResultNXCOR[i] = new ResultFile<float>();
		pResultNXCOR[i]->allocateContent(numSamples);
	}
	pResultFIR = new ResultFile<int>();
	pResultFIR->allocateContent(numSamples*NUM_CHANNELS);
#endif

	mNumSamples = numSamples;

	return 0;
}

int TemplateMatch::updateCoefficients()
{
	// Updating coefficients based on table with double numbers
	for (int i = 0; i < FIR_TAPS; i++) {
		mCoeff[i] = (int)round(FIR_coeffs[i]*pow(2,FIR_FORMAT)); // Convert to format 1.FIR_FORMAT
	}
	for (int i = 0; i < FIR_NUM; i++) {
		pFirFilter[i]->updateCoefficients(mCoeff);
	}
	for (int i = 0; i < NUM_CHANNELS; i++)
		mFiltered[i] = 0;
	return 0;
}

int TemplateMatch::updateTemplates()
{
	// Update template in HLS IP core
	for (int i = 0; i < TEMP_NUM; i++) {
		pNXCOR[i]->updateTemplate(pTemplate[i]->getTemplate(), round(pTemplate[i]->getMean()));
	}
	return 0;
}

void TemplateMatch::processResults(void) {
	// TODO implement threshold selection
	//printf("NXCT1 %f\r\n", mNXCORRes[0]);
	//printf("NXCT2 %f\r\n", mNXCORRes[1]);
}

void TemplateMatch::run()
{
	int count = mNumSamples;
    bool firstTime = true;
    int *pSampleData = (int *)lxRecord.board[0].data;
    int start_tick, end_tick;

	printf("Updating FIR coefficients and template 1+2\r\n");
    updateCoefficients();
    updateTemplates();
	printf("Neuron template matching running\r\n");
	start_tick = xTaskGetTickCount();

	while (count > 0) {

		// Get next sample from data generator
		pNeuronData->GenerateSampleRecord(&lxRecord);

		if (!firstTime) {
			// After fist iteration read filtered samples
			for (int i = 0; i < FIR_NUM; i++) {
				pFirFilter[i]->readFiltered(&mFiltered[i*FIR_SIZE]);
			}
			// Read result of normalized cross core correlation
			for (int i = 0; i < TEMP_NUM; i++) {
				mNXCORRes[i] = pNXCOR[i]->readResultNXCOR(pTemplate[i]->getVariance());
			}
		}

		// Start filtering next NUM_CHANNELS of samples
		for (int i = 0; i < FIR_NUM; i++) {
			pFirFilter[i]->startFilter((int *)&pSampleData[i*FIR_SIZE]);
		}
		// Start normalized cross core correlation of filtered samples
		for (int i = 0; i < TEMP_NUM; i++) {
			pNXCOR[i]->startNXCOR((int *)&mFiltered[i+pTemplate[i]->getChOffset()]);
		}

		processResults();

		// Append test result to memory
#ifdef DEBUG_FILES
		pResultFIR->appendData(mFiltered, NUM_CHANNELS);
		for (int i = 0; i < TEMP_NUM; i++) {
			pResultNXCOR[i]->appendData(&mNXCORRes[i], 1);
		}
#endif
		// Wait for one sample delay
		//printf(".");
		//vTaskDelay( pdMS_TO_TICKS( 1 ) );
		//vTaskDelay( pdMS_TO_TICKS( 0.0333333 ) );
		firstTime = false;
		count--;
	}
	end_tick = xTaskGetTickCount();

	printf("Tick start %d and tick end %d, duration = %d ms\r\n", start_tick, end_tick, (1000*(end_tick-start_tick))/configTICK_RATE_HZ);
	printf("Neuron template matching completed on %d samples\r\n", mNumSamples);

#ifdef DEBUG_FILES
	// Save test result from memory to files
	pResultFIR->saveContent("FIRFilt.bin");
	pResultNXCOR[0]->saveContent("NXCORT1.bin");
	pResultNXCOR[1]->saveContent("NXCORT2.bin");
	printf("Saved result to files FIRFilt.bin, NXCORT1.bin and NXCORT2.bin\r\n");
#endif

}

