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

void TemplateMatch::printSettings(char *buf)
{
	for (int i = 0; i < TEMP_NUM; i++) {
		if (i < mpConfig->getNumTemplates()) {
			pNXCOR[i]->printSettings(buf);
		} else {
			printf("NXCOR template %d is not used\r\n", i+1);
		}
	}
}

void TemplateMatch::updateTemplateData(int id, float *data,  int length, int width)
{
	mpConfig->setSize(id, length, width);
    pTemplate[id]->updateData(data, length, width, id+1);
}

void TemplateMatch::updateConfig(int numSamples)
{
	if (mpConfig->isTabsValid())
		pCoeffFloat = mpConfig->getCoeffs(); // Use coefficients from FIR.txt file
	else {
		pCoeffFloat = (float *)FIR_coeffs; // Use default coefficients
		printf("Using default FIR coefficients number of taps %d\r\n", FIR_TAPS);
	}

	mNumCfgTemplates = mpConfig->getNumTemplates();

	for (int i = 0; i < TEMP_NUM; i++) {
		if (i < mNumCfgTemplates) {
			pNXCOR[i]->setSize(mpConfig->getLength(i), mpConfig->getWidth(i));
			pNXCOR[i]->setNXCORThreshold(mpConfig->getThreshold(i));
			pNXCOR[i]->setPeakThreshold(mpConfig->getMin(i), mpConfig->getMax(i));
			pNXCOR[i]->setMaxPeakLimits(mpConfig->getPeakMaxLimits(i));
			pNXCOR[i]->setMinPeakLimits(mpConfig->getPeakMinLimits(i));
			pNXCOR[i]->setMinGradient(mpConfig->getMinGradient(i));
			pNXCOR[i]->setChannelMap(mpConfig->getChannelMap(i));
			pNXCOR[i]->printSettings();
		} else
			pTemplate[i]->clearTemplate();
	}

	mNumSamples = numSamples;
	// Set sample counter used to trigger when template 1 and 2 is seen at the same time
	if (mNumCfgTemplates >= 2) {
		mTemplate12Counter = mpConfig->getCounter(0);
		printf("Trigger output JB9/LD7 when template 1 and 2 seen within %d samples\r\n", mTemplate12Counter);
	}
	else
		mTemplate12Counter = 0;
}

