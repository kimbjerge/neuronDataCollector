/*
 * FirHLS.h
 *
 *  Created on: 16. aug. 2017
 *      Author: Kim Bjerge
 */

#ifndef SRC_FIRFILTER_HLS_H_
#define SRC_FIRFILTER_HLS_H_

#include "xfirfilter.h"
#include "IRQ.h"


class FirFilter
{
public:
	FirFilter(int deviceId, int numSamples, int numTaps) :
		mDeviceId(deviceId), mNumSamples(numSamples),
		mNumTaps(numTaps), mResultAvailHlsFir(0) {}

	int Init(IRQ* pIrq = 0, int mIrqDeviceId = 0);

	void Disable(void);
	void Enable(void);

	void updateCoefficients(int *coefficients);
	void startFilter(int *samples);
	unsigned long readFiltered(int *samples);
	void executeFilter(int *samples);

private:
	static void hls_fir_isr(void *InstancePtr);

	int mDeviceId;
	int mNumSamples;
	int mNumTaps;
	volatile int mResultAvailHlsFir;

	// HLS FIR HW instance
	XFirfilter mFirfilter;
	IRQ  *mpIrq;
};

#endif /* SRC_FIRFILTER_HLS_H_ */
