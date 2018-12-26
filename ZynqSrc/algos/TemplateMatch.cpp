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

TemplateMatch::~TemplateMatch()
{
	if (mNumSamples > 0) {
		for (int i = 0; i < FIR_NUM; i++) {
			delete(pFirFilter[i]);
		}
		for (int i = 0; i < TEMP_NUM; i++) {
			delete(pNXCOR[i]);
			delete(pTemplate[i]);
			delete(pResultNXCOR[i]);
		}
		delete(pResultFIR);
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
		pResultNXCOR[i] = new ResultFile<float>();
		pResultNXCOR[i]->allocateContent(numSamples);
	}
	pResultFIR = new ResultFile<int>();
	pResultFIR->allocateContent(numSamples*NUM_CHANNELS);
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
		pNXCOR[i]->updateTemplate(pTemplate[i]->getTemplate(), pTemplate[i]->getMean());
	}
	return 0;
}

void TemplateMatch::processResults(void) {
	// TODO implement threshold selection
	printf("NXCOR template1 %f\r\n", mNXCORRes[0]);
	printf("NXCOR template2 %f\r\n", mNXCORRes[1]);
}

void TemplateMatch::run()
{
	int count = mNumSamples;
    bool firstTime = true;
    int *pSampleData = (int *)lxRecord.board[0].data;

    updateCoefficients();
    updateTemplates();

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
		pResultFIR->appendData(mFiltered, NUM_CHANNELS);
		for (int i = 0; i < TEMP_NUM; i++) {
			pResultNXCOR[i]->appendData(&mNXCORRes[i], 1);
		}

		// Wait for one sample delay
		vTaskDelay( pdMS_TO_TICKS( 100 ) );
		//vTaskDelay( pdMS_TO_TICKS( 0.0333333 ) );

		firstTime = false;
		count--;
	}

	printf("Neuron template matching completed on %d samples\r\n", mNumSamples);
	// Save test result from memory to files
	pResultFIR->saveContent("FIRFiltered.bin");
	pResultNXCOR[0]->saveContent("NXCORTemp1.bin");
	pResultNXCOR[1]->saveContent("NXCORTemp2.bin");
	printf("Saved result to files FIRFiltered.bin, NXCORTemp1.bin and NXCORTemp2.bin\r\n");
}

