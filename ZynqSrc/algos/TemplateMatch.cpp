/*
 * FirHLS.cpp
 *
 *  Created on: 16. aug. 2017
 *      Author: Kim Bjerge
 */

#include "TemplateMatch.h"
#include "xparameters.h"


int TemplateMatch::Init(IRQ* pIrq)
{
	for (int i = 0; i < FIR_NUM; i++) {
		pFirFilter[i] = new FirFilter(XPAR_FIRFILTER_0_DEVICE_ID+i, FIR_SIZE, FIR_TAPS);
		pFirFilter[i]->Init(pIrq, XPAR_FABRIC_FIRFILTER_0_INTERRUPT_INTR+i);
	}
	for (int i = 0; i < TEMP_NUM; i++) {
		pNXCOR[i] = new NXCOR(XPAR_NXCOR_0_DEVICE_ID+i, TEMP_LENGTH, TEMP_WIDTH);
		pNXCOR[i]->Init(pIrq, XPAR_FABRIC_NXCOR_0_INTERRUPT_INTR+i);
	}
	return 0;
}

int TemplateMatch::updateCoefficients()
{
	// TODO implement setting coefficients
	for (int i = 0; i < FIR_NUM; i++) {
		pFirFilter[i]->updateCoefficients(mCoeff);
	}
	for (int i = 0; i < NUM_CHANNELS; i++)
		mFiltered[i] = 0;

	return 0;
}

int TemplateMatch::updateTemplates()
{
	// TODO implement updating of templates
	for (int i = 0; i < TEMP_NUM; i++) {
		pNXCOR[i]->updateTemplate((int *)&mTemplates[i*TEMP_SIZE]);
	}
	return 0;
}

void TemplateMatch::processResults(void) {
	// TODO implement threshold selection

}

void TemplateMatch::run()
{
    bool firstTime = true;
    int *pSampleData = (int *)lxRecord.board[0].data;

    updateCoefficients();
    updateTemplates();

	while (1) {

		// Get next sample from data generator
		pNeuronData->GenerateSampleRecord(&lxRecord);

		if (!firstTime) {
			// After fist iteration read filtered samples
			for (int i = 0; i < FIR_NUM; i++) {
				pFirFilter[i]->readFiltered(&mFiltered[i*FIR_SIZE]);
			}
			// Read result of normalized cross core correlation
			for (int i = 0; i < TEMP_NUM; i++) {
				mNXCORRes[i] = pNXCOR[i]->readResultNXCOR();
			}
		}

		// Start filtering next NUM_CHANNELS of samples
		for (int i = 0; i < FIR_NUM; i++) {
			pFirFilter[i]->startFilter((int *)&pSampleData[i*FIR_SIZE]);
		}
		// Start normalized cross core correlation of filtered samples
		for (int i = 0; i < TEMP_NUM; i++) {
			pNXCOR[i]->startNXCOR((int *)&mFiltered[i]);
		}

		processResults();

		// Wait for one sample delay
		vTaskDelay( pdMS_TO_TICKS( 100 ) );
		//vTaskDelay( pdMS_TO_TICKS( 0.0333333 ) );

		firstTime = false;
	}
}

