/*
 * hpp.c
 *
 *  Created on: Feb 18, 2014
 *      Author: stevem
 */

#include "hpp.h"
#include "Nlx_Demos.h"
#include <stdio.h>
#include "platform.h"
//#include "xtmrctr.h"
#include "xscugic.h"
#include "xparameters.h"
#include "xstatus.h"
#include "xgpiops.h"
#include "freertos.h"
#include "semphr.h"
#include "task.h"
#include "portmacro.h"
#include "xil_cache.h"
#include "sleep.h"
#include "xil_mmu.h"
#include "file_loader.h"
#include <stdlib.h>
#include <stdarg.h>
#include "pcap.h"


const struct HPP_Data_Type* HPP_Data = (const struct HPP_Data_Type *)(HPP_DATA_ADDR);
//Overload Ack Address for either type of response
struct HPP_Ascii_Ack_Type* HPP_Ascii_Ack = (struct HPP_Ascii_Ack_Type *)(CHEETAH_ACK_ADDR);
const struct HPP_Logic_Ack_Type* HPP_Logic_Ack = (const struct HPP_Logic_Ack_Type *)(CHEETAH_ACK_ADDR);

const struct HPP_File_Command* HPP_File_Cmd = (const struct HPP_File_Command *)(FILE_BASE_ADDR);
const struct HPP_File_DataSet* HPP_File_Data = (const struct HPP_File_DataSet *)(FILE_TRANSFER_ADDR);
const struct HPP_File_End* HPP_File_Finished = (const struct HPP_File_End *)(FILE_BASE_ADDR);
const struct HPP_File_Exe* HPP_File_Execute = (const struct HPP_File_Exe *)(FILE_BASE_ADDR);

const unsigned int* num_data_interrupts = (const unsigned int *)(PL_INTERRUPTS_ADDR);


s32 tm_high[32];
s32 tm_low[32];
u32 num_data_buffers_loaded = 0;
u32 data_channels_loaded = 0;
u32 most_matches = 0;
u32 active_buffer_index = 0;
enum LED_Status led_s1_value = LED_Off;
enum LED_Status led_s2_value = LED_Off;
u32 gpio_bank2 = 0;
u32 gpio_bank3 = 0;
u32 logic_status = 0;

u32 dir = 0;

u32 num_file_interrupts = 0;

XGpioPs xGpiops;
SemaphoreHandle_t xHPP_Data_Sem = NULL;
SemaphoreHandle_t xHPP_Response_Sem = NULL;
SemaphoreHandle_t xHPP_File_Sem = NULL;

SemaphoreHandle_t xPulse_Train_Sem = NULL;
static signed portBASE_TYPE xHigherPriorityTaskWoken;
u32 hpp_index;

//static portBASE_TYPE xStatus;

struct HPP_Cheetah_Command HPP_Cmd;
struct HPP_Audio_Filter HPP_AF;
char temp_msg_str[1256];
u32 temp_msg_str_len = 0;
char cheetah_msg_str[1256];
u32 cheetah_msg_str_len = 0;

struct HPP_Analog_Data	AnalogData;

extern SemaphoreHandle_t xHPP_Burst_Analysis_Sem;
extern SemaphoreHandle_t xHPP_Neural_Ensemble_Sem;
extern SemaphoreHandle_t xHPP_Spike_Detect_Sem;

void vSetupHPP(void)
{
	BaseType_t xReturn;
	s8 status = 0;

	status = vSetupHPPIIC(XPAR_PS7_I2C_0_DEVICE_ID);
	if(status != XST_SUCCESS)
	{
		xil_printf("\n\rProblem with vSetupHPPIIC...\n\r");
	}

    vSemaphoreCreateBinary(xHPP_Data_Sem);
	if(xSemaphoreTake( xHPP_Data_Sem, portMAX_DELAY ) != pdTRUE)
	{
		xil_printf("\n\rProblem with initial xSemaphoreTake of xHPP_Data_Sem\n\r");
	}
    if(xHPP_Data_Sem == NULL)
    {
    	xil_printf("Problem creating xHPP_Data_Sem.\n\r");
    }

    vSemaphoreCreateBinary(xHPP_Response_Sem);
	if(xSemaphoreTake( xHPP_Response_Sem, portMAX_DELAY ) != pdTRUE)
	{
		xil_printf("\n\rProblem with initial xSemaphoreTake of xHPP_Response_Sem\n\r");
	}
    if(xHPP_Response_Sem == NULL)
    {
    	xil_printf("Problem creating xHPP_Response_Sem.\n\r");
    }

    vSemaphoreCreateBinary(xHPP_File_Sem);
	if(xSemaphoreTake( xHPP_File_Sem, portMAX_DELAY ) != pdTRUE)
	{
		xil_printf("\n\rProblem with initial xSemaphoreTake of xHPP_File_Sem\n\r");
	}
    if(xHPP_File_Sem == NULL)
    {
    	xil_printf("Problem creating xHPP_File_Sem.\n\r");
    }

	//Create and Take Demo Semaphores
    vSemaphoreCreateBinary(xHPP_Burst_Analysis_Sem);
	if(xSemaphoreTake(xHPP_Burst_Analysis_Sem, portMAX_DELAY) != pdTRUE)
	{
		xil_printf("\n\rProblem with initial xSemaphoreTake of xHPP_Burst_Analysis_Sem\n\r");
	}
    if(xHPP_Burst_Analysis_Sem == NULL)
    {
    	xil_printf("Problem creating xHPP_Burst_Analysis_Sem.\n\r");
    }

    vSemaphoreCreateBinary(xHPP_Neural_Ensemble_Sem);
	if(xSemaphoreTake(xHPP_Neural_Ensemble_Sem, portMAX_DELAY) != pdTRUE)
	{
		xil_printf("\n\rProblem with initial xSemaphoreTake of xHPP_Neural_Ensemble_Sem\n\r");
	}
    if(xHPP_Neural_Ensemble_Sem == NULL)
    {
    	xil_printf("Problem creating xHPP_Neural_Ensemble_Sem.\n\r");
    }

    vSemaphoreCreateBinary(xHPP_Spike_Detect_Sem);
	if(xSemaphoreTake(xHPP_Spike_Detect_Sem, portMAX_DELAY) != pdTRUE)
	{
		xil_printf("\n\rProblem with initial xSemaphoreTake of xHPP_Spike_Detect_Sem\n\r");
	}
    if(xHPP_Spike_Detect_Sem == NULL)
    {
    	xil_printf("Problem creating xHPP_Spike_Detect_Sem.\n\r");
    }

	xil_printf("\n\rSemaphores created and taken\n\r");

	HPP_Flash_Init();

	//Wait for Serdes links to be trained...
	xil_printf("\n\rWaiting for DLSX synchronization");

	vSetup_HPP_GPIO();


	xReturn = xTaskCreate(vFile_System_Task, "vFile_System_Task", (uint16_t)65535, NULL, 5, NULL);
	xil_printf("vFile_System_Task created with return value of %d\n\r", xReturn);
}


void vSetup_HPP_GPIO()
{
	XGpioPs_SetDirection(&xGpiops, GPIO_BANK_2, DIR_INPUT);

	do
	{
		logic_status = XGpioPs_Read(&xGpiops, GPIO_BANK_2);
		logic_status = logic_status & 0x00000007;
		xil_printf(".");
		usleep(250000);
#ifdef ZEDBOARD_DEBUG // Inserted for debugging on ZedBoard
	}while(logic_status != 0);
#else
	}while(logic_status != 7);
#endif

	xil_printf("\n\r");

	XGpioPs_SetDirection(&xGpiops, GPIO_BANK_2, DIR_OUTPUT);

	ClearPSGPIO();

	//Assert PS control over messages to DLSX at start of day
	SetPSControl(PS_Ctrl);
}


void vInit_HPPData(void)
{
	u32 *data = (u32 *)(FILE_BASE_ADDR);
	u32 index = 0;
	u32 data_size = HPP_DATA_ADDR_HIGH - FILE_BASE_ADDR - 1;
	u32 data_count = data_size >> 20; //Number of iterations for loop to set memory to not cached
	u32 address = FILE_BASE_ADDR;


	for(index = 0; index < (data_size >> 2); index++)
	{
		data[index] = 0;
	}

	xil_printf("\n\rCleared File and Data DDR locations.\n\r");


	//http://forums.xilinx.com/xlnx/board/crawl_message?board.id=EMBEDDED&message.id=8268
	//http://forums.xilinx.com/xlnx/board/crawl_message?board.id=EMBEDDED&message.id=8659
	for(index = 0; index < data_count + 1; index++)
	{
		address = FILE_BASE_ADDR + (index << 20);
		Xil_SetTlbAttributes(address, 0x04de2); //Set memory used for HPP_Data, acks, and file transfers to non-cacheable
	}

//	data_size = HPP_DATA_ADDR_HIGH - HPP_DATA_ADDR;
//	data_count = data_size >> 20;
//	address = HPP_DATA_ADDR;
//	data = (u32 *)(HPP_DATA_ADDR);
//
//	for(index = 0; index < (data_size >> 2); index++)
//	{
//		data[index] = 0;
//	}
//
//	xil_printf("\n\rCleared HPP_DATA DDR locations.\n\r");
//
//
//	//http://forums.xilinx.com/xlnx/board/crawl_message?board.id=EMBEDDED&message.id=8268
//	//http://forums.xilinx.com/xlnx/board/crawl_message?board.id=EMBEDDED&message.id=8659
//	for(index = 0; index < data_count + 1; index++)
//	{
//		address = HPP_DATA_ADDR + (index << 20);
//		Xil_SetTlbAttributes(address, 0x04de2); //Set memory used for HPP_Data, acks, and file transfers to non-cacheable
//	}
}


