/*
 * NXCOR_HLS.cpp
 *
 *  Created on: 24. dec. 2018
 *      Author: Kim Bjerge
 */

#include "NXCOR_HLS.h"
#include <math.h>

void NXCOR::updateTemplate(int *temp)
{
	while (XNxcor_IsReady(&mNXCOR) == 0); // Polling ready register
	XNxcor_Write_templateData_Words(&mNXCOR, 0, temp, mLength*mWidth);
	XNxcor_Start(&mNXCOR);
	while (XNxcor_IsDone(&mNXCOR) == 0); // Polling done register
}

void NXCOR::startNXCOR(int *samples)
{
	// Clear done flags
	mResultAvailHlsNXCOR = 0;
	while (XNxcor_IsReady(&mNXCOR) == 0); // Polling ready register
	XNxcor_Write_signalData_Words(&mNXCOR, 0, samples, mWidth);
	XNxcor_Start(&mNXCOR);
}

float NXCOR::readResultNXCOR(void)
{
	u64 u64Result, u64Variance;
	float result;

	//if (mpIrq != 0)	// Busy wait for fir irq
	//	while(!mResultAvailHlsNXCOR);
	//else
		while (XNxcor_IsDone(&mNXCOR) == 0); // Polling done register

	u64Result = XNxcor_Get_result(&mNXCOR);
	u64Variance = XNxcor_Get_varSig(&mNXCOR);
	mResult = u64Result;
	mVarianceSignal = u64Variance;
	result = mResult / sqrt(mVarianceSignal * mVarianceTemplate);
    return result;
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
