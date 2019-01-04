/*
 * NXCOR_HLS.cpp
 *
 *  Created on: 24. dec. 2018
 *      Author: Kim Bjerge
 */

#include "NXCOR_HLS.h"
#include <math.h>
#include <stdio.h>

void NXCOR::printSettings(void)
{
	printf("---------------------------------------------------------------------\r\n");
	printf("NXCOR template %2d settings:\r\n", mDeviceId+1);
	printf("  Length      : %d\r\n", mLength);
	printf("  Width       : %d\r\n", mWidth);
	printf("  Threshold   : %f\r\n", mNXCORThreshold);
	printf("  Peak max.   : ");
	for (int ch = 0; ch < TEMP_WIDTH; ch++)
		printf("%5d ", mPeakMaxLimits[ch]);
	printf("\r\n");
	printf("  Peak min.   : ");
	for (int ch = 0; ch < TEMP_WIDTH; ch++)
		printf("%5d ", mPeakMinLimits[ch]);
	printf("\r\n");
	printf("  Channel map : ");
	for (int ch = 0; ch < TEMP_WIDTH; ch++)
		printf("%5d ", mChannelMap[ch]);
	printf("\r\n");
	//printf("  Peak max : %d\r\n", mMaxPeakThreshold); // NOT USED OLD VER. 1.0
	//printf("  Peak min : %d\r\n", mMinPeakThreshold); // NOT USED OLD VER. 1.0
}

void NXCOR::updateTemplate(TTYPE *temp, int avgTemp)
{
	while (XNxcor_IsReady(&mNXCOR) == 0); // Polling ready register
	XNxcor_Set_width(&mNXCOR, mWidth);
	XNxcor_Set_length_r(&mNXCOR, mLength);
	XNxcor_Write_templateData_Words(&mNXCOR, 0, (int *)temp, (TEMP_SIZE+1)/SINT); //TTYPE = int
	//XNxcor_Write_templateData_Bytes(&mNXCOR, 0, (char *)temp, mLength*mWidth*sizeof(TTYPE));
	//XNxcor_Set_avgTemp(&mNXCOR, average);
	*(volatile int*)(mNXCOR.Axilites_BaseAddress + XNXCOR_AXILITES_ADDR_AVGTEMP_DATA) = avgTemp;
}

void NXCOR::startNXCOR(TTYPE *samples)
{
#if 0 // Used if TTYPE different from STYPE
    TTYPE samplesType[TEMP_WIDTH+1];
    int i;
    // Convert samples to TTYPE used by NXCOR
    for (i = 0; i < TEMP_WIDTH; i++) samplesType[i] = samples[i];
    samplesType[i] = 0;
	// Clear done flags
	mResultAvailHlsNXCOR = 0;
	while (XNxcor_IsReady(&mNXCOR) == 0); // Polling ready register
	XNxcor_Write_signalData_Words(&mNXCOR, 0, (int *)samplesType, (TEMP_WIDTH+1)/SINT); //TTYPE = int
#else
	int ch;
    TTYPE samplesMoved[TEMP_WIDTH+1];
    // Use channel map to move samples from neuron channels where template is located
    for (ch = 0; ch < TEMP_WIDTH; ch++) {
        if (ch < mWidth)
        	samplesMoved[ch] = samples[mChannelMap[ch]];
        else
            samplesMoved[ch] = 0;
    }
    samplesMoved[ch] = 0;
	mResultAvailHlsNXCOR = 0;
	while (XNxcor_IsReady(&mNXCOR) == 0); // Polling ready register
	XNxcor_Write_signalData_Words(&mNXCOR, 0, (int *)samplesMoved, (TEMP_WIDTH+1)/SINT); //TTYPE = int
#endif
	//XNxcor_Write_signalData_Bytes(&mNXCOR, 0, (char *)samplesType, mWidth*sizeof(TTYPE));
	XNxcor_Start(&mNXCOR);

	updateLastSamples(samplesMoved);
	//updatePeakSamples(samples); // NOT USED - Ver. 1.0
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

float NXCOR::executeNXCOR(TTYPE *samples, float varTemplate)
{
	startNXCOR(samples);
	return readResultNXCOR(varTemplate);
}

// NOT USED - Ver. 1.0
void NXCOR::updatePeakSamples(TTYPE *samples)
{
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

// NOT USED - Ver. 1.0
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
	if (mPeakSample >= mMinPeakThreshold &&
		mPeakSample <= mMaxPeakThreshold)
		return true;

	return false;
}

void NXCOR::setChannelMap(short *map)
{
	// Set channel map of index to neuron channels where template is located
	for (int ch = 0; ch < TEMP_WIDTH; ch++)
		mChannelMap[ch] = map[ch];
}

void NXCOR::updateLastSamples(TTYPE *samples)
{
	memcpy(mLastSamples[mLastIdx], samples, sizeof(TTYPE)*TEMP_WIDTH);
	mLastIdx = (mLastIdx+1)%mLength;
}

bool NXCOR::checkWithinChannelPeakLimits(void)
{
	// Set minimum values to first sample in last sample buffer
	for (int ch = 0; ch < mWidth; ch++)
		mPeakMin[ch] = mLastSamples[0][ch];

	// Search for minimum sample value for each channel in sample buffer
	for (int i = 1; i < mLength; i++) {
		for (int ch = 0; ch < mWidth; ch++) {
			if (mPeakMin[ch] > mLastSamples[i][ch])
				mPeakMin[ch] = mLastSamples[i][ch];
		}
	}

	// Check that found minimum sample is within valid limits
	for (int ch = 0; ch < mWidth; ch++) {
		if (mPeakMin[ch] > mPeakMaxLimits[ch] ||
			mPeakMin[ch] < mPeakMinLimits[ch])
			return false;
	}

	return true;
}

void NXCOR::setMaxPeakLimits(TTYPE *max)
{
	// Set max peak limits for each channel
	for (int i = 0; i < TEMP_WIDTH; i++)
		mPeakMaxLimits[i] = max[i];
}

void NXCOR::setMinPeakLimits(TTYPE *min)
{
	// Set min peak limits for each channel
	for (int i = 0; i < TEMP_WIDTH; i++)
		mPeakMinLimits[i] = min[i];
}

int NXCOR::verifyActivation(void)
{
    mActiveState = 0;

	if (mActivationCounts > 0) {
		mActivationCounts--; // Filter on minimum distance in samples between neuron activations
		if (mActivationCounts == 0)
			mActiveState = 3;
		else
			mActiveState = 2;
	} else {
		if (mResultNXCOR >= mNXCORThreshold) { // NXCOR above threshold then match found and activation detected
			//if (checkWithinPeakLimits()) { // Check whether neuron peak is within valid limits VER. 1.0
			if (checkWithinChannelPeakLimits()) { // Check whether neuron peak is within valid limits
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
