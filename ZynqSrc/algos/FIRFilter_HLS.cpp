/*
 * FirHLS.cpp
 *
 *  Created on: 16. aug. 2017
 *      Author: Kim Bjerge
 */

#include "FIRFilter_HLS.h"

void FirFilter::updateCoefficients(int *coefficients, int channel)
{
	while (XFirfilter_IsReady(&mFirfilter) == 0); // Polling ready register
	XFirfilter_Set_operation(&mFirfilter, channel); // Set operation to update coefficients
	XFirfilter_Write_coeff_Words(&mFirfilter, 0, coefficients, mNumTaps);
	XFirfilter_Start(&mFirfilter);
	while (XFirfilter_IsDone(&mFirfilter) == 0); // Polling done register
	XFirfilter_Set_operation(&mFirfilter, mNumChannels); // Set operation back to filtering
}

void FirFilter::startFilter(STYPE *samples)
{
	// Clear done flags
	mResultAvailHlsFir = 0;
	// send samples after shifting least significant 8 bits as the filter
	// requires 16 bit input sample
	while (XFirfilter_IsReady(&mFirfilter) == 0); // Polling ready register
	XFirfilter_Write_samples_Words(&mFirfilter, 0, (int *)samples, mNumChannels/SNUM);
	XFirfilter_Start(&mFirfilter);
}

unsigned long FirFilter::readFiltered(STYPE *results)
{
	//if (mpIrq != 0)	// Busy wait for fir irq
	//	while(!mResultAvailHlsFir);
	//else
	while (XFirfilter_IsDone(&mFirfilter) == 0); // Polling done register
	return XFirfilter_Read_results_Words(&mFirfilter, 0, (int *)results, mNumChannels/SNUM);
}

void FirFilter::executeFilter(STYPE *samples)
{
	startFilter(samples);
	while (XFirfilter_IsDone(&mFirfilter) == 0); // Polling done register
}

void FirFilter::Disable(void)
{
	// Disable Global and instance interrupts
	XFirfilter_InterruptDisable(&mFirfilter, 1);
	XFirfilter_InterruptGlobalDisable(&mFirfilter);
	// clear the local interrupt
	XFirfilter_InterruptClear(&mFirfilter, 1);
}

void FirFilter::Enable(void)
{
	// Enable Global and instance interrupts
	mResultAvailHlsFir = 0;
	XFirfilter_InterruptEnable(&mFirfilter,1);
	XFirfilter_InterruptGlobalEnable(&mFirfilter);
}

void FirFilter::hls_fir_isr(void *InstancePtr)
{
	FirFilter *pHlsFir = (FirFilter *)InstancePtr;
	XFirfilter *pAccelerator = &(pHlsFir->mFirfilter);

	// clear the local interrupt
	XFirfilter_InterruptClear(pAccelerator, 1);

	pHlsFir->mResultAvailHlsFir = 1;
}

int FirFilter::Init(IRQ* pIrq, int mIrqDeviceId)
{
	XFirfilter_Config *cfgPtr;
	int status;

	// Set IRQ pointer
	mpIrq = pIrq;

	cfgPtr = XFirfilter_LookupConfig(mDeviceId);
	if (!cfgPtr) {
	  print("ERROR: Lookup of FIR configuration failed.\n\r");
	  return XST_FAILURE;
	}

	status = XFirfilter_CfgInitialize(&mFirfilter, cfgPtr);
	if (status != XST_SUCCESS) {
	  print("ERROR: Could not initialize FIR.\n\r");
	  return XST_FAILURE;
	}

	// Disable and clear interrupts from FIR
	Disable();

	if (mpIrq != 0) { // Enable interrupt if interrupt controller specified
		// Connect the Left FIR ISR to the exception table
		status = XScuGic_Connect(mpIrq->getScuGic(), mIrqDeviceId,
								 (Xil_InterruptHandler)hls_fir_isr, this);
		if(status != XST_SUCCESS){
		   return status;
		}

		// Enable the FIR ISR
		XScuGic_Enable(mpIrq->getScuGic(), mIrqDeviceId);
	}

	return status;
}
