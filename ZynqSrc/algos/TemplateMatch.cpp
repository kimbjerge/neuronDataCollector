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

// Define dummy interrupts if not used
#ifndef XPAR_FABRIC_FIRFILTER_0_INTERRUPT_INTR
#define XPAR_FABRIC_FIRFILTER_0_INTERRUPT_INTR 	60
#define XPAR_FABRIC_NXCOR_0_INTERRUPT_INTR 		70
#define XPAR_FABRIC_NXCOR_4_INTERRUPT_INTR 		74
#define XPAR_FABRIC_NXCOR_5_INTERRUPT_INTR 		75
#endif

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

int TemplateMatch::Init(Config *pConfig, int numSamples, IRQ* pIrq)
{

	// Create FIR filters
	for (int i = 0; i < FIR_NUM; i++) {
		pFirFilter[i] = new FirFilter(XPAR_FIRFILTER_0_DEVICE_ID+i, FIR_SIZE, FIR_TAPS);
		pFirFilter[i]->Init(pIrq, XPAR_FABRIC_FIRFILTER_0_INTERRUPT_INTR+i);
	}
	if (pConfig->isTabsValid())
		pCoeffFloat = pConfig->getCoeffs(); // Use coefficients from FIR.txt file
	else {
		pCoeffFloat = (float *)FIR_coeffs; // Use default coefficients
		printf("Using default FIR coefficients number of taps %d\r\n", FIR_TAPS);
	}

	// Create NXCOR filters
	for (int i = 0; i < TEMP_NUM; i++) {
		pNXCOR[i] = new NXCOR(XPAR_NXCOR_0_DEVICE_ID+i, pConfig->getLength(i), pConfig->getWidth(i));
		if (i < 4)  pNXCOR[i]->Init(pIrq, XPAR_FABRIC_NXCOR_0_INTERRUPT_INTR+i);
		if (i == 4)	pNXCOR[i]->Init(pIrq, XPAR_FABRIC_NXCOR_4_INTERRUPT_INTR);
		if (i == 5) pNXCOR[i]->Init(pIrq, XPAR_FABRIC_NXCOR_5_INTERRUPT_INTR);
		pTemplate[i] = new Template();

		if (i < pConfig->getNumTemplates()) {
			// Load template from file defined by pTempNames
			pTemplate[i]->loadTemplate(pConfig->getTemplateName(i), pConfig->getLength(i), pConfig->getWidth(i));
			pNXCOR[i]->setNXCORThreshold(pConfig->getThreshold(i));
			pNXCOR[i]->setPeakThreshold(pConfig->getMin(i), pConfig->getMax(i));
			pNXCOR[i]->setMaxPeakLimits(pConfig->getPeakMaxLimits(i));
			pNXCOR[i]->setMinPeakLimits(pConfig->getPeakMinLimits(i));
			pNXCOR[i]->setMinGradient(pConfig->getMinGradient(i));
			pNXCOR[i]->setChannelMap(pConfig->getChannelMap(i));
			pNXCOR[i]->printSettings();
		} else
			pTemplate[i]->clearTemplate();
	}

#ifdef DEBUG_FILES
	for (int i = 0; i < TEMP_NUM; i++) {
		pResultNXCOR[i] = new ResultFile<float>();
		pResultNXCOR[i]->allocateContent(numSamples);
	}
	pResultFIR = new ResultFile<STYPE>();
	pResultFIR->allocateContent(numSamples*NUM_CHANNELS);
#endif

	mNumCfgTemplates = pConfig->getNumTemplates();
	mNumSamples = numSamples;
	// Set sample counter used to trigger when template 1 and 2 is seen at the same time
	if (mNumCfgTemplates >= 2) {
		mTemplate12Counter = pConfig->getCounter(0);
		printf("Trigger output JB9/LD7 when template 1 and 2 seen within %d samples\r\n", mTemplate12Counter);
	}
	else
		mTemplate12Counter = 0;

	mpConfig = pConfig;

	return 0;
}

