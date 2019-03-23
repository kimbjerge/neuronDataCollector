/*
 * NXCOR_HLS.h
 *
 *  Created on: 24. dec. 2018
 *      Author: Kim Bjerge
 */

#ifndef SRC_NXCOR_HLS_H_
#define SRC_NXCOR_HLS_H_

#include "xnxcor.h"
#include "IRQ.h"
#include "Template.h"

class NXCOR
{
public:
	NXCOR(int deviceId, int length, int width) :
		mDeviceId(deviceId), mLength(length), mWidth(width),
		mNXCORThreshold(0.7), // Default threshold
		mMaxPeakThreshold(32767), mMinPeakThreshold(-32768), // Peak limit disabled
		mMaxActivationCount(30), // Filter default 1 ms at fs = 30 kHz
		mResultAvailHlsNXCOR(0), mVarianceTemplate(1),
		mIdxPeak(0), mPeakSample(0), mActivationCounts(0),
		mCounts(0), mActiveState(0), mLastIdx(0),
		mPeakMinOffset(4), mPeakMinGradient(0)
	{ mPeakSamples = new int(length); }

	~NXCOR() { delete mPeakSamples; }

	int Init(IRQ* pIrq = 0, int mIrqDeviceId = 0);

	void Disable(void);
	void Enable(void);

	void updateTemplate(TTYPE *temp, int avgTemp);

	void setSize(int length, int width) { mLength = length; mWidth = width; }
	void setNXCORThreshold(float threshold) { mNXCORThreshold = threshold; }
	void setMaxPeakLimits(TTYPE *max); // Set max peak limits for each channel
	void setMinPeakLimits(TTYPE *min); // Set min peak limits for each channel
	void setChannelMap(short *map); // Set channel map of index to neuron channels where template is located
	void setPeakThreshold(int min, int max) { mMinPeakThreshold = min; mMaxPeakThreshold = max; }
	void setMaxActivationCount(int max) { mMaxActivationCount = max; }
	void setMinGradient(int min) { mPeakMinGradient = min; }
	float getNXCORResult(void) { return mResultNXCOR; }
	int getNumActivations(void) { return mCounts; }
	int getMaxPeak(void) { return mPeakSample; }
	int getActiveState(void) { return mActiveState; }
	void printSettings(char *buf = 0);

	void startNXCOR(TTYPE *samples); // Start NXCOR asynchronous
	float readResultNXCOR(float varTemplate); // Wait for NXCOR to complete and return result
	float executeNXCOR(TTYPE *samples, float varTemplate); // Start NXCOR and wait for completing
	int verifyActivation(void); // 0=no activation, 1=neuron activation detected, 2+3=filter active after detection
	void reset(void) { mActivationCounts = 0; mCounts = 0; mActiveState = 0; mLastIdx = 0; }

private:
	static void hls_NXCOR_isr(void *InstancePtr);

	// Parameters dependent on HW IP Core
	int mDeviceId;
	int mLength;
	int mWidth;

	// Template parameters dependent on training
	float mNXCORThreshold;
	int mMaxPeakThreshold;
	int mMinPeakThreshold;
	int mMaxActivationCount;

	volatile int mResultAvailHlsNXCOR;

	// Variables updated by readResultNXCOR
	double mVarianceTemplate;
	double mResult;
	double mVarianceSignal;
	float mResultNXCOR;

	// Variable to hold channel map
	short mChannelMap[TEMP_WIDTH]; // Contains index to neuron channel used by NXCOR

	bool checkWithinPeakLimits(void);
	void updatePeakSamples(TTYPE *pSamples);
	// Holds index to array of peak samples
	// using absolute value over channel width
	int mIdxPeak;
	int *mPeakSamples;
	int mPeakSample;

	// Decrements mMaxActivationCount the number of samples
	// since neuron activation detected
	int mActivationCounts;
	int mCounts; // Sum of all neuron activations
	int mActiveState; // Current active state

	bool checkPeakGradient(int ch);
	bool checkWithinChannelPeakLimits(void);
	void updateLastSamples(TTYPE *pSamples);
	// Array to hold copy of samples used to perform NXCOR
	// Used to calculate minimum peaks within limits
	TTYPE mLastSamples[TEMP_LENGTH][TEMP_WIDTH];
	int mLastIdx;
	int mPeakIdx[TEMP_WIDTH];
	TTYPE mPeakMin[TEMP_WIDTH];
	TTYPE mPeakMaxLimits[TEMP_WIDTH];
	TTYPE mPeakMinLimits[TEMP_WIDTH];
	int mPeakMinOffset; // Offset to minimum value where gradient is measured
	int mPeakMinGradient; // Minimum gradient value to offset above

	// HLS FIR HW instance
	XNxcor mNXCOR;
	IRQ  *mpIrq;
};



#endif /* SRC_NXCOR_HLS_H_ */
