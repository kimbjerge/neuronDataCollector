#include "hpp_iic_master.h"



volatile u32 SendComplete;
volatile u32 RecvComplete;
volatile u32 TotalErrorCount;

extern XScuGic xInterruptController;

static int SetupInterruptSystem(XIicPs *IicPsPtr);

XIicPs_Config *Config;

XIicPs Iic;

s8 vSetupHPPIIC(u16 DeviceId)
{
	int Status;
	XIicPs_Config *Config;

	Config = XIicPs_LookupConfig(DeviceId);
	if (NULL == Config)
	{
		return XST_FAILURE;
	}

	Status = XIicPs_CfgInitialize(&Iic, Config, Config->BaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	Status = SetupInterruptSystem(&Iic);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XIicPs_SetStatusHandler(&Iic, (void *) &Iic, Handler);


	// Perform a self-test.
	Status = XIicPs_SelfTest(&Iic);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	XIicPs_SetSClk(&Iic, IIC_SCLK_RATE);

	//XIicPs_Reset(&Iic);


	return XST_SUCCESS;
}

s8 SendHeadstageStimCommand(char *SendBuffer, char *RecvBuffer)
{
	u32 bufferlen = strlen(SendBuffer);

	while (XIicPs_BusIsBusy(&Iic))
	{
		;
	}

	SendComplete = FALSE;


	XIicPs_MasterSend(&Iic, (u8 *)SendBuffer, bufferlen, IIC_SLAVE_ADDR);

	while (!SendComplete)
	{
		if (0 != TotalErrorCount)
		{
			return XST_FAILURE;
		}
	}


	while (XIicPs_BusIsBusy(&Iic))
	{
		;
	}


	RecvComplete = FALSE;
	XIicPs_MasterRecv(&Iic, (u8 *)RecvBuffer, bufferlen, IIC_SLAVE_ADDR);

	while (!RecvComplete)
	{
		if (0 != TotalErrorCount)
		{
			return XST_FAILURE;
		}
	}


	return XST_SUCCESS;
}




void Handler(void *CallBackRef, u32 Event)
{
	(void) CallBackRef;

	if (0 != (Event & XIICPS_EVENT_COMPLETE_RECV)){
		RecvComplete = TRUE;
	} else if (0 != (Event & XIICPS_EVENT_COMPLETE_SEND)) {
		SendComplete = TRUE;
	} else if (0 == (Event & XIICPS_EVENT_SLAVE_RDY)){
		/*
		 * If it is other interrupt but not slave ready interrupt, it is
		 * an error.
		 * Data was received with an error.
		 */
		TotalErrorCount++;
	}
}


static int SetupInterruptSystem(XIicPs *IicPsPtr)
{
	int Status;
	XScuGic_Config *IntcConfig; /* Instance of the interrupt controller */

	Xil_ExceptionInit();

	/*
	 * Initialize the interrupt controller driver so that it is ready to
	 * use.
	 */
	IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);
	if (NULL == IntcConfig) {
		return XST_FAILURE;
	}

	Status = XScuGic_CfgInitialize(&xInterruptController, IntcConfig,
					IntcConfig->CpuBaseAddress);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	/*
	 * Connect the interrupt controller interrupt handler to the hardware
	 * interrupt handling logic in the processor.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IRQ_INT,
				(Xil_ExceptionHandler)XScuGic_InterruptHandler,
				&xInterruptController);

	/*
	 * Connect the device driver handler that will be called when an
	 * interrupt for the device occurs, the handler defined above performs
	 * the specific interrupt processing for the device.
	 */
	Status = XScuGic_Connect(&xInterruptController, IIC_INT_VEC_ID,
			(Xil_InterruptHandler)XIicPs_MasterInterruptHandler,
			(void *)IicPsPtr);
	if (Status != XST_SUCCESS) {
		return Status;
	}

	/*
	 * Enable the interrupt for the Iic device.
	 */
	XScuGic_Enable(&xInterruptController, IIC_INT_VEC_ID);


	/*
	 * Enable interrupts in the Processor.
	 */
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}

