/*
 * IIRHLS.cpp
 *
 *  Created on: 4. maj. 2019
 *      Author: Kim Bjerge
 */

#include "IIRFilter_HLS.h"

void IIRFilter::updateCoefficients(int *coefficients, int channel)
{
	while (XIirfilter_IsReady(&mIIRfilter) == 0); // Polling ready register
	XIirfilter_Set_operation(&mIIRfilter, channel); // Set operation to update coefficients
	XIirfilter_Write_coeff_Words(&mIIRfilter, 0, coefficients, mNumTaps);
	XIirfilter_Start(&mIIRfilter);
	while (XIirfilter_IsDone(&mIIRfilter) == 0); // Polling done register
	XIirfilter_Set_operation(&mIIRfilter, mNumChannels); // Set operation back to filtering
}

void IIRFilter::startFilter(STYPE *samples)
{
	// Clear done flags
	mResultAvailHlsIIR = 0;

	// send samples after shifting least significant 8 bits as the filter
	// requires 16 bit input sample
	//for (int i=0; i<mNumChannels; i++) // Convert samples from short to int
	//	mSamples[i] = ((int)samples[i]) << SHIFT_BITS; // Shift bits to improve accuracy

	while (XIirfilter_IsReady(&mIIRfilter) == 0); // Polling ready register
	XIirfilter_Write_samples_Words(&mIIRfilter, 0, (int *)samples, mNumChannels/SNUM);
	XIirfilter_Start(&mIIRfilter);
}

unsigned long IIRFilter::readFiltered(STYPE *results)
{
	unsigned long counts;
	//if (mpIrq != 0)	// Busy wait for fir irq
	//	while(!mResultAvailHlsFir);
	//else
	while (XIirfilter_IsDone(&mIIRfilter) == 0); // Polling done register
	counts =  XIirfilter_Read_results_Words(&mIIRfilter, 0, (int *)results, mNumChannels/SNUM);
	//for (int i=0; i<mNumChannels; i++) // Convert samples from int to short
	//	results[i] = (mSamples[i] + (1<<(SHIFT_BITS-1))) >> SHIFT_BITS; // Round and shift bits back
	return counts;
}

void IIRFilter::executeFilter(STYPE *samples)
{
	startFilter(samples);
	while (XIirfilter_IsDone(&mIIRfilter) == 0); // Polling done register
}

void IIRFilter::Disable(void)
{
	// Disable Global and instance interrupts
	XIirfilter_InterruptDisable(&mIIRfilter, 1);
	XIirfilter_InterruptGlobalDisable(&mIIRfilter);
	// clear the local interrupt
	XIirfilter_InterruptClear(&mIIRfilter, 1);
}

void IIRFilter::Enable(void)
{
	// Enable Global and instance interrupts
	mResultAvailHlsIIR = 0;
	XIirfilter_InterruptEnable(&mIIRfilter,1);
	XIirfilter_InterruptGlobalEnable(&mIIRfilter);
}

void IIRFilter::hls_iir_isr(void *InstancePtr)
{
	IIRFilter *pHlsFir = (IIRFilter *)InstancePtr;
	XIirfilter *pAccelerator = &(pHlsFir->mIIRfilter);

	// clear the local interrupt
	XIirfilter_InterruptClear(pAccelerator, 1);

	pHlsFir->mResultAvailHlsIIR = 1;
}

int IIRFilter::Init(IRQ* pIrq, int mIrqDeviceId)
{
	XIirfilter_Config *cfgPtr;
	int status;

	// Set IRQ pointer
	mpIrq = pIrq;

	cfgPtr = XIirfilter_LookupConfig(mDeviceId);
	if (!cfgPtr) {
	  print("ERROR: Lookup of FIR configuration failed.\n\r");
	  return XST_FAILURE;
	}

	status = XIirfilter_CfgInitialize(&mIIRfilter, cfgPtr);
	if (status != XST_SUCCESS) {
	  print("ERROR: Could not initialize FIR.\n\r");
	  return XST_FAILURE;
	}

	// Disable and clear interrupts from FIR
	Disable();

	if (mpIrq != 0) { // Enable interrupt if interrupt controller specified
		// Connect the Left FIR ISR to the exception table
		status = XScuGic_Connect(mpIrq->getScuGic(), mIrqDeviceId,
								 (Xil_InterruptHandler)hls_iir_isr, this);
		if(status != XST_SUCCESS){
		   return status;
		}

		// Enable the FIR ISR
		XScuGic_Enable(mpIrq->getScuGic(), mIrqDeviceId);
	}

	return status;
}