void vVerifyCommandControl(void* pvNotUsed)
{
	s8 status = 0;
	u16 ind = 0;
	(void) pvNotUsed;

	//Assert PS control (as opposed to fabric control) over the commands sent to the motherboard
	status = SetPSControl(PS_Ctrl);
	if(status != 0)
	{
		xil_printf("\n\rProblem with SetPSControl(PS_Ctrl)");
	}
	else
	{
		xil_printf("\n\rSetPSControl(PS_Ctrl) successfully called.\n\r");
	}

	//Assume LED Control
	status = SetFPLEDCtrl(HPP_Ctrl);
	if(status != 0)
	{
		xil_printf("\n\rProblem with SetFPLEDCtrl(LED_S1), Status = %d", status);
	}


	while(1)
	{
		//usleep(500000);

		//Toggle each LED once for verification of functionality before template matching
		status = SetFPLED(LED_S1, LED_On);
		if(status != 0)
		{
			xil_printf("\n\rProblem with SetFPLED(LED_S1, LED_On), Status = %d", status);
		}

		//usleep(500000);

		status = SetFPLED(LED_S1, LED_Off);
		if(status != 0)
		{
			xil_printf("\n\rProblem with SetFPLED(LED_S1, LED_Off), Status = %d", status);
		}

		//usleep(500000);

		status = SetFPLED(LED_S2, LED_On);
		if(status != 0)
		{
			xil_printf("\n\rProblem with SetFPLED(LED_S2, LED_On), Status = %d", status);
		}

		//usleep(500000);

		status = SetFPLED(LED_S2, LED_Off);
		if(status != 0)
		{
			xil_printf("\n\rProblem with SetFPLED(LED_S2, LED_Off), Status = %d", status);
		}

	}


	for(ind = 0; ind < 4; ind++)
	{
		//Assert HPP control over MB TTL lines
		status = SetDIOCtrl(ind, HPP_Ctrl);
		if(status != 0)
		{
			xil_printf("\n\rProblem with SetDigitalIOCtrl(%d, HPP_Ctrl), Status = %d", ind, status);
		}

		//Set TTLs to outputs
		status = SetDIOPortDir(ind, DIO_PORT_DIR_OUTPUT);
		if(status != 0)
		{
			xil_printf("\n\rProblem with SetDigitalIOPortDir(%d, DIO_PORT_DIR_OUTPUT), Status = %d", ind, status);
		}
	}

	//Assume LED Control
	status = SetFPLEDCtrl(HPP_Ctrl);
	if(status != 0)
	{
		xil_printf("\n\rProblem with SetFPLEDCtrl(LED_S1), Status = %d", status);
	}

	//Toggle each LED once for verification of functionality before template matching
	status = SetFPLED(LED_S1, LED_On);
	if(status != 0)
	{
		xil_printf("\n\rProblem with SetFPLED(LED_S1, LED_On), Status = %d", status);
	}


	status = SetFPLED(LED_S1, LED_Off);
	if(status != 0)
	{
		xil_printf("\n\rProblem with SetFPLED(LED_S1, LED_Off), Status = %d", status);
	}


	status = SetFPLED(LED_S2, LED_On);
	if(status != 0)
	{
		xil_printf("\n\rProblem with SetFPLED(LED_S2, LED_On), Status = %d", status);
	}


	status = SetFPLED(LED_S2, LED_Off);
	if(status != 0)
	{
		xil_printf("\n\rProblem with SetFPLED(LED_S2, LED_Off), Status = %d", status);
	}

	xil_printf("\n\rFinished vVerifyCommandControl\n\r");

	vTaskDelete(NULL);

}


void vConfigureHPPDataInterrupt(void *pvNotUsed)
{
	BaseType_t xStatus;
	extern XScuGic xInterruptController;
	const uint8_t ucRisingEdge = 3;

	// Remove compile warnings if configASSERT() is not defined.
	(void) pvNotUsed;

	XScuGic_Disable(&xInterruptController, XPAR_FABRIC_SYSTEM_HPP_INT_INTR);

	XScuGic_SetPriorityTriggerType(&xInterruptController, XPAR_FABRIC_SYSTEM_HPP_INT_INTR, 0xa0, ucRisingEdge);

	// Connect to the interrupt controller.
	xStatus = XScuGic_Connect(&xInterruptController, XPAR_FABRIC_SYSTEM_HPP_INT_INTR, (Xil_InterruptHandler) vHPP_DataInterruptHandler, (void *) &xGpiops);
	configASSERT(xStatus == XST_SUCCESS);

	// Enable the interrupt in the GIC.
	XScuGic_Enable( &xInterruptController, XPAR_FABRIC_SYSTEM_HPP_INT_INTR );
}


void vConfigureHPPAckInterrupt(void *pvNotUsed)
{
	BaseType_t xStatus;
	extern XScuGic xInterruptController;
	const uint8_t ucRisingEdge = 3;

	// Remove compile warnings if configASSERT() is not defined.
	(void) pvNotUsed;

	XScuGic_Disable(&xInterruptController, XPAR_FABRIC_SYSTEM_GPIO_INT_INTR);

	XScuGic_SetPriorityTriggerType(&xInterruptController, XPAR_FABRIC_SYSTEM_GPIO_INT_INTR, 0xc0, ucRisingEdge);

	// Connect to the interrupt controller.
	xStatus = XScuGic_Connect(&xInterruptController, XPAR_FABRIC_SYSTEM_GPIO_INT_INTR, (Xil_InterruptHandler) vHPP_AckInterruptHandler, ( void * ) &xGpiops);
	configASSERT(xStatus == XST_SUCCESS);

	// Enable the interrupt in the GIC.
	XScuGic_Enable( &xInterruptController, XPAR_FABRIC_SYSTEM_GPIO_INT_INTR );
}


void vConfigureHPPFileInterrupt(void *pvNotUsed)
{
	BaseType_t xStatus;
	extern XScuGic xInterruptController;
	const uint8_t ucRisingEdge = 3;

	// Remove compile warnings if configASSERT() is not defined.
	(void) pvNotUsed;

	XScuGic_Disable(&xInterruptController, XPAR_FABRIC_SYSTEM_FILE_INT_INTR);

	XScuGic_SetPriorityTriggerType(&xInterruptController, XPAR_FABRIC_SYSTEM_FILE_INT_INTR, 0xb0, ucRisingEdge);

	// Connect to the interrupt controller.
	xStatus = XScuGic_Connect(&xInterruptController, XPAR_FABRIC_SYSTEM_FILE_INT_INTR, (Xil_InterruptHandler) vHPP_FileInterruptHandler, (void *) &xGpiops);
	configASSERT(xStatus == XST_SUCCESS);

	// Enable the interrupt in the GIC.
	XScuGic_Enable(&xInterruptController, XPAR_FABRIC_SYSTEM_FILE_INT_INTR);
}


void vHPP_DataInterruptHandler(void *pvNotUsed)
{
	// Remove compile warnings if configASSERT() is not defined.
	(void) pvNotUsed;

//STM 6Oct14
//The following commented out code is example code in case the PS needs to synchronize to the number of records the PL has DMAed.
//The problem with it is that the PS automatically assigns the GPIO input to the GPIO output when in input mode.  When using
//gpio_bank3 as the input for the num_interrupts input, this assigns that value to the gpio_bank3 outputs (which are also used for
//digital I/O messages/assignments to the DLSX).  Even with the correction code below to reset the gpio_bank3 outputs to their original
//value, the gpio_bank3 outputs are incorrect for around 60 100MHz clock cycles...

//	if(num_data_buffers_loaded == 0)
//	{
//		dir = XGpioPs_GetDirection(&xGpiops, GPIO_BANK_3);
//		XGpioPs_SetDirection(&xGpiops, GPIO_BANK_3, 0);
//		num_data_buffers_loaded = XGpioPs_Read(&xGpiops, GPIO_BANK_3);
//		XGpioPs_SetDirection(&xGpiops, GPIO_BANK_3, dir);
//		XGpioPs_Write(&xGpiops, GPIO_BANK_3, gpio_bank3);
//
//		gpio_bank2 |= 1 << ASCII_TRANSFER;
//		XGpioPs_Write(&xGpiops, GPIO_BANK_2, gpio_bank2);
//
//		gpio_bank2 ^= 1 << ASCII_TRANSFER;
//		XGpioPs_Write(&xGpiops, GPIO_BANK_2, gpio_bank2);
//	}
//	else
//	{
		//num_data_buffers_loaded++;
//	}

	num_data_buffers_loaded++;
	if(num_data_buffers_loaded != num_data_interrupts[0])
	{
		//For ILA debug purposes
		//SetPSReady(0);
		xil_printf("\n\rMismatch between num_data_buffers_loaded (%d) and num_data_interrupts[0] (%d)...\n\r", num_data_buffers_loaded, num_data_interrupts[0]);
		if(num_data_buffers_loaded < num_data_interrupts[0])
		{
			num_data_buffers_loaded = num_data_interrupts[0];
		}
	}

	data_channels_loaded = 1;

    xHigherPriorityTaskWoken = pdFALSE;

    //FastToggleDigIO(0);

	xSemaphoreGiveFromISR(xHPP_Data_Sem, &xHigherPriorityTaskWoken);
	//xSemaphoreGiveFromISR(xHPP_Burst_Analysis_Sem, &xHigherPriorityTaskWoken);
	//xSemaphoreGiveFromISR(xHPP_Neural_Ensemble_Sem, &xHigherPriorityTaskWoken);
	xSemaphoreGiveFromISR(xHPP_Spike_Detect_Sem, &xHigherPriorityTaskWoken);


    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}