int TemplateMatch::Init(Config *pConfig, int numSamples, IRQ* pIrq)
{
	mpConfig = pConfig;

	// Create FIR filters
	for (int i = 0; i < FIR_NUM; i++) {
		pFirFilter[i] = new FirFilter(XPAR_FIRFILTER_0_DEVICE_ID+i, FIR_SIZE, FIR_TAPS);
		pFirFilter[i]->Init(pIrq, XPAR_FABRIC_FIRFILTER_0_INTERRUPT_INTR+i);
	}

	// Create NXCOR filters and templates
	for (int i = 0; i < TEMP_NUM; i++) {
		pNXCOR[i] = new NXCOR(XPAR_NXCOR_0_DEVICE_ID+i, mpConfig->getLength(i), mpConfig->getWidth(i));
		if (i < 4)  pNXCOR[i]->Init(pIrq, XPAR_FABRIC_NXCOR_0_INTERRUPT_INTR+i);
		if (i == 4)	pNXCOR[i]->Init(pIrq, XPAR_FABRIC_NXCOR_4_INTERRUPT_INTR);
		if (i == 5) pNXCOR[i]->Init(pIrq, XPAR_FABRIC_NXCOR_5_INTERRUPT_INTR);
		pTemplate[i] = new Template();
		if (i < mpConfig->getNumTemplates()) {
			// Load template from file defined by pTempNames
			pTemplate[i]->loadTemplate(mpConfig->getTemplateName(i), mpConfig->getLength(i), mpConfig->getWidth(i));
		} else
			pTemplate[i]->clearTemplate();
	}

#ifdef DEBUG_FILES
	for (int i = 0; i < TEMP_NUM; i++) {
		pResultNXCOR[i] = new ResultFile<float>();
		pResultNXCOR[i]->allocateContent(SAMPLES_SAVED);
	}
	pResultFIR = new ResultFile<STYPE>();
	pResultFIR->allocateContent(SAMPLES_SAVED*NUM_CHANNELS);
#endif

	updateConfig(numSamples);

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
			// Delay 1.2 ms delay
			if (mPrintDebug)
				xil_printf("%06d %04d %s %02d\r\n",
							mCount,
							pNXCOR[i]->getNumActivations(),
							pTemplate[i]->getTemplateName(),
							(int)round(pNXCOR[i]->getNXCORResult()*100)); // Percentage
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
					if (mPrintDebug)
						xil_printf("%06d TRIG %d\r\n", mCount, i+1);
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
    STYPE *pSampleData;
    int start_tick, end_tick;

	printf("Updating FIR coefficients and templates\r\n");
    updateCoefficients();
    updateTemplates();
	printf("Neuron template matching running\r\n");
	mRunning = true;

	while (mRunning)
	{
		// Clear NXCORE and FIR filter IP Core's HW Memory
		clearIPCoresMemory();

		testOut.setOn(TestIO::JB9, false); // Clear digital outputs
		testOut.setOn(TestIO::JB10, false);
		printf("Turn SW0 on (ZedBoard) or start acquisition on Digital Lynx SX\r\n");
		while (!sw.isOn(Switch::SW0) && mRunning) {
			Sleep(100);
		}

		// Break if task stopped
		if (!mRunning) break;

		// Reset counters and indexes
		reset();
		mCount = 0;
		mCounter = mNumSamples;
		printf("Acquisition running for %.2f seconds\r\n", (float)mCounter/Fs_RATE);

		start_tick = xTaskGetTickCount();

		while (mCounter > 0) {

			// Get next sample from data generator
			pSampleData = pNeuronData->GenerateSamples();

			if (mCount < 1) { // First time
				//xil_printf("Acquisition started count %d, c0 %d, c1 %d.. c30 %d, c31 %d\r\n",
				//		   mCount+1, pSampleData[0], pSampleData[1], pSampleData[30], pSampleData[31]);
				// LED + Hardware signals for debugging
				leds.setOn(Leds::LED6 , true);
				// Read start counter
				start_tick = xTaskGetTickCount();
				testOut.setOn(TestIO::JB10, true);
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
			if (mCount < SAMPLES_SAVED) {
				if (mSaveRawData) // Save raw data from HPP
					pResultFIR->appendData(pSampleData, NUM_CHANNELS);
				else // Save filtered data
					pResultFIR->appendData(mFiltered, NUM_CHANNELS);
				for (int i = 0; i < mNumCfgTemplates; i++) {
					float NXCORRes = pNXCOR[i]->getNXCORResult();
					if (pNXCOR[i]->getActiveState() == 1)
						NXCORRes = 1.0; // Set to max when active
					pResultNXCOR[i]->appendData(&NXCORRes, 1);
				}
			}
#endif
			// Wait for one sample delay
			//printf(".");
			//vTaskDelay( pdMS_TO_TICKS( 1 ) );
			//vTaskDelay( pdMS_TO_TICKS( 0.0333333 ) );
			mCounter--;
			mCount++;
		}
		end_tick = xTaskGetTickCount();

		testOut.setOn(TestIO::JB9, false); // Clear trigger
		leds.setOn(Leds::LED7, false);
		testOut.setOn(TestIO::JB10, false);
		leds.setOn(Leds::LED6 , false);

		printf("Tick start %d and tick end %d, duration = %d ms\r\n", start_tick, end_tick, (1000*(end_tick-start_tick))/configTICK_RATE_HZ);
		printf("Neuron template matching completed after %d samples\r\n", mCount);
		printf("Saving results to binary files - please wait....\r\n");

#ifdef DEBUG_FILES
		// Save test result from memory to files
		for (int i = 0; i < mNumCfgTemplates; i++) {
			char name[20];
			sprintf(name, "NXCORT%d.bin", i+1);
			pResultNXCOR[i]->saveContent(name);
		}
		if (mSaveRawData) {
			pResultFIR->saveContent("RAWData.bin");
			printf("Saved result to files RAWData.bin and NXCORT1-%d.bin\r\n", mNumCfgTemplates);
		} else {
			pResultFIR->saveContent("FIRFilt.bin");
			printf("Saved result to files FIRFilt.bin and NXCORT1-%d.bin\r\n", mNumCfgTemplates);
		}
#endif
		// Clear sample interrupts
		pSampleData = pNeuronData->GenerateSamples();
		mRunning = false; // Only one iteration

	} // while (mRunning)
    printf("Neuron template matching thread shutting down and exit\r\n");
    vTaskDelete(NULL);
}

