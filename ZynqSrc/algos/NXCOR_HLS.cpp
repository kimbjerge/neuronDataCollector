/*
 * NXCOR_HLS.cpp
 *
 *  Created on: 24. dec. 2018
 *      Author: Kim Bjerge
 */

#include "NXCOR_HLS.h"
#include <math.h>
#include <stdio.h>

void NXCOR::printSettings(char *buf)
{
	char text[200];

	if (buf == 0) {
		// Print to USB-UART output
		printf("---------------------------------------------------------------------\r\n");
		printf("NXCOR template %2d settings:\r\n", mDeviceId+1);
		printf("  Length       : %d\r\n", mLength);
		printf("  Width        : %d\r\n", mWidth);
		printf("  Threshold    : %f\r\n", mNXCORThreshold);
		//printf("  Gradient min : %d\r\n", mPeakMinGradient); // Ver. 3.x
		printf("  Coherency min: %d\r\n", mCoherencyMinLimit);
		printf("  Coherency max: %d\r\n", mCoherencyMaxLimit);
		printf("  Peak idx min : %d\r\n", mIndexTMin);
		printf("  Peak idx max : %d\r\n", mIndexTMax);
		printf("  Peak max.    : ");
		for (int ch = 0; ch < TEMP_WIDTH; ch++)
			printf("%5d ", mPeakMaxLimits[ch]);
		printf("\r\n");
		printf("  Peak min.   : ");
		for (int ch = 0; ch < TEMP_WIDTH; ch++)
			printf("%5d ", mPeakMinLimits[ch]);
		printf("\r\n");
		printf("  Peak diff.  : ");
		for (int ch = 0; ch < TEMP_WIDTH; ch++)
			printf("%5d ", mPeakMinMaxLimits[ch]);
		printf("\r\n");
		printf("  Channel map : ");
		for (int ch = 0; ch < TEMP_WIDTH; ch++)
			printf("%5d ", mChannelMap[ch]);
		printf("\r\n");
	} else {
		// Print to text buffer output
		sprintf(text, "---------------------------------------------------------------------\r\n");
		strcat(buf, text);
		sprintf(text, "NXCOR template %2d settings:\r\n", mDeviceId+1);
		strcat(buf, text);
		sprintf(text, "  Length       : %d\r\n", mLength);
		strcat(buf, text);
		sprintf(text, "  Width        : %d\r\n", mWidth);
		strcat(buf, text);
		sprintf(text, "  Threshold    : %f\r\n", mNXCORThreshold);
		strcat(buf, text);
		//sprintf(text, "  Gradient min : %d\r\n", mPeakMinGradient);
		//strcat(buf, text);
		sprintf(text, "  Coherency min: %d\r\n", mCoherencyMinLimit);
		strcat(buf, text);
		sprintf(text, "  Coherency max: %d\r\n", mCoherencyMaxLimit);
		strcat(buf, text);
		sprintf(text, "  Peak idx min : %d\r\n", mIndexTMin);
		strcat(buf, text);
		sprintf(text, "  Peak idx max : %d\r\n", mIndexTMax);
		strcat(buf, text);
		sprintf(text, "  Peak max.    : ");
		strcat(buf, text);
		for (int ch = 0; ch < TEMP_WIDTH; ch++) {
			sprintf(text, "%5d ", mPeakMaxLimits[ch]);
			strcat(buf, text);
		}
		sprintf(text, "\r\n");
		strcat(buf, text);
		sprintf(text, "  Peak min.   : ");
		strcat(buf, text);
		for (int ch = 0; ch < TEMP_WIDTH; ch++) {
			sprintf(text, "%5d ", mPeakMinLimits[ch]);
			strcat(buf, text);
		}
		sprintf(text, "\r\n");
		strcat(buf, text);
		sprintf(text, "  Peak diff.  : ");
		strcat(buf, text);
		for (int ch = 0; ch < TEMP_WIDTH; ch++) {
			sprintf(text, "%5d ", mPeakMinMaxLimits[ch]);
			strcat(buf, text);
		}
		sprintf(text, "\r\n");
		strcat(buf, text);
		sprintf(text, "  Channel map : ");
		strcat(buf, text);
		for (int ch = 0; ch < TEMP_WIDTH; ch++) {
			sprintf(text, "%5d ", mChannelMap[ch]);
			strcat(buf, text);
		}
		sprintf(text, "\r\n");
		strcat(buf, text);
	}

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

	updateLastSamples(samplesMoved, samples);
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

void NXCOR::updateLastSamples(TTYPE *pSamples, TTYPE *pAllSamples)
{
	memcpy(mLastSamples[mLastIdx], pSamples, sizeof(TTYPE)*TEMP_WIDTH);
	memcpy(mAllLastSamples[mLastIdx], pAllSamples, sizeof(TTYPE)*NUM_CHANNELS);
	mLastIdx = (mLastIdx+1)%mLength;
}

bool NXCOR::checkPeakGradient(int ch)
{
	int offset, gradient;

	offset = mPeakMinIdx[ch] - mPeakMinOffset; // Find offset value from mimimum peak
	if (offset < 0) // Handle index wrap around
		offset = mLength - offset;

	// Calculate the gradient as difference between peak min. and offset value
	gradient = abs(mLastSamples[offset][ch] - mPeakMin[ch]);
	if (gradient < mPeakMinGradient) {
		//printf("Gradient %d too small ch %d\r\n", gradient, ch);
		return false;
	} else
		return true;
}

bool NXCOR::checkMinMaxDiffPeakLimits(int ch)
{
	TTYPE diffMinMax = mPeakMax[ch] - mPeakMin[ch];

	if (diffMinMax < mPeakMinMaxLimits[ch])
		return false;
	else {
		// printf("%d   %d\r\n", diffMinMax, mPeakMinMaxLimits[ch]);
		return true;

	}
}


// Check channels coherency using template max/min index
bool NXCOR::checkCoherencyTemplate(void)
{
	// Sum of all channel values at peak max and min in template
	int sumPeakMin = 0;
	int sumPeakMax = 0;

	// Index for min and max peaks relative to position in template
	int peakIdxMin = (mLastIdx+mIndexTMin)%mLength;
	int peakIdxMax = (mLastIdx+mIndexTMax)%mLength;

	//printf("Coherency: peakIdxMin %d, peakIdxMax %d\r\n", peakIdxMin, peakIdxMax);

	// Sum of all channels at index
	for (int ch = 0; ch < NUM_CHANNELS; ch++)
	{
		sumPeakMin += (int)mAllLastSamples[peakIdxMin][ch]; // Buffer with last samples for all channels equal to length of template
		sumPeakMax += (int)mAllLastSamples[peakIdxMax][ch];
	}
	// Subtracting samples from template channels at index
	for (int ch = 0; ch < mWidth; ch++)
	{
		sumPeakMin -= (int)mLastSamples[peakIdxMin][ch]; // Buffer with last samples where template match found
		sumPeakMax -= (int)mLastSamples[peakIdxMax][ch];
	}

	//printf("Coherency: sumPeakMin %d, sumPeakMax %d, mWidth %d\r\n", sumPeakMin, sumPeakMax, mWidth);

	// Calculate mean at min/max template peaks
    int meanHolderMin = round((float)sumPeakMin/(NUM_CHANNELS-mWidth));
    int meanHolderMax = round((float)sumPeakMax/(NUM_CHANNELS-mWidth));

    //printf("Coherency: meanHolderMin %d, meanHolderMax %d\r\n", meanHolderMin, meanHolderMax);

	if (meanHolderMin < mCoherencyMinLimit) return false;
    if (meanHolderMax > mCoherencyMaxLimit) return false;

    return true;
}

bool NXCOR::checkCoherency(void)
{
	// Start with mapped channel 0
	int peakChMin = 0;
	int peakChMax = 0;
	// Peaks for every channel
	TTYPE peakMin = mPeakMin[0];
	TTYPE peakMax = mPeakMax[0];

	// Search for channels with min/max peaks
	for (int ch = 1; ch < mWidth; ch++) {
		if (peakMin > mPeakMin[ch]) {
			peakMin = mPeakMin[ch];
			peakChMin = ch;
		}
		if (peakMax < mPeakMax[ch]) {
			peakMax = mPeakMax[ch];
			peakChMax = ch;
		}
	}

	// Sum of all channel values at peak max and min
	int sumPeakMin = 0;
	int sumPeakMax = 0;
	// Index for min and max peaks
	int peakIdxMin = mPeakMinIdx[peakChMin];
	int peakIdxMax = mPeakMaxIdx[peakChMax];

	// Sum of all channels at index
	for (int ch = 0; ch < NUM_CHANNELS; ch++)
	{
		sumPeakMin += (int)mAllLastSamples[peakIdxMin][ch]; // Buffer with last samples for all channels equal to length of template
		sumPeakMax += (int)mAllLastSamples[peakIdxMax][ch];
	}
	// Subtracting samples from template channels at index
	for (int ch = 0; ch < mWidth; ch++)
	{
		sumPeakMin -= (int)mLastSamples[peakIdxMin][ch]; // Buffer with last samples where template match found
		sumPeakMax -= (int)mLastSamples[peakIdxMax][ch];
	}

    // Calculate mean at min/max template peaks
    int meanHolderMin = round((float)sumPeakMin/(NUM_CHANNELS-mWidth));
    int meanHolderMax = round((float)sumPeakMax/(NUM_CHANNELS-mWidth));
    if (meanHolderMin < mCoherencyMinLimit) return false;
    if (meanHolderMax > mCoherencyMaxLimit) return false;

    return true;
}


// made by JH
/*
bool NXCOR::checkWithinChannelPeakLimits(void)
{
	// Set min/max values to first sample in last sample buffer
	for (int ch = 0; ch < mWidth; ch++) {
		mPeakMin[ch] = mLastSamples[0][ch];
		mPeakMax[ch] = mLastSamples[0][ch];
		mPeakMinIdx[ch] = 0;
		mPeakMaxIdx[ch] = 0;
	}

	// Search for min/max sample value for each channel in sample buffer
	for (int i = 1; i < mLength; i++) {
		for (int ch = 0; ch < mWidth; ch++) {
			if (mPeakMin[ch] > mLastSamples[i][ch]) {
				mPeakMin[ch] = mLastSamples[i][ch];
				mPeakMinIdx[ch] = i;
			}
			if (mPeakMax[ch] < mLastSamples[i][ch]) {
				mPeakMax[ch] = mLastSamples[i][ch];
				mPeakMaxIdx[ch] = i;
			}
		}
	}

#if 0 // Version 3.x
		if (!checkPeakGradient(ch))
			return false; // Gradient not within limits
#else // Version 4.x
		if (!checkMinMaxDiffPeakLimits(ch))
			return false; // Check difference between min. and max. peak is above limits
#endif
	}


	return 1;
	//return checkCoherencyTemplate();
}
*/
// (kim version - above is the changed version )
bool NXCOR::checkWithinChannelPeakLimits(void)
{
	// Set min/max values to first sample in last sample buffer
	for (int ch = 0; ch < mWidth; ch++) {
		mPeakMin[ch] = mLastSamples[0][ch];
		mPeakMax[ch] = mLastSamples[0][ch];
		mPeakMinIdx[ch] = 0;
		mPeakMaxIdx[ch] = 0;
	}

	// Search for min/max sample value for each channel in sample buffer
	for (int i = 1; i < mLength; i++) {
		for (int ch = 0; ch < mWidth; ch++) {
			if (mPeakMin[ch] > mLastSamples[i][ch]) {
				mPeakMin[ch] = mLastSamples[i][ch];
				mPeakMinIdx[ch] = i;
			}
			if (mPeakMax[ch] < mLastSamples[i][ch]) {
				mPeakMax[ch] = mLastSamples[i][ch];
				mPeakMaxIdx[ch] = i;
			}
		}
	}

	// Check that found minimum sample is within valid limits
	for (int ch = 0; ch < mWidth; ch++) {
		if (mPeakMin[ch] > mPeakMaxLimits[ch] ||
			mPeakMin[ch] < mPeakMinLimits[ch]) {
			//printf("Peak %d out limits ch %d\r\n", mPeakMin[ch], ch);
			return false; // Peak not within limits
		}
#if 0 // Version 3.x
		if (!checkPeakGradient(ch))
			return false; // Gradient not within limits
#else // Version 4.x
		if (!checkMinMaxDiffPeakLimits(ch))
			return false; // Check difference between min. and max. peak is above limits
#endif
	}

	// Version 4.x
	return checkCoherency();
	//return checkCoherencyTemplate();
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

void NXCOR::setMinMaxPeakLimits(TTYPE *limits)
{
	// Set peak limits for difference of min/max for each channel
	for (int i = 0; i < TEMP_WIDTH; i++)
		mPeakMinMaxLimits[i] = limits[i];
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
