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

#define STYPE      	  int16_t // Sample type used by HLS IP Core - Version 1.0 uses int32_t
#define SNUM          2       // Number of samples in int32_t

class FirFilter
{
public:
	FirFilter(int deviceId, int numChannels, int numTaps) :
		mDeviceId(deviceId), mNumChannels(numChannels),
		mNumTaps(numTaps), mResultAvailHlsFir(0) {}

	int Init(IRQ* pIrq = 0, int mIrqDeviceId = 0);

	void Disable(void);
	void Enable(void);

	void updateCoefficients(int *coefficients, int channel);
	void startFilter(STYPE *samples);
	unsigned long readFiltered(STYPE *samples);
	void executeFilter(STYPE *samples);

private:
	static void hls_fir_isr(void *InstancePtr);

	int mDeviceId;
	int mNumChannels;
	int mNumTaps;
	volatile int mResultAvailHlsFir;

	// HLS FIR HW instance
	XFirfilter mFirfilter;
	IRQ  *mpIrq;
};

#endif /* SRC_FIRFILTER_HLS_H_ */
