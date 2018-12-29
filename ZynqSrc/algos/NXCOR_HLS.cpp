/*
 * NXCOR_HLS.cpp
 *
 *  Created on: 24. dec. 2018
 *      Author: Kim Bjerge
 */

#include "NXCOR_HLS.h"
#include <math.h>

void NXCOR::updateTemplate(int *temp, int avgTemp)
{
	while (XNxcor_IsReady(&mNXCOR) == 0); // Polling ready register
	XNxcor_Write_templateData_Words(&mNXCOR, 0, temp, mLength*mWidth);

	//XNxcor_Set_avgTemp(&mNXCOR, average);
	*(volatile int*)(mNXCOR.Axilites_BaseAddress + XNXCOR_AXILITES_ADDR_AVGTEMP_DATA) = avgTemp;
}

void NXCOR::startNXCOR(int *samples)
{
	// Clear done flags
	mResultAvailHlsNXCOR = 0;
	while (XNxcor_IsReady(&mNXCOR) == 0); // Polling ready register
	XNxcor_Write_signalData_Words(&mNXCOR, 0, samples, mWidth);
	XNxcor_Start(&mNXCOR);

	// Update maximum sample value
	int peakSample = samples[0];
	for (int i = 1; i < mWidth; i++) {
		int peak = samples[i];
		if (abs(peak) > abs(peakSample))
			peakSample = peak;
	}
	mPeakSamples[mIdxPeak] = peakSample;
	mIdxPeak = (mIdxPeak + 1) % mLength;
}

float NXCOR::readResultNXCOR(float varTemplate)
{
	uint64_t u64Variance;
	uint32_t u32ResultLow;
	int64_t i64ResultHigh;
	int64_t i64Result;

	//if (mpIrq != 0)	// Busy wait for fir irq
	//	while(!mResultAvailHlsNXCOR);
	//else
		while (XNxcor_IsDone(&mNXCOR) == 0); // Polling done register

	//i64Result = XNxcor_Get_result(&mNXCOR);
	u32ResultLow = *(volatile uint32_t*)(mNXCOR.Axilites_BaseAddress + XNXCOR_AXILITES_ADDR_RESULT_DATA);
	i64ResultHigh = *(volatile int32_t*)(mNXCOR.Axilites_BaseAddress + XNXCOR_AXILITES_ADDR_RESULT_DATA + 4);
	i64Result = (i64ResultHigh << 32) + u32ResultLow;

	u64Variance = XNxcor_Get_varSig(&mNXCOR);
	mResult = i64Result;
	mVarianceSignal = u64Variance;
	mVarianceTemplate = varTemplate;
	mResultNXCOR = mResult / sqrt(mVarianceSignal * mVarianceTemplate);
    return mResultNXCOR;
}

float NXCOR::executeNXCOR(int *samples, float varTemplate)
{
	startNXCOR(samples);
	return readResultNXCOR(varTemplate);
}

bool NXCOR::checkWithinPeakLimits(void)
{
	// Search for peak sample in length of template window
	mPeakSample = mPeakSamples[0];
	for (int i = 1; i < mLength; i++) {
		int peak = mPeakSamples[i];
		if (abs(peak) > abs(mPeakSample))
			mPeakSample = peak;
	}
	// Check that peak sample is within limits
	// TODO check if negative and positive values are important
	if (abs(mPeakSample) >= abs(mMinPeakThreshold) &&
		abs(mPeakSample) <= abs(mMaxPeakThreshold))
		return true;

	return false;
}

int NXCOR::verifyActivation(void)
{
    mActiveState = 0;

	if (mActivationCounts > 0) {
		mActivationCounts--; // Filter on minimum distance in samples between neuron activations
		mActiveState = 2;
	} else {
		if (mResultNXCOR >= mNXCORThreshold) { // NXCOR above threshold then match found and activation detected
			if (checkWithinPeakLimits()) { // Check whether neuron peak is within valid limits
				mActivationCounts = mMaxActivationCount; // Set filter to ignore future activations
				mCounts++;
				mActiveState = 1;
			}
		}
	}
	return mActiveState;
}

void NXCOR::Disable(void)
{
	// Disable Global and instance interrupts
    XNxcor_InterruptDisable(&mNXCOR, 1);
    XNxcor_InterruptGlobalDisable(&mNXCOR);
	// clear the local interrupt
	XNxcor_InterruptClear(&mNXCOR, 1);
}

void NXCOR::Enable(void)
{
	// Enable Global and instance interrupts
	mResultAvailHlsNXCOR = 0;
    XNxcor_InterruptEnable(&mNXCOR,1);
    XNxcor_InterruptGlobalEnable(&mNXCOR);
}

void NXCOR::hls_NXCOR_isr(void *InstancePtr)
{
	NXCOR *pHlsNXCOR = (NXCOR *)InstancePtr;
	XNxcor *pAccelerator = &(pHlsNXCOR->mNXCOR);

	// clear the local interrupt
	XNxcor_InterruptClear(pAccelerator, 1);

	pHlsNXCOR->mResultAvailHlsNXCOR = 1;
}

int NXCOR::Init(IRQ* pIrq, int mIrqDeviceId)
{
	XNxcor_Config *cfgPtr;
	int status;

	// Set IRQ pointer
	mpIrq = pIrq;

	cfgPtr = XNxcor_LookupConfig(mDeviceId);
	if (!cfgPtr) {
	  print("ERROR: Lookup of FIR configuration failed.\n\r");
	  return XST_FAILURE;
	}

	status = XNxcor_CfgInitialize(&mNXCOR, cfgPtr);
	if (status != XST_SUCCESS) {
	  print("ERROR: Could not initialize FIR.\n\r");
	  return XST_FAILURE;
	}

	// Disable and clear interrupts from FIR
	Disable();

	if (mpIrq != 0) { // Enable interrupt if interrupt controller specified
		// Connect the Left FIR ISR to the exception table
		status = XScuGic_Connect(mpIrq->getScuGic(), mIrqDeviceId,
								 (Xil_InterruptHandler)hls_NXCOR_isr, this);
		if(status != XST_SUCCESS){
		   return status;
		}

		// Enable the FIR ISR
		XScuGic_Enable(mpIrq->getScuGic(), mIrqDeviceId);
	}

	return status;
}
