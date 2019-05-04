/*
 * IIRHLS.h
 *
 *  Created on: 4. maj. 2019
 *      Author: Kim Bjerge
 */

#ifndef SRC_IIRFILTER_HLS_H_
#define SRC_IIRFILTER_HLS_H_

#include "xiirfilter.h"
#include "IRQ.h"

#define STYPE      	  int16_t // Sample type used by HLS IP Core - Version 1.0 uses int32_t
#define SNUM          2       // Number of samples in int32_t

class IIRFilter
{
public:
	IIRFilter(int deviceId, int numChannels, int numTaps) :
		mDeviceId(deviceId), mNumChannels(numChannels),
		mNumTaps(numTaps), mResultAvailHlsIIR(0) {}

	int Init(IRQ* pIrq = 0, int mIrqDeviceId = 0);

	void Disable(void);
	void Enable(void);

	void updateCoefficients(int *coefficients, int channel);
	void startFilter(STYPE *samples);
	unsigned long readFiltered(STYPE *samples);
	void executeFilter(STYPE *samples);

private:
	static void hls_iir_isr(void *InstancePtr);

	int mDeviceId;
	int mNumChannels;
	int mNumTaps;
	volatile int mResultAvailHlsIIR;

	// HLS IIR HW instance
	XIirfilter mIIRfilter;
	IRQ  *mpIrq;
};

#endif /* SRC_IIRFILTER_HLS_H_ */
