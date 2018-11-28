/*
 * Timer.cpp
 *
 *  Created on: 16. aug. 2017
 *      Author: Kim Bjerge
 */


#include "Timer.h"

#define TIMER_FREQ_HZ (XPAR_CPU_CORTEXA9_0_CPU_CLK_FREQ_HZ / 2)

int Timer::Init(void)
{
	int Status;
	XScuTimer_Config *ConfigTmrPtr;

	/* Initialize the Scu Private Timer driver. */
	ConfigTmrPtr = XScuTimer_LookupConfig(mDeviceId);

	/* This is where the virtual address would be used, this uses physical address. */
	Status = XScuTimer_CfgInitialize(&mTimerInstance, ConfigTmrPtr,
			ConfigTmrPtr->BaseAddr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/* Disable Auto reload mode and set prescaler to 1 */
	XScuTimer_SetPrescaler(&mTimerInstance, 0);

	return Status;
}

void Timer::Delay(int uSDelay)
{
	u32 timerCnt;

	timerCnt = (TIMER_FREQ_HZ / 1000000) * uSDelay;

	XScuTimer_Stop(&mTimerInstance);
	XScuTimer_DisableAutoReload(&mTimerInstance);
	XScuTimer_LoadTimer(&mTimerInstance, timerCnt);
	XScuTimer_Start(&mTimerInstance);
	while (XScuTimer_GetCounterValue(&mTimerInstance))
	{};

}
