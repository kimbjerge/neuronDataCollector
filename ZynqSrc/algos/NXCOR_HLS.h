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

class NXCOR
{
public:
	NXCOR(int deviceId, int length, int width) :
		mDeviceId(deviceId), mResultAvailHlsNXCOR(0),
		mLength(length), mWidth(width), mVarianceTemplate(1) { }

	int Init(IRQ* pIrq = 0, int mIrqDeviceId = 0);

	void Disable(void);
	void Enable(void);

	void updateTemplate(int *temp, int avgTemp);
	void startNXCOR(int *samples); // Start NXCOR asynchronous
	float readResultNXCOR(float varTemplate); // Wait for NXCOR to complete and return result
	float executeNXCOR(int *samples, float varTemplate); // Start NXCOR and wait for completing

private:
	static void hls_NXCOR_isr(void *InstancePtr);

	int mDeviceId;
	volatile int mResultAvailHlsNXCOR;
	int mLength;
	int mWidth;
	float mVarianceTemplate;
	float mResult;
	float mVarianceSignal;

	// HLS FIR HW instance
	XNxcor mNXCOR;
	IRQ  *mpIrq;
};



#endif /* SRC_NXCOR_HLS_H_ */