void vHPP_AckInterruptHandler(void *pvNotUsed)
{

	gpio_bank2 |= 1 << CMD_MSG_ACK_BIT;
	XGpioPs_Write(&xGpiops, GPIO_BANK_2, gpio_bank2);

	gpio_bank2 ^= 1 << CMD_MSG_ACK_BIT;
	XGpioPs_Write(&xGpiops, GPIO_BANK_2, gpio_bank2);

	// Remove compile warnings if configASSERT() is not defined.
	( void ) pvNotUsed;

    xHigherPriorityTaskWoken = pdFALSE;

	xSemaphoreGiveFromISR(xHPP_Response_Sem, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}


void vHPP_FileInterruptHandler(void *pvNotUsed)
{
	// Remove compile warnings if configASSERT() is not defined.
	( void ) pvNotUsed;
	//char *tempstr = (char *)FILE_TRANSFER_ADDR;

	//xil_printf("\n\rCommandID = %d\n\r", HPP_File_Cmd[0].CommandID);

	//xil_printf("\n\rtempstr = %s\n\r", tempstr);

	num_file_interrupts++;

    xHigherPriorityTaskWoken = pdFALSE;

	xSemaphoreGiveFromISR(xHPP_File_Sem, &xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}





void vToggleLED(void* params)
{
	s8 status = 0;
	u32 *int_ptr = 0;
	u32 temp_int = 0;
	u8 led_num = 0;

	int_ptr = (u32 *)params;
	temp_int = 0x00000001 & (u32)(*int_ptr);
	led_num = (u8)(temp_int);

	if(led_num == 0)
	{
		led_s1_value = (LED_Status) (led_s1_value ^ LED_On);

		status = SetFPLED(LED_S1, led_s1_value);
		if(status != 0)
		{
			xil_printf("\n\rProblem with SetFPLED(LED_S1, %d), Status = %d", led_s1_value, status);
		}
	}
	else
	{
		led_s2_value = (LED_Status) (led_s1_value ^ LED_On);

		status = SetFPLED(LED_S2, led_s2_value);
		if(status != 0)
		{
			xil_printf("\n\rProblem with SetFPLED(LED_S1, %d), Status = %d", led_s2_value, status);
		}
	}

	vTaskDelete(NULL);
}


void vToggleDigIO(void* params)
{
	//Requires full DIO control and full direction output prior to being called

	s8 status = 0;
	u32 temp = 0;
	u32 *int_ptr = 0;
	u32 temp_int = 0;
	u8 dig_io_num = 0;
	//u8 portnum = 0;
	//u8 bitnum = 0;

	int_ptr = (u32 *)params;
	temp_int = 0x000000FF & (u32)(*int_ptr);
	dig_io_num = (u8)(temp_int);

	//vIdentifyDIOParams((u16) dig_io_num, &portnum, &bitnum);

	//temp = ((gpio_bank3 >> bitnum) >> (portnum * 8)) & 0x1;

	//temp = ~temp;

	//temp = (1 << bitnum) << (portnum * 8);

	temp = 1 << dig_io_num;

	//gpio_bank3 ^= temp;

	temp ^= gpio_bank3;


	//status = SetDIOAllValues(gpio_bank3);
	status = SetDIOAllValues(temp);
	if(status != 0)
	{
		xil_printf("\n\rProblem with SetDIOAllValues(%d), Status = %d", gpio_bank3, status);
	}

	vTaskDelete(NULL);
}


void ToggleLED(u8 led_num)
{
	s8 status = 0;

	if(led_num == 0)
	{
		led_s1_value = (LED_Status)(led_s1_value ^ LED_On);

		status = SetFPLED(LED_S1, led_s1_value);
		if(status != 0)
		{
			xil_printf("\n\rProblem with SetFPLED(LED_S1, %d), Status = %d", led_s1_value, status);
		}
	}
	else
	{
		led_s2_value = (LED_Status)(led_s1_value ^ LED_On);

		status = SetFPLED(LED_S2, led_s2_value);
		if(status != 0)
		{
			xil_printf("\n\rProblem with SetFPLED(LED_S1, %d), Status = %d", led_s2_value, status);
		}
	}
}


void ToggleDigIO(u8 bitnum)
{
	//Requires full DIO control and full direction output prior to being called

	s8 status = 0;
	u32 temp = 0;
	u32 dig_io_num = 0;

	dig_io_num = 0x0000001F & bitnum;

	temp = 1 << dig_io_num;

	temp ^= gpio_bank3;

	//status = SetDIOAllValues(gpio_bank3);
	status = SetDIOAllValues(temp);
	if(status != 0)
	{
		xil_printf("\n\rProblem with SetDIOAllValues(%d), Status = %d", gpio_bank3, status);
	}
}

void FastToggleDigIO(u8 bitnum)
{
	//Requires full DIO control and full direction output prior to being called

	s8 status = 0;
	u32 temp = 0;
	u32 dig_io_num = 0;

	dig_io_num = 0x0000001F & bitnum;

	temp = 1 << dig_io_num;

	temp ^= gpio_bank3;

	//status = SetDIOAllValues(gpio_bank3);
	status = SetDIOAllValuesNoAck(temp);
	if(status != 0)
	{
		xil_printf("\n\rProblem with SetDIOAllValues(%d), Status = %d", gpio_bank3, status);
	}
}


void vPulseTrain(void *pvparams)
{
	u32 high_time_ms = 0;
	u32 low_time_ms = 0;
	u8 portnum = 0;
	u8 bitnum = 0;
	//struct Pulse_Train_Params * pvt_params;
	//pvt_params = (struct Pulse_Train_Params *)pvparams;
	(void) pvparams;

//	high_time_ms = pvt_params[0].high_time_ms;
//	low_time_ms = pvt_params[0].low_time_ms;
//	portnum = pvt_params[0].portnum;
//	bitnum = pvt_params[0].bitnum;

	high_time_ms = pulse_params.high_time_ms;
	low_time_ms = pulse_params.low_time_ms;
	portnum = pulse_params.portnum;
	bitnum = pulse_params.bitnum;

	TickType_t xLastWakeTime;

	if(xSemaphoreTake(xPulse_Train_Sem, portMAX_DELAY) == pdTRUE)
	{
		const TickType_t xHighPeriod = (high_time_ms / portTICK_PERIOD_MS); //If timing is wrong, the chances are that the constants need to be updated for our application/clock speed...
		const TickType_t xLowPeriod = (low_time_ms / portTICK_PERIOD_MS); //If timing is wrong, the chances are that the constants need to be updated for our application/clock speed...

		SetDIOCtrl(portnum, HPP_Ctrl);
		SetDIOPortDir(portnum, DIO_PORT_DIR_OUTPUT);

		//Set pulse high
		SetDIOPortBit(portnum, bitnum, Bit_High);
		//xil_printf("1");
		//Get current time
		xLastWakeTime = xTaskGetTickCount();
		//Delay while bit is high
		vTaskDelayUntil(&xLastWakeTime, xHighPeriod);

		//Set pulse low
		SetDIOPortBit(portnum, bitnum, Bit_Low);
		//xil_printf("0");
		//Get current time
		xLastWakeTime = xTaskGetTickCount();
		//Delay while bit is low
		vTaskDelayUntil(&xLastWakeTime, xLowPeriod);
		xSemaphoreGive(xPulse_Train_Sem);
	}

	vTaskDelete(NULL);
}


void vGPIOInit(void)
{
	XGpioPs_Config *pxConfigPtr;
	BaseType_t xStatus;

	 //Initialize the PS GPIO driver
	pxConfigPtr = XGpioPs_LookupConfig(XPAR_XGPIOPS_0_DEVICE_ID);
	xStatus = XGpioPs_CfgInitialize(&xGpiops, pxConfigPtr, pxConfigPtr->BaseAddr);

	configASSERT(xStatus == XST_SUCCESS);
	(void) xStatus; // Remove compiler warning if configASSERT() is not defined.

	//Enable outputs
	XGpioPs_SetDirection(&xGpiops, GPIO_BANK_2, DIR_OUTPUT);
	XGpioPs_SetOutputEnable(&xGpiops, GPIO_BANK_2, DIR_OUTPUT);
	XGpioPs_Write(&xGpiops, GPIO_BANK_2, gpio_bank2);
	XGpioPs_SetDirection(&xGpiops, GPIO_BANK_3, DIR_OUTPUT);
	XGpioPs_SetOutputEnable(&xGpiops, GPIO_BANK_3, DIR_OUTPUT);
	XGpioPs_Write(&xGpiops, GPIO_BANK_3, gpio_bank3);
}


s8 SetPSControl(enum HPPMessageControl msg_controller)
{
	u32 temp = 0;

	//Clear GPIO Lines
	//ClearPSGPIO();

	temp = gpio_bank2 >> CONTROL_BIT;

	if(temp == msg_controller)
	{
		return 0;
	}

	if(msg_controller == 1)
	{
		temp = msg_controller << CONTROL_BIT;
		gpio_bank2 |= temp;
	}
	else
	{
		temp = 1 << CONTROL_BIT;
		temp = ~temp;
		//dio_values &= temp;
		gpio_bank2 &= temp;
	}

	XGpioPs_Write(&xGpiops, GPIO_BANK_2, gpio_bank2);

	//Purely a logic GPIO write, no message to MB for this activity.
	return 0;
}


s8 SetPSReady(enum HPPMessageControl msg_controller)
{
	u32 temp = 0;

	//Clear GPIO Lines
	//ClearPSGPIO();

	temp = gpio_bank2 >> READY_BIT;

	if(temp == msg_controller)
	{
		return 0;
	}

	if(msg_controller == 1)
	{
		temp = msg_controller << READY_BIT;
		gpio_bank2 |= temp;
	}
	else
	{
		temp = 1 << READY_BIT;
		temp = ~temp;
		//dio_values &= temp;
		gpio_bank2 &= temp;
	}

	XGpioPs_Write(&xGpiops, GPIO_BANK_2, gpio_bank2);

	//Purely a logic GPIO write, no message to MB for this activity.
	return 0;
}


s8 SetFTReady(enum HPPMessageControl msg_controller)
{
	u32 temp = 0;

	//Clear GPIO Lines
	//ClearPSGPIO();

	temp = gpio_bank2 >> FILE_TRANSFER_READY;

	if(temp == msg_controller)
	{
		return 0;
	}

	if(msg_controller == 1)
	{
		temp = msg_controller << FILE_TRANSFER_READY;
		gpio_bank2 |= temp;
	}
	else
	{
		temp = 1 << FILE_TRANSFER_READY;
		temp = ~temp;
		//dio_values &= temp;
		gpio_bank2 &= temp;
	}

	XGpioPs_Write(&xGpiops, GPIO_BANK_2, gpio_bank2);

	//Purely a logic GPIO write, no message to MB for this activity.
	return 0;
}


s8 SetDIOCtrl(u8 portnum, enum ControlSource source)
{
	s8 status = 0;
	void *unused = NULL;
	u32 temp = 0;

	temp = ((gpio_bank2 >> TTL_IO_CTL_BASE) >> portnum) & 0x1;

	if(temp == source)
	{
		return 0;
	}

	temp = (1 << TTL_IO_CTL_BASE) << portnum;
	temp = ~temp;
	gpio_bank2 &= temp;

	temp = (source << TTL_IO_CTL_BASE) << portnum;
	gpio_bank2 |= temp;
	XGpioPs_Write(&xGpiops, GPIO_BANK_2, gpio_bank2);

	//Need to process acknowledgment, so take data semaphore
	if(xSemaphoreTake(xHPP_Response_Sem, portMAX_DELAY) == pdTRUE)
	{
		status = ProcessAck(unused);
	}
	else
	{
		status = -10;
	}
	return status;
}


s8 SetFPLEDCtrl(enum ControlSource source)
{
	s8 status = 0;
	void *unused = NULL;
	u32 temp = 0;

	temp = (gpio_bank2 >> LED_CTL_BIT) & 0x1;

	if(temp == source)
	{
		return 0;
	}


	temp = 1 << LED_CTL_BIT;
	temp = ~temp;
	gpio_bank2 &= temp;

	temp = source << LED_CTL_BIT;
	gpio_bank2 |= temp;
	XGpioPs_Write(&xGpiops, GPIO_BANK_2, gpio_bank2);

	//Need to process acknowledgment, so take data semaphore
	if(xSemaphoreTake(xHPP_Response_Sem, portMAX_DELAY) == pdTRUE)
	{
		status = ProcessAck(unused);
	}
	else
	{
		status = -10;
	}
	return status;
}


s8 SetAnalogOutputCtrl(enum ControlSource source)
{
	s8 status = 0;
	void *unused = NULL;
	u32 temp = 0;

	temp = (gpio_bank2 >> ANALOG_CTL_BIT) & 0x1;

	if(temp == source)
	{
		return 0;
	}

	temp = 1 << ANALOG_CTL_BIT;
	temp = ~temp;
	gpio_bank2 &= temp;

	temp = source << ANALOG_CTL_BIT;
	gpio_bank2 |= temp;
	XGpioPs_Write(&xGpiops, GPIO_BANK_2, gpio_bank2);

	//Need to process acknowledgment, so take data semaphore
	if(xSemaphoreTake(xHPP_Response_Sem, portMAX_DELAY) == pdTRUE)
	{
		status = ProcessAck(unused);
	}
	else
	{
		status = -10;
	}
	return status;
}


s8 SetHeadstageStimCtrl(enum ControlSource source)
{
	s8 status = 0;
	void *unused = NULL;
	u32 temp = 0;

	temp = (gpio_bank2 >> STIM_CTL_BIT) & 0x1;

	if(temp == source)
	{
		return 0;
	}

	temp = 1 << STIM_CTL_BIT;
	temp = ~temp;
	gpio_bank2 &= temp;

	temp = source << STIM_CTL_BIT;
	gpio_bank2 |= temp;
	XGpioPs_Write(&xGpiops, GPIO_BANK_2, gpio_bank2);

	//Need to process acknowledgment, so take data semaphore
	if(xSemaphoreTake(xHPP_Response_Sem, portMAX_DELAY) == pdTRUE)
	{
		status = ProcessAck(unused);
	}
	else
	{
		status = -10;
	}
	return status;
}


s8 SetDIOPortDir(u8 portnum, enum DigitalIOPortDirection dir)
{
	s8 status = 0;
	void *unused = NULL;
	u32 temp = 0;

	temp = ((gpio_bank2 >> TTL_PORT_DIR_BASE) >> portnum) & 0x1;

	if(temp == dir)
	{
		return 0;
	}

	temp = (1 << TTL_PORT_DIR_BASE) << portnum;
	temp = ~temp;
	gpio_bank2 &= temp;

	temp = (dir << TTL_PORT_DIR_BASE) << portnum;
	gpio_bank2 |= temp;
	XGpioPs_Write(&xGpiops, GPIO_BANK_2, gpio_bank2);

	//Need to process acknowledgment, so take data semaphore
	if(xSemaphoreTake(xHPP_Response_Sem, portMAX_DELAY) == pdTRUE)
	{
		status = ProcessAck(unused);
	}
	else
	{
		status = -10;
	}
	return status;
}


s8 SetDIOPortBit(u8 portnum, u8 bitnum, enum BitValue bitval)
{
	s8 status = 0;
	void *unused = NULL;
	u32 temp = 0;

	temp = ((gpio_bank3 >> bitnum) >> (portnum * 8)) & 0x1;

	if(temp == bitval)
	{
		return 0;
	}

	temp = (1 << bitnum) << (portnum * 8);
	temp = ~temp;
	gpio_bank3 &= temp;

	temp = (bitval << bitnum) << (portnum * 8);
	gpio_bank3 |= temp;
	XGpioPs_Write(&xGpiops, GPIO_BANK_3, gpio_bank3);

	//Need to process acknowledgment, so take data semaphore
	if(xSemaphoreTake(xHPP_Response_Sem, portMAX_DELAY) == pdTRUE)
	{
		status = ProcessAck(unused);
	}
	else
	{
		status = -10;
	}
	return status;
}


s8 SetDIOPortValue(u8 portnum, u8 value)
{
	s8 status = 0;
	void *unused = NULL;
	u32 temp = 0;

	temp = (gpio_bank3 >> (portnum * 8)) & 0xFF;

	if(temp == value)
	{
		return 0;
	}

	temp = 0xFF << (portnum * 8);
	temp = ~temp;
	gpio_bank3 &= temp;

	temp = value << (portnum * 8);
	gpio_bank3 |= temp;
	XGpioPs_Write(&xGpiops, GPIO_BANK_3, gpio_bank3);

	//Need to process acknowledgment, so take data semaphore
	if(xSemaphoreTake(xHPP_Response_Sem, portMAX_DELAY) == pdTRUE)
	{
		status = ProcessAck(unused);
	}
	else
	{
		status = -10;
	}
	return status;
}


s8 SetDIOAllValues(u32 values)
{
	s8 status = 0;
	void *unused = NULL;

	if(gpio_bank3 == values)
	{
		return 0;
	}

	gpio_bank3 = values;
	XGpioPs_Write(&xGpiops, GPIO_BANK_3, gpio_bank3);

	//Need to process acknowledgment, so take data semaphore
	if(xSemaphoreTake(xHPP_Response_Sem, portMAX_DELAY) == pdTRUE)
	{
		status = ProcessAck(unused);
	}
	else
	{
		status = -10;
	}
	return status;
}

s8 SetDIOAllValuesNoAck(u32 values)
{
	s8 status = 0;

	if(gpio_bank3 == values)
	{
		return 0;
	}

	gpio_bank3 = values;
	XGpioPs_Write(&xGpiops, GPIO_BANK_3, gpio_bank3);

	//Need to process acknowledgment, so take data semaphore
	/*if(xSemaphoreTake(xHPP_Response_Sem, portMAX_DELAY) == pdTRUE)
	{
		status = ProcessAck(unused);
	}
	else
	{
		status = -10;
	}*/
	return status;
}


s8 SetFPLED(enum LEDNum led_num, enum LED_Status value)
{
	s8 status = 0;
	void *unused = NULL;
	u32 temp = 0;

	temp = ((gpio_bank2 >> S1_DATA_BIT) >> led_num) & 0x1;

	if(temp == value)
	{
		return 0;
	}

	temp = (0x1 << S1_DATA_BIT) << led_num;
	temp = ~temp;
	gpio_bank2 &= temp;

	temp = (value << S1_DATA_BIT) << led_num;
	gpio_bank2 |= temp;
	XGpioPs_Write(&xGpiops, GPIO_BANK_2, gpio_bank2);

	if(led_num == 0)
	{
		led_s1_value = value;
	}
	else
	{
		led_s2_value = value;
	}

	//Need to process acknowledgment, so take data semaphore
	if(xSemaphoreTake(xHPP_Response_Sem, portMAX_DELAY) == pdTRUE)
	{
		status = ProcessAck(unused);
	}
	return status;
}


void New_HPPCmd(void)
{
	HPP_Cmd.SOP = 0x00000800;
	HPP_Cmd.PacketID = 0x00000002;
	HPP_Cmd.Size = 0;
	HPP_Cmd.Stx = 8;
	HPP_Cmd.RecordID = 0;
	HPP_Cmd.DataSize = 2;
	strcpy(HPP_Cmd.Data,"0\0");
	HPP_Cmd.CRC = 0;
	HPP_Cmd.OverallCRC = 0;
}


void New_AudioFilterCmd(void)
{
	u8 afindex = 0;
	HPP_AF.SOP = 0x00000800;
	HPP_AF.PacketID = 0x00000002;
	HPP_AF.Size = 0;
	HPP_AF.Stx = 8;
	HPP_AF.RecordID = 0;
	HPP_AF.DataSize = 8;
	strcpy(HPP_AF.Data,"0\0");

	for(afindex = 0; afindex < 128; afindex++)
	{
		HPP_AF.lowcutcoeffs[afindex] = 0.0F;
		HPP_AF.highcutcoeffs[afindex] = 0.0F;
	}
	strcpy(HPP_AF.Data2,"0\0");
	strcpy(HPP_AF.Data3,"0\0");
	HPP_AF.CRC = 0;
	HPP_AF.OverallCRC = 0;
}


void Calc8CRC(u8 msgType)
{
	u8 i = 0;
	u8 result = 0;
	u16 temp = 0;
	u8 temp2 = 0;
	u16 msg_size = 0;
	temp_msg_str_len = 0;

	u16 tempval = 0;

	if(msgType == PURE_ASCII_MSG)
	{
		temp_msg_str[0] = HPP_Cmd.Stx;
		temp_msg_str[1] = HPP_Cmd.RecordID;
		tempval = HPP_Cmd.DataSize;
		//tempval = (tempval & 0x00FF) << 8 | (tempval & 0xFF00) >> 8;
		memcpy(&temp_msg_str[2], &tempval, 2);
		memcpy(&temp_msg_str[4], &HPP_Cmd.Data, HPP_Cmd.DataSize);

		for(i = 0; i < HPP_Cmd.DataSize + 4; i++)
		{
			result ^= (u8)(temp_msg_str[i]);
		}
		HPP_Cmd.CRC = result;

		temp_msg_str[HPP_Cmd.DataSize + 4] = HPP_Cmd.CRC;
		temp_msg_str_len = HPP_Cmd.DataSize + 5;
	}
	else
	{
		temp = (HPP_AF.DataSize - strlen(HPP_AF.Data) - strlen(HPP_AF.Data2) - strlen(HPP_AF.Data3)) >> 1;
		temp_msg_str[0] = HPP_AF.Stx;
		temp_msg_str[1] = HPP_AF.RecordID;
		tempval = HPP_AF.DataSize;
		//tempval = (tempval & 0x00FF) << 8 | (tempval & 0xFF00) >> 8;
		memcpy(&temp_msg_str[2], &tempval, 2);
		temp2 = strlen(HPP_AF.Data);
		memcpy(&temp_msg_str[4], &HPP_AF.Data, temp2);
		msg_size = 4 + strlen(HPP_AF.Data);
		memcpy(&temp_msg_str[msg_size], &HPP_AF.lowcutcoeffs, temp);
		msg_size = msg_size + temp;
		temp2 = strlen(HPP_AF.Data2);
		memcpy(&temp_msg_str[msg_size], &HPP_AF.Data2, temp2);
		msg_size = msg_size + temp2;
		memcpy(&temp_msg_str[msg_size], &HPP_AF.highcutcoeffs, temp);
		msg_size = msg_size + temp;
		temp2 = strlen(HPP_AF.Data3);
		memcpy(&temp_msg_str[msg_size], &HPP_AF.Data3, temp2);
		msg_size = msg_size + temp2;

		temp_msg_str[msg_size] = 0;

		msg_size = HPP_AF.DataSize + 4;

		for(i = 0; i < msg_size; i++)
		{
			result ^= (u8)(temp_msg_str[i]);
		}
		HPP_AF.CRC = result;

		temp_msg_str[msg_size] = HPP_AF.CRC;

		HPP_AF.Size = HPP_AF.DataSize + 5; //Add 1 for CRC and 16 for Overall Message Wrapper
		temp_msg_str_len = HPP_AF.Size;
	}
}


void Calc32CRC(u8 msgType)
{
	u32 i = 0;
	u32 j = 0;
	u32 num_words = 0;
	u32 result = 0;
	u32 tempval = 0;
	u8 remainder = 0;
	cheetah_msg_str_len = 0;

	//Due to byte swapping of char * when copying to PL, the PL expects the numerical values to be byte-swapped as well...
	if(msgType == PURE_ASCII_MSG)
	{
		//Calculate CRC of string before it's loaded
		num_words = temp_msg_str_len >> 2;
		for(i = 0; i < num_words; i++)
		{
			tempval = 0;
			j = i << 2;
			tempval = ((u32)(temp_msg_str[j] << 24) & 0xFF000000);
			tempval |= ((u32)(temp_msg_str[j+1] << 16) & 0x00FF0000);
			tempval |= ((u32)(temp_msg_str[j+2] << 8) & 0x0000FF00);
			tempval |= ((u32)(temp_msg_str[j+3]) & 0x000000FF);

			result ^= tempval;
		}
		remainder = temp_msg_str_len % 4;

		tempval = 0;

		switch(remainder)
		{
			case 0:
				break;
			case 1:
				tempval = ((u32)(temp_msg_str[(num_words << 2)] << 24) & 0xFF000000);
				break;
			case 2:
				tempval = ((u32)(temp_msg_str[(num_words << 2)] << 24) & 0xFF000000);
				tempval |= ((u32)(temp_msg_str[(num_words << 2) + 1] << 16) & 0x00FF0000);
				break;
			case 3:
				tempval = ((u32)(temp_msg_str[(num_words << 2)] << 24) & 0xFF000000);
				tempval |= ((u32)(temp_msg_str[(num_words << 2) + 1] << 16) & 0x00FF0000);
				tempval |= ((u32)(temp_msg_str[(num_words << 2) + 2] << 8) & 0x0000FF00);
				break;
		}

		result ^= tempval;

		tempval = HPP_Cmd.SOP;
		result ^= tempval;
		tempval = (tempval & 0x0000FFFF) << 16 | (tempval & 0xFFFF0000) >> 16;
		tempval = (tempval & 0x00FF00FF) << 8 | (tempval & 0xFF00FF00) >> 8;
		memcpy(&cheetah_msg_str[0], &tempval, 4);
		//xil_printf("\n\rcheetah_msg_str[0] = %2x", cheetah_msg_str[0]);
		//xil_printf("\n\rcheetah_msg_str[1] = %2x", cheetah_msg_str[1]);
		//xil_printf("\n\rcheetah_msg_str[2] = %2x", cheetah_msg_str[2]);
		//xil_printf("\n\rcheetah_msg_str[3] = %2x", cheetah_msg_str[3]);
		tempval = HPP_Cmd.PacketID;
		result ^= tempval;
		tempval = (tempval & 0x0000FFFF) << 16 | (tempval & 0xFFFF0000) >> 16;
		tempval = (tempval & 0x00FF00FF) << 8 | (tempval & 0xFF00FF00) >> 8;
		memcpy(&cheetah_msg_str[4], &tempval, 4);
		//xil_printf("\n\rcheetah_msg_str[4] = %2x", cheetah_msg_str[4]);
		//xil_printf("\n\rcheetah_msg_str[5] = %2x", cheetah_msg_str[5]);
		//xil_printf("\n\rcheetah_msg_str[6] = %2x", cheetah_msg_str[6]);
		//xil_printf("\n\rcheetah_msg_str[7] = %2x", cheetah_msg_str[7]);
		tempval = HPP_Cmd.Size;
		result ^= tempval;
		tempval = (tempval & 0x0000FFFF) << 16 | (tempval & 0xFFFF0000) >> 16;
		tempval = (tempval & 0x00FF00FF) << 8 | (tempval & 0xFF00FF00) >> 8;
		memcpy(&cheetah_msg_str[8], &tempval, 4);
		//xil_printf("\n\rcheetah_msg_str[8] = %2x", cheetah_msg_str[8]);
		//xil_printf("\n\rcheetah_msg_str[9] = %2x", cheetah_msg_str[9]);
		//xil_printf("\n\rcheetah_msg_str[10] = %2x", cheetah_msg_str[10]);
		//xil_printf("\n\rcheetah_msg_str[11] = %2x", cheetah_msg_str[11]);
		memcpy(&cheetah_msg_str[12], temp_msg_str, temp_msg_str_len);
		//xil_printf("\n\rcheetah_msg_str[12] = %2x", cheetah_msg_str[12]);
		//xil_printf("\n\rtemp_msg_str = %2x", temp_msg_str);

		HPP_Cmd.OverallCRC = result;
		tempval = HPP_Cmd.OverallCRC;
		tempval = (tempval & 0x0000FFFF) << 16 | (tempval & 0xFFFF0000) >> 16;
		tempval = (tempval & 0x00FF00FF) << 8 | (tempval & 0xFF00FF00) >> 8;
		memcpy(&cheetah_msg_str[HPP_Cmd.Size + 12], &tempval, 4);
		cheetah_msg_str_len = HPP_Cmd.Size + 16;
	}
	else
	{
		//Calculate CRC of string before it's loaded
		num_words = temp_msg_str_len >> 2;
		for(i = 0; i < num_words; i++)
		{
			tempval = 0;
			j = i << 2;
			tempval = ((u32)(temp_msg_str[j] << 24) & 0xFF000000);
			tempval |= ((u32)(temp_msg_str[j+1] << 16) & 0x00FF0000);
			tempval |= ((u32)(temp_msg_str[j+2] << 8) & 0x0000FF00);
			tempval |= ((u32)(temp_msg_str[j+3]) & 0x000000FF);

			result ^= tempval;
		}
		remainder = temp_msg_str_len % 4;

		tempval = 0;

		switch(remainder)
		{
			case 0:
				break;
			case 1:
				tempval = ((u32)(temp_msg_str[(num_words << 2)] << 24) & 0xFF000000);
				break;
			case 2:
				tempval = ((u32)(temp_msg_str[(num_words << 2)] << 24) & 0xFF000000);
				tempval |= ((u32)(temp_msg_str[(num_words << 2) + 1] << 16) & 0x00FF0000);
				break;
			case 3:
				tempval = ((u32)(temp_msg_str[(num_words << 2)] << 24) & 0xFF000000);
				tempval |= ((u32)(temp_msg_str[(num_words << 2) + 1] << 16) & 0x00FF0000);
				tempval |= ((u32)(temp_msg_str[(num_words << 2) + 2] << 8) & 0x0000FF00);
				break;
		}

		result ^= tempval;

		tempval = HPP_AF.SOP;
		result ^= tempval;
		tempval = (tempval & 0x0000FFFF) << 16 | (tempval & 0xFFFF0000) >> 16;
		tempval = (tempval & 0x00FF00FF) << 8 | (tempval & 0xFF00FF00) >> 8;
		memcpy(&cheetah_msg_str[0], &tempval, 4);
		//xil_printf("\n\rcheetah_msg_str[0] = %2x", cheetah_msg_str[0]);
		//xil_printf("\n\rcheetah_msg_str[1] = %2x", cheetah_msg_str[1]);
		//xil_printf("\n\rcheetah_msg_str[2] = %2x", cheetah_msg_str[2]);
		//xil_printf("\n\rcheetah_msg_str[3] = %2x", cheetah_msg_str[3]);
		tempval = HPP_AF.PacketID;
		result ^= tempval;
		tempval = (tempval & 0x0000FFFF) << 16 | (tempval & 0xFFFF0000) >> 16;
		tempval = (tempval & 0x00FF00FF) << 8 | (tempval & 0xFF00FF00) >> 8;
		memcpy(&cheetah_msg_str[4], &tempval, 4);
		//xil_printf("\n\rcheetah_msg_str[4] = %2x", cheetah_msg_str[4]);
		//xil_printf("\n\rcheetah_msg_str[5] = %2x", cheetah_msg_str[5]);
		//xil_printf("\n\rcheetah_msg_str[6] = %2x", cheetah_msg_str[6]);
		//xil_printf("\n\rcheetah_msg_str[7] = %2x", cheetah_msg_str[7]);
		tempval = HPP_AF.Size;
		result ^= tempval;
		tempval = (tempval & 0x0000FFFF) << 16 | (tempval & 0xFFFF0000) >> 16;
		tempval = (tempval & 0x00FF00FF) << 8 | (tempval & 0xFF00FF00) >> 8;
		memcpy(&cheetah_msg_str[8], &tempval, 4);
		//xil_printf("\n\rcheetah_msg_str[8] = %2x", cheetah_msg_str[8]);
		//xil_printf("\n\rcheetah_msg_str[9] = %2x", cheetah_msg_str[9]);
		//xil_printf("\n\rcheetah_msg_str[10] = %2x", cheetah_msg_str[10]);
		//xil_printf("\n\rcheetah_msg_str[11] = %2x", cheetah_msg_str[11]);
		memcpy(&cheetah_msg_str[12], temp_msg_str, temp_msg_str_len);
		//xil_printf("\n\rcheetah_msg_str[12] = %2x", cheetah_msg_str[12]);
		//xil_printf("\n\rtemp_msg_str = %2x", temp_msg_str);

		HPP_AF.OverallCRC = result;
		tempval = HPP_AF.OverallCRC;
		tempval = (tempval & 0x0000FFFF) << 16 | (tempval & 0xFFFF0000) >> 16;
		tempval = (tempval & 0x00FF00FF) << 8 | (tempval & 0xFF00FF00) >> 8;
		memcpy(&cheetah_msg_str[HPP_AF.Size + 12], &tempval, 4);
		cheetah_msg_str_len = HPP_AF.Size + 16;




		/*
		tempval = HPP_Cmd.SOP;
		tempval = (tempval & 0x0000FFFF) << 16 | (tempval & 0xFFFF0000) >> 16;
		tempval = (tempval & 0x00FF00FF) << 8 | (tempval & 0xFF00FF00) >> 8;
		memcpy(&cheetah_msg_str[0], &tempval, 4);
		tempval = HPP_AF.PacketID;
		tempval = (tempval & 0x0000FFFF) << 16 | (tempval & 0xFFFF0000) >> 16;
		tempval = (tempval & 0x00FF00FF) << 8 | (tempval & 0xFF00FF00) >> 8;
		memcpy(&cheetah_msg_str[4], &tempval, 4);
		tempval = HPP_AF.Size;
		tempval = (tempval & 0x0000FFFF) << 16 | (tempval & 0xFFFF0000) >> 16;
		tempval = (tempval & 0x00FF00FF) << 8 | (tempval & 0xFF00FF00) >> 8;
		memcpy(&cheetah_msg_str[8], &tempval, 4);
		memcpy(&cheetah_msg_str[12], &temp_msg_str, temp_msg_str_len);

		num_words = ((HPP_AF.Size - 4) >> 2);  //Size contains length of CRC, which isn't included in calculating itself...

		for(i = 0; i < num_words; i++)
		{
			tempval = 0;
			j = i << 2;
			tempval = ((u32)(cheetah_msg_str[j] << 24) & 0xFF000000);
			tempval |= ((u32)(cheetah_msg_str[j+1] << 16) & 0x00FF0000);
			tempval |= ((u32)(cheetah_msg_str[j+2] << 8) & 0x0000FF00);
			tempval |= ((u32)(cheetah_msg_str[j+3]) & 0x000000FF);

			result ^= tempval;
		}

		remainder = HPP_AF.Size % 4;

		tempval = 0;

		switch(remainder)
		{
			case 0:
				break;
			case 1:
				tempval = ((u32)(cheetah_msg_str[(num_words << 2)] << 24) & 0xFF000000);
				break;
			case 2:
				tempval = ((u32)(cheetah_msg_str[(num_words << 2)] << 24) & 0xFF000000);
				tempval = ((u32)(cheetah_msg_str[(num_words << 2) + 1] << 16) & 0x00FF0000);
				break;
			case 3:
				tempval = ((u32)(cheetah_msg_str[(num_words << 2)] << 24) & 0xFF000000);
				tempval = ((u32)(cheetah_msg_str[(num_words << 2) + 1] << 16) & 0x00FF0000);
				tempval = ((u32)(cheetah_msg_str[(num_words << 2) + 2] << 8) & 0x0000FF00);
				break;
		}

		result ^= tempval;

		HPP_AF.OverallCRC = result;
		tempval = HPP_AF.OverallCRC;
		tempval = (tempval & 0x0000FFFF) << 16 | (tempval & 0xFFFF0000) >> 16;
		tempval = (tempval & 0x00FF00FF) << 8 | (tempval & 0xFF00FF00) >> 8;
		memcpy(&cheetah_msg_str[HPP_AF.Size - 4], &tempval, 4);
		cheetah_msg_str_len = HPP_AF.Size;*/
	}
}


void CalcCmdInternals(void)
{
	HPP_Cmd.DataSize = strlen(HPP_Cmd.Data) + 1;

	//Stx = 1 Byte
	//RecordID = 1 Byte
	//DataSize = 2 Bytes
	//CRC = 1 Byte
	HPP_Cmd.Size = HPP_Cmd.DataSize + 5;

	Calc8CRC(PURE_ASCII_MSG);

	Calc32CRC(PURE_ASCII_MSG);
}


void CalcAFInternals(u16 lowcuttapcount, u16 highcuttapcount)
{
	u32 temp = 0;
	u32 temp1 = 0;
	u32 temp2 = 0;
	u32 temp3 = 0;
	u32 temp4 = 0;

	temp = strlen(HPP_AF.Data);
	temp1 = strlen(HPP_AF.Data2);
	temp2 = strlen(HPP_AF.Data3);
	temp3 = (lowcuttapcount << 1);
	temp4 = (highcuttapcount << 1);

	//DataSize = lengths of the 3 strings + 2*lowtapcount (# values = lowtapcount/2 * 4 bytes per value) + 2*hightapcount
	HPP_AF.DataSize = temp + temp1 + temp2 + temp3 + temp4 + 1; //Add 1 for null terminator at the end

	//Stx = 1 Byte
	//RecordID = 1 Byte
	//DataSize = 2 Bytes
	//CRC = 1 Byte
	HPP_AF.Size = HPP_AF.DataSize + 5;

	Calc8CRC(FILTER_MSG);

	Calc32CRC(FILTER_MSG);
}


void Send_HPPCmd(void)
{
	u32 temp = 0;
	//Copy Message to AXI GP0
	memcpy((int *)CHEETAH_CMD_ADDR, cheetah_msg_str, cheetah_msg_str_len);

	//Set ASCII Transfer Complete Bit
	temp = 1 << ASCII_TRANSFER;
	gpio_bank2 |= temp;
	XGpioPs_Write(&xGpiops, GPIO_BANK_2, gpio_bank2);

	//Clear ASCII Transfer Complete Bit
	gpio_bank2 ^= temp;
	XGpioPs_Write(&xGpiops, GPIO_BANK_2, gpio_bank2);
}


s8 ProcessAck(void *result)
{
	(void) result;
	return HPP_Logic_Ack[0].Status;
}


s8 ProcessAsciiAck(void *result)
{
	s32 error_code;
	s8 status = 0;
	char *temp_str = NULL;
	u32 sample_freq = 0;
	u32 *sample_freq_ptr = NULL;

	temp_str = strtok(HPP_Ascii_Ack[0].Command, " ");
	temp_str = strtok(NULL, " ");
	error_code = atoi(temp_str);
	status = (s8)(error_code);
	temp_str = strtok(NULL, " ");

	if(strstr(HPP_Ascii_Ack[0].Command, "-GetSampleFrequency") != NULL)
	{
		sample_freq = atoi(temp_str);
		sample_freq_ptr = &sample_freq;
		memcpy(result, sample_freq_ptr, 4);
	}
	if(strstr(HPP_Ascii_Ack[0].Command, "-GetBoardInformation") != NULL)
	{
		strcpy((char *)result, temp_str);
	}

	return status;
}


s8 StartAcquisition()
{
	s8 status = 0;
	void *unused = NULL;

	New_HPPCmd();

	strcpy(HPP_Cmd.Data,"-StartAcquisition\0");

	CalcCmdInternals();

	Send_HPPCmd();

	//Need to process acknowledgment, so take data semaphore
	if(xSemaphoreTake( xHPP_Response_Sem, portMAX_DELAY ) == pdTRUE)
	{
		status = ProcessAsciiAck(unused);
	}
	return status;
}


s8 StopAcquisition()
{
	s8 status = 0;
	void *unused = NULL;

	New_HPPCmd();

	strcpy(HPP_Cmd.Data, "-StopAcquisition\0");

	CalcCmdInternals();

	Send_HPPCmd();

	//Need to process acknowledgment, so take data semaphore
	if(xSemaphoreTake( xHPP_Response_Sem, portMAX_DELAY ) == pdTRUE)
	{
		status = ProcessAsciiAck(unused);
	}
	return status;
}


s8 GetSampleFrequency(u32 *sample_freq)
{
	s8 status = 0;
	New_HPPCmd();

	strcpy(HPP_Cmd.Data, "-GetSampleFrequency\0");

	CalcCmdInternals();

	Send_HPPCmd();

	//Need to get return value, so take data semaphore


	if(xSemaphoreTake( xHPP_Response_Sem, portMAX_DELAY ) == pdTRUE)
	{
		//Process ack to get sample frequency
		status = ProcessAsciiAck((void *)sample_freq);
	}
	return status;
}


s8 GetBoardInformation(u32 boardnum, char *board_type)
{
	s8 status = 0;
	New_HPPCmd();

	//Need to verify sprintf includes the '\0' in the output
	sprintf(HPP_Cmd.Data, "-GetBoardInformation %u", (unsigned int)boardnum);

	CalcCmdInternals();

	Send_HPPCmd();

	//Need to get return value, so take data semaphore


	if(xSemaphoreTake( xHPP_Response_Sem, portMAX_DELAY ) == pdTRUE)
	{
		status = ProcessAsciiAck((void *) board_type);
	}

	return status;
}


s8 DRSCommand(char *cmdstring)
{
	s8 status = 0;
	void *unused = NULL;
	New_HPPCmd();

	//Need to verify sprintf includes the '\0' in the output
	//sprintf(HPP_Cmd.Data, "-DrsCommand %s\0", cmdstring);
	sprintf(HPP_Cmd.Data, "-DrsCommand %s", cmdstring);

	CalcCmdInternals();

	Send_HPPCmd();

	//Need to process acknowledgment, so take data semaphore
	if(xSemaphoreTake( xHPP_Response_Sem, portMAX_DELAY ) == pdTRUE)
	{
		status = ProcessAsciiAck(unused);
	}
	return status;
}


s8 SetDigitalIOPortDirection(u8 portnum, enum DigitalIOPortDirection direction)
{
	s8 status = 0;
	void *unused = NULL;
	New_HPPCmd();

	//Need to verify sprintf includes the '\0' in the output
	//sprintf(HPP_Cmd.Data, "-SetDigitalIOPortDirection %d %d\0", portnum, (u8)direction);
	sprintf(HPP_Cmd.Data, "-SetDigitalIOPortDirection %d %d", portnum, (u8)direction);

	CalcCmdInternals();

	Send_HPPCmd();

	//Need to process acknowledgment, so take data semaphore
	//if(xSemaphoreTake( xHPP_Response_Sem, portMAX_DELAY ) == pdTRUE)
	if(xSemaphoreTake(xHPP_Response_Sem, portMAX_DELAY) == pdTRUE)
	{
		status = ProcessAsciiAck(unused);
	}
	else
	{
		xil_printf("Problem taking semaphore in SetDIOPortDir\n\r");
	}

	return status;
}


s8 ExecuteTTLPulse(u8 portnum, u8 bitnum, enum TTLPulseType pulsetype)
{
	s8 status = 0;
	void *unused = NULL;
	New_HPPCmd();

	//Need to verify sprintf includes the '\0' in the output
	//sprintf(HPP_Cmd.Data, "-ExecuteTTLPulse %d %d %d\0", portnum, bitnum, (u8)pulsetype);
	sprintf(HPP_Cmd.Data, "-ExecuteTTLPulse %d %d %d", portnum, bitnum, (u8)pulsetype);

	CalcCmdInternals();

	Send_HPPCmd();

	//Need to process acknowledgment, so take data semaphore
	if(xSemaphoreTake( xHPP_Response_Sem, portMAX_DELAY ) == pdTRUE)
	{
		status = ProcessAsciiAck(unused);
	}
	return status;
}


s8 SetTTLPulseDuration(u8 portnum, u16 duration)
{
	s8 status = 0;
	void *unused = NULL;
	New_HPPCmd();

	//Need to verify sprintf includes the '\0' in the output
	//sprintf(HPP_Cmd.Data, "-SetTTLPulseDuration %d %d\0", portnum, duration);
	sprintf(HPP_Cmd.Data, "-SetTTLPulseDuration %d %d", portnum, duration);

	CalcCmdInternals();

	Send_HPPCmd();

	//Need to process acknowledgment, so take data semaphore
	if(xSemaphoreTake( xHPP_Response_Sem, portMAX_DELAY ) == pdTRUE)
	{
		status = ProcessAsciiAck(unused);
	}
	return status;
}


s8 SetDigitalIOBit(u8 portnum, u8 bitnum, enum BitValue value)
{
	s8 status = 0;
	void *unused = NULL;
	New_HPPCmd();

	//Need to verify sprintf includes the '\0' in the output
	//sprintf(HPP_Cmd.Data, "-SetDigitalIOBit %d %d %d\0", portnum, bitnum, value);
	sprintf(HPP_Cmd.Data, "-SetDigitalIOBit %d %d %d", portnum, bitnum, value);

	CalcCmdInternals();

	Send_HPPCmd();

	//Need to process acknowledgment, so take data semaphore
	if(xSemaphoreTake( xHPP_Response_Sem, portMAX_DELAY ) == pdTRUE)
	{
		status = ProcessAsciiAck(unused);
	}
	return status;
}


s8 SetDigitalIOPortValue(u8 portnum, u8 value)
{
	s8 status = 0;
	void *unused = NULL;
	New_HPPCmd();

	//Need to verify sprintf includes the '\0' in the output
	//sprintf(HPP_Cmd.Data, "-SetDigitalIOPortValue %d %d\0", portnum, value);
	sprintf(HPP_Cmd.Data, "-SetDigitalIOPortValue %d %d", portnum, value);

	CalcCmdInternals();

	Send_HPPCmd();

	//Need to process acknowledgment, so take data semaphore
	if(xSemaphoreTake( xHPP_Response_Sem, portMAX_DELAY ) == pdTRUE)
	{
		status = ProcessAsciiAck(unused);
	}
	return status;
}


s8 SetAudioSource(u8 channel, enum AudioSource source)
{
	s8 status = 0;
	void *unused = NULL;
	New_HPPCmd();

	//Need to verify sprintf includes the '\0' in the output
	//sprintf(HPP_Cmd.Data, "-SetAudioSource %d %d\0", channel, (u8)(source));
	sprintf(HPP_Cmd.Data, "-SetAudioSource %d %d", channel, (u8)(source));

	CalcCmdInternals();

	Send_HPPCmd();

	//Need to process acknowledgment, so take data semaphore
	if(xSemaphoreTake( xHPP_Response_Sem, portMAX_DELAY ) == pdTRUE)
	{
		status = ProcessAsciiAck(unused);
	}
	return status;
}


//Check ASCII commands from here down
s8 SetADSignalOutput(u8 daoutchannel, u8 dasubchannel, u16 adchannel)
{
	s8 status = 0;
	void *unused = NULL;
	New_HPPCmd();

	//Need to verify sprintf includes the '\0' in the output
	//sprintf(HPP_Cmd.Data, "-SetADSignalOutput %d %d %d\0", daoutchannel, dasubchannel, adchannel);
	sprintf(HPP_Cmd.Data, "-SetADSignalOutput %d %d %d", daoutchannel, dasubchannel, adchannel);

	CalcCmdInternals();

	Send_HPPCmd();

	//Need to process acknowledgment, so take data semaphore
	if(xSemaphoreTake( xHPP_Response_Sem, portMAX_DELAY ) == pdTRUE)
	{
		status = ProcessAsciiAck(unused);
	}
	return status;
}


s8 SetWaveformSignalOutput(u8 daoutchannel, u16 buffer, char *mode)
{
	s8 status = 0;
	void *unused = NULL;
	New_HPPCmd();

	//Need to verify sprintf includes the '\0' in the output
	//sprintf(HPP_Cmd.Data, "-SetWaveformSignalOutput %d %d %s\0", daoutchannel, buffer, mode);
	sprintf(HPP_Cmd.Data, "-SetWaveformSignalOutput %d %d %s", daoutchannel, buffer, mode);

	CalcCmdInternals();

	Send_HPPCmd();

	//Need to process acknowledgment, so take data semaphore
	if(xSemaphoreTake( xHPP_Response_Sem, portMAX_DELAY ) == pdTRUE)
	{
		status = ProcessAsciiAck(unused);
	}
	return status;
}


s8 PlayOutputWaveform(u8 daoutchannel, enum ContinuousType continuous)
{
	s8 status = 0;
	void *unused = NULL;
	New_HPPCmd();

	//Need to verify sprintf includes the '\0' in the output
	//sprintf(HPP_Cmd.Data, "-PlayOutputWaveform %d %d\0", daoutchannel, (u8)(continuous));
	sprintf(HPP_Cmd.Data, "-PlayOutputWaveform %d %d", daoutchannel, (u8)(continuous));

	CalcCmdInternals();

	Send_HPPCmd();

	//Need to process acknowledgment, so take data semaphore
	if(xSemaphoreTake( xHPP_Response_Sem, portMAX_DELAY ) == pdTRUE)
	{
		status = ProcessAsciiAck(unused);
	}
	return status;
}


s8 SetDAOutputFrequency(u8 daoutchannel, u16 rate)
{
	s8 status = 0;
	void *unused = NULL;
	New_HPPCmd();

	//Need to verify sprintf includes the '\0' in the output
	//sprintf(HPP_Cmd.Data, "-SetDAOutputFrequency %d %d\0", daoutchannel, rate);
	sprintf(HPP_Cmd.Data, "-SetDAOutputFrequency %d %d", daoutchannel, rate);

	CalcCmdInternals();

	Send_HPPCmd();

	//Need to process acknowledgment, so take data semaphore
	if(xSemaphoreTake( xHPP_Response_Sem, portMAX_DELAY ) == pdTRUE)
	{
		status = ProcessAsciiAck(unused);
	}
	return status;
}


s8 InitiateWaveformDownload(char *ipaddress, u8 portnum)
{
	s8 status = 0;
	void *unused = NULL;
	New_HPPCmd();

	//Need to verify sprintf includes the '\0' in the output
	//sprintf(HPP_Cmd.Data, "-InitWaveformDownload %s %d\0", ipaddress, portnum);
	sprintf(HPP_Cmd.Data, "-InitWaveformDownload %s %d", ipaddress, portnum);

	CalcCmdInternals();

	Send_HPPCmd();

	//Need to process acknowledgment, so take data semaphore
	if(xSemaphoreTake( xHPP_Response_Sem, portMAX_DELAY ) == pdTRUE)
	{
		status = ProcessAsciiAck(unused);
	}
	return status;
}


s8 ClearAudioOutput(u8 daoutchannel, u8 dasubchannel)
{
	s8 status = 0;
	void *unused = NULL;
	New_HPPCmd();

	//Need to verify sprintf includes the '\0' in the output
	//sprintf(HPP_Cmd.Data, "-ClearAudioOutput %d %d\0", daoutchannel, dasubchannel);
	sprintf(HPP_Cmd.Data, "-ClearAudioOutput %d %d", daoutchannel, dasubchannel);

	CalcCmdInternals();

	Send_HPPCmd();

	//Need to process acknowledgment, so take data semaphore
	if(xSemaphoreTake( xHPP_Response_Sem, portMAX_DELAY ) == pdTRUE)
	{
		status = ProcessAsciiAck(unused);
	}
	return status;
}


s8 SetAudioFilter(u16 adchannel, u8 daoutchannel, u8 dasubchannel, double lowcutfreq,
					u16 lowcuttapcount, float *lowcutcoeffs, enum FilterEnabled lowcutenabled,
					double highcutfreq, u16 highcuttapcount, float *highcutcoeffs, enum
					FilterEnabled highcutenabled)
{
	s8 status = 0;
	void *unused = NULL;
	//char str1[256];
	//u32 len1 = 0;
	//char str2[256];
	//u32 len2 = 0;
	//char str3[256];
	//u32 len3 = 0;

	New_AudioFilterCmd();

	//sprintf(str1, "-SetAudioFilter %d %d %d ", adchannel, daoutchannel, dasubchannel);

	//sprintf(str2, "%f", lowcutfreq);

	//sprintf(str3, " %d ", lowcuttapcount);

	//strcat(str1, str2);
	//strcat(str1, str3);

	//strcpy(HPP_AF.Data, str1);

	sprintf(HPP_AF.Data, "-SetAudioFilter %d %d %d %f %d ", adchannel, daoutchannel, dasubchannel, lowcutfreq, lowcuttapcount);

	memcpy(&HPP_AF.lowcutcoeffs, lowcutcoeffs, lowcuttapcount << 1);

	sprintf(HPP_AF.Data2, "%d %f %d ", (u8)lowcutenabled, highcutfreq, highcuttapcount);

	memcpy(&HPP_AF.highcutcoeffs, highcutcoeffs, highcuttapcount << 1);

	//Need to verify sprintf includes the '\0' in the output
	//sprintf(HPP_AF.Data3, " %d \0", (u8)highcutenabled);
	sprintf(HPP_AF.Data3, "%d", (u8)highcutenabled);

	CalcAFInternals(lowcuttapcount, highcuttapcount);

	Send_HPPCmd();

	//Need to process acknowledgment, so take data semaphore
	if(xSemaphoreTake( xHPP_Response_Sem, portMAX_DELAY ) == pdTRUE)
	{
		status = ProcessAsciiAck(unused);
	}
	return status;
}


s8 SetAudioVolume(u8 daoutchannel, u8 volume)
{
	s8 status = 0;
	void *unused = NULL;
	New_HPPCmd();

	//Need to verify sprintf includes the '\0' in the output
	//sprintf(HPP_Cmd.Data, "-SetAudioVolume %d %d\0", daoutchannel, volume);
	sprintf(HPP_Cmd.Data, "-SetAudioVolume %d %d", daoutchannel, volume);

	CalcCmdInternals();

	Send_HPPCmd();

	//Need to process acknowledgment, so take data semaphore
	if(xSemaphoreTake( xHPP_Response_Sem, portMAX_DELAY ) == pdTRUE)
	{
		status = ProcessAsciiAck(unused);
	}
	return status;
}


s8 SetAudioInputInverted(u8 daoutchannel, u8 dasubchannel, enum InputInversion invert)
{
	s8 status = 0;
	void *unused = NULL;
	New_HPPCmd();

	//Need to verify sprintf includes the '\0' in the output
	//sprintf(HPP_Cmd.Data, "-SetAudioInputInverted %d %d %d\0", dasubchannel, daoutchannel, (u8)(invert));
	sprintf(HPP_Cmd.Data, "-SetAudioInputInverted %d %d %d", daoutchannel, dasubchannel, (u8)(invert));

	CalcCmdInternals();

	Send_HPPCmd();

	//Need to process acknowledgment, so take data semaphore
	if(xSemaphoreTake( xHPP_Response_Sem, portMAX_DELAY ) == pdTRUE)
	{
		status = ProcessAsciiAck(unused);
	}
	return status;
}


s8 SetAudioInputRange(u16 adchannel, u8 daoutchannel, s32 lowrange, s32 highrange)
{
	s8 status = 0;
	void *unused = NULL;
	New_HPPCmd();

	//Need to verify sprintf includes the '\0' in the output
	//sprintf(HPP_Cmd.Data, "-SetAudioInputRange %d %d %d %d\0", adchannel, daoutchannel, lowrange, highrange);
	sprintf(HPP_Cmd.Data, "-SetAudioInputRange %u %u %d %d", (unsigned int)adchannel, (unsigned int)daoutchannel, (int)lowrange, (int)highrange);

	CalcCmdInternals();

	Send_HPPCmd();

	//Need to process acknowledgment, so take data semaphore
	if(xSemaphoreTake( xHPP_Response_Sem, portMAX_DELAY ) == pdTRUE)
	{
		status = ProcessAsciiAck(unused);
	}
	return status;
}


s8 HeadstageStimCommand(char *command)
{
	s8 status = 0;
	void *unused = NULL;
	New_HPPCmd();

	//Need to verify sprintf includes the '\0' in the output
	//sprintf(HPP_Cmd.Data, "-HeadstageStimCommand %s\0", command);
	sprintf(HPP_Cmd.Data, "-HSStimCommand %s", command);

	CalcCmdInternals();

	Send_HPPCmd();

	//Need to process acknowledgment, so take data semaphore
	if(xSemaphoreTake( xHPP_Response_Sem, portMAX_DELAY ) == pdTRUE)
	{
		status = ProcessAsciiAck(unused);
	}
	return status;
}


s8 SetEth2IP(char *ipaddress)
{
	s8 status = 0;
	void *unused = NULL;
	New_HPPCmd();

	//Need to verify sprintf includes the '\0' in the output
	//sprintf(HPP_Cmd.Data, "-SetEth2IP %s\0", ipaddress);
	sprintf(HPP_Cmd.Data, "-SetEth2IP %s", ipaddress);

	CalcCmdInternals();

	Send_HPPCmd();

	//Need to process acknowledgment, so take data semaphore
	if(xSemaphoreTake( xHPP_Response_Sem, portMAX_DELAY ) == pdTRUE)
	{
		status = ProcessAsciiAck(unused);
	}
	return status;
}



void ClearPSGPIO(void)
{
	XGpioPs_Write(&xGpiops, GPIO_BANK_2, ALL_LOW);
	gpio_bank2 = 0;
	XGpioPs_Write(&xGpiops, GPIO_BANK_3, ALL_LOW);
	gpio_bank3 = 0;
}


s8 UpdateAnalog(u8 update_mask, u16 analog0, u16 analog1, u16 analog2, u16 analog3)
{

	//Address 0x83C00000
	u32 Analog_Address = XPAR_ANALOG_M_AXI_BASEADDR;
	char analog_data[9];

	AnalogData.Update_Mask = update_mask;
	AnalogData.Analog_Out_0_Data = analog0;
	AnalogData.Analog_Out_1_Data = analog1;
	AnalogData.Analog_Out_2_Data = analog2;
	AnalogData.Analog_Out_3_Data = analog3;

	//memcpy((int *)Analog_Address, &AnalogData, sizeof(AnalogData));
	xil_printf("\n\rupdate_mask = %d, analog0 = %d, analog1 = %d, analog2 = %d, analog3 = %d\n\r", update_mask, analog0, analog1, analog2, analog3);
	memcpy(&analog_data[0], &update_mask, 1);
	xil_printf("\n\ranalog_data[0] = %d\n\r", analog_data[0]);
	memcpy(&analog_data[1], &analog0, 2);
	xil_printf("\n\ranalog_data[1] = %d, analog_data[2] = %d\n\r", analog_data[1], analog_data[2]);
	memcpy(&analog_data[3], &analog1, 2);
	xil_printf("\n\ranalog_data[3] = %d, analog_data[4] = %d\n\r", analog_data[3], analog_data[4]);
	memcpy(&analog_data[5], &analog2, 2);
	xil_printf("\n\ranalog_data[5] = %d, analog_data[6] = %d\n\r", analog_data[5], analog_data[6]);
	memcpy(&analog_data[7], &analog3, 2);
	xil_printf("\n\ranalog_data[7] = %d, analog_data[8] = %d\n\r", analog_data[7], analog_data[8]);

	memcpy((int *)Analog_Address, analog_data, 9);

	return 0;
}


void IdentifyDIOParams(u16 bit_number, u8 *portnum, u8 *bitnum)
{
	switch(bit_number)
		{
			case 0:
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
				//portnum = (u8 *)0;
				*portnum = 0;
				break;
			case 8:
			case 9:
			case 10:
			case 11:
			case 12:
			case 13:
			case 14:
			case 15:
				//portnum = (u8 *)1;
				*portnum = 1;
				break;
			case 16:
			case 17:
			case 18:
			case 19:
			case 20:
			case 21:
			case 22:
			case 23:
				//portnum = (u8 *)2;
				*portnum = 2;
				break;
			case 24:
			case 25:
			case 26:
			case 27:
			case 28:
			case 29:
			case 30:
			case 31:
				//portnum = (u8 *)3;
				*portnum = 3;
				break;
		}

		//Identify bit number to be passed to set digital I/O commands
		//bitnum = (u8 *)(bit_number & 0x00000007);
		*bitnum = bit_number & 0x00000007;
}