void TemplateMatch::updateCoefficients()
{
	// Updating coefficients based on table with double numbers
	if (mpConfig->isAllTabsValid()) {
		// Update FIR filters with taps from binary configuration file
		for (int ch = 0; ch < NUM_CHANNELS; ch++) {
			float *pTabs = mpConfig->getCoeffsAll(ch);
			// Convert coefficients from float to integer format 1.23
			for (int i = 0; i < FIR_TAPS; i++) {
				mCoeff[i] = (int)round(pTabs[i]*pow(2,FIR_FORMAT)); // Convert to format 1.FIR_FORMAT
			}
			int i = ch/FIR_SIZE; // Index to FIR HLS core
			int j = ch%FIR_SIZE; // Index to FIR filter within HLS core
			pFirFilter[i]->updateCoefficients(mCoeff, j);
		}
		printf("Used FIR coefficients for 32 individual channels\n\r");
	}
	else {
		// Update FIR filters with same taps
		for (int i = 0; i < FIR_TAPS; i++) {
			mCoeff[i] = (int)round(pCoeffFloat[i]*pow(2,FIR_FORMAT)); // Convert to format 1.FIR_FORMAT
		}
		for (int i = 0; i < FIR_NUM; i++) {
			for (int ch = 0; ch < FIR_SIZE; ch++)
				pFirFilter[i]->updateCoefficients(mCoeff, ch);
		}
		printf("Used same FIR coefficients for all 32 channels\n\r");
	}
}

void TemplateMatch::updateTemplates()
{
	// Update template in HLS IP core
	for (int i = 0; i < TEMP_NUM; i++) {
		pNXCOR[i]->updateTemplate(pTemplate[i]->getTemplate(), round(pTemplate[i]->getMean()));
	}
}

void TemplateMatch::clearIPCoresMemory()
{
	for (int i = 0; i < NUM_CHANNELS; i++)
		mFiltered[i] = 0;

	// Clear filter IP Core memory
	for (int i = 0; i < FIR_TAPS; i++) {
		for (int i = 0; i < FIR_NUM; i++)
			pFirFilter[i]->executeFilter(mFiltered);
	}
	// Update template in HLS IP core
	for (int i = 0; i < TEMP_NUM; i++) {
		for (int j = 0; j < TEMP_LENGTH; j++) { // Clear IP Core memory
			pNXCOR[i]->executeNXCOR(mFiltered, pTemplate[i]->getVariance());
		}
	}
}

void TemplateMatch::processResults(void)
{
	for (int i = 0; i < mNumCfgTemplates; i++) {
		int state = pNXCOR[i]->verifyActivation();
		if (state == 1) {
			//printf("%06d %04d %s %.3f P%05d\r\n",
			printf("%06d %04d %s %.3f\r\n",
					mCount,
					pNXCOR[i]->getNumActivations(),
					pTemplate[i]->getTemplateName(),
					pNXCOR[i]->getNXCORResult());
					//pNXCOR[i]->getMaxPeak());
			leds.setOn((Leds::LedTypes)i, true);
			testOut.setOn((TestIO::IOTypes)(i+TestIO::JB1), true);
		}
		if (state == 3) {
			leds.setOn((Leds::LedTypes)i, false);
			testOut.setOn((TestIO::IOTypes)(i+TestIO::JB1), false);
		}
	}
}

void TemplateMatch::triggerTemplate12(void)
{
	if (mTemplate12Counter > 0) {
		if (mTemplate12Trigger == 1) { // Reset trigger when expired
			testOut.setOn(TestIO::JB9, false);
			leds.setOn(Leds::LED7, false);
		}
		for (int i = 0; i < 2; i++) { // Check for neuron template 1 and 2
			if (pNXCOR[i]->getActiveState() == 1) {
				if (mTemplate12Trigger > 0 && mTemplate12TriggerIdx == (i+1)%2) {
					// Another neuron activated in same time interval
					testOut.setOn(TestIO::JB9, true);
					leds.setOn(Leds::LED7, true);
					mTemplate12Trigger = 31; // Keep trigger high in min. 1 ms
					printf("%06d TRIG %d\r\n", mCount, i+1);
				} else {
					mTemplate12Trigger = mTemplate12Counter; // First neuron activation
					mTemplate12TriggerIdx = i;
				}
			}
		}
		if (mTemplate12Trigger > 0) mTemplate12Trigger--; // Decrement trigger
	}
}

void TemplateMatch::reset(void)
{
	pNeuronData->reset();
	pResultFIR->reset();
	for (int i = 0; i < TEMP_NUM; i++)
		pResultNXCOR[i]->reset();
	for (int i = 0; i < TEMP_NUM; i++)
		pNXCOR[i]->reset();

	mTemplate12Trigger = 0;
	mTemplate12TriggerIdx = 0;
}

void TemplateMatch::run()
{
	int count = mNumSamples;
    STYPE *pSampleData;
    int start_tick, end_tick;

	printf("Updating FIR coefficients and templates\r\n");
    updateCoefficients();
    updateTemplates();
	printf("Neuron template matching running\r\n");

	while (1)
	{
		// Clear NXCORE and FIR filter IP Core's HW Memory
		clearIPCoresMemory();

		testOut.setOn(TestIO::JB9, false); // Clear digital outputs
		testOut.setOn(TestIO::JB10, false);
		printf("Turn SW0 on (ZedBoard) or start acquisition on Digital Lynx SX\r\n");
		while (!sw.isOn(Switch::SW0))
			Sleep(100);

		// Reset counters and indexes
		reset();
		mCount = 0;
		count = mNumSamples;

		while (count > 0) {

			// Get next sample from data generator
			pSampleData = pNeuronData->GenerateSamples();

			if (mCount < 1) { // First time
				// LED + Hardware signals for debugging
				leds.setOn(Leds::LED6 , true);
				testOut.setOn(TestIO::JB10, true);
				printf("Acquisition started, sample %d\r\n", mCount+1);
				// Read start counter
				start_tick = xTaskGetTickCount();
			} else {
				// After fist iteration read filtered samples
				for (int i = 0; i < FIR_NUM; i++) {
					pFirFilter[i]->readFiltered(&mFiltered[i*FIR_SIZE]);
				}
				// Read result of normalized cross core correlation
				for (int i = 0; i < mNumCfgTemplates; i++) {
					pNXCOR[i]->readResultNXCOR(pTemplate[i]->getVariance());
				}
			}

			// Start filtering next NUM_CHANNELS of samples
			for (int i = 0; i < FIR_NUM; i++) {
				pFirFilter[i]->startFilter(&pSampleData[i*FIR_SIZE]);
			}
			// Start normalized cross core correlation of filtered samples
			for (int i = 0; i < mNumCfgTemplates; i++) {
				pNXCOR[i]->startNXCOR(mFiltered);
				//pNXCOR[i]->startNXCOR(&mFiltered[pTemplate[i]->getChOffset()]); OLD Ver. 1.0
			}

			processResults();
			triggerTemplate12();

			// Append test result to memory
#ifdef DEBUG_FILES
			pResultFIR->appendData(mFiltered, NUM_CHANNELS);
			for (int i = 0; i < mNumCfgTemplates; i++) {
				float NXCORRes = pNXCOR[i]->getNXCORResult();
				if (pNXCOR[i]->getActiveState() == 1)
					NXCORRes = 1.0; // Set to max when active
				pResultNXCOR[i]->appendData(&NXCORRes, 1);
			}
#endif
			// Wait for one sample delay
			//printf(".");
			//vTaskDelay( pdMS_TO_TICKS( 1 ) );
			//vTaskDelay( pdMS_TO_TICKS( 0.0333333 ) );
			count--;
			mCount++;
		}
		end_tick = xTaskGetTickCount();

		testOut.setOn(TestIO::JB9, false); // Clear trigger
		leds.setOn(Leds::LED7, false);
		testOut.setOn(TestIO::JB10, false);
		leds.setOn(Leds::LED6 , false);

		printf("Tick start %d and tick end %d, duration = %d ms\r\n", start_tick, end_tick, (1000*(end_tick-start_tick))/configTICK_RATE_HZ);
		printf("Neuron template matching completed after %d samples\r\n", mNumSamples);
		printf("Saving results to binary files - please wait....\r\n");

#ifdef DEBUG_FILES
		// Save test result from memory to files
		pResultFIR->saveContent("FIRFilt.bin");
		for (int i = 0; i < mNumCfgTemplates; i++) {
			char name[20];
			sprintf(name, "NXCORT%d.bin", i+1);
			pResultNXCOR[i]->saveContent(name);
		}
		printf("Saved result to files FIRFilt.bin and NXCORT1-%d.bin\r\n", mNumCfgTemplates);
#endif
		// Clear sample interrupts
		pSampleData = pNeuronData->GenerateSamples();

	} // while (1)

}

