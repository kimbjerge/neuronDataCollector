/*
 * HPPDataGenerator.cpp
 *
 *  Created on: 11. dec. 2018
 *      Author: au288681
 */

///////////////////////////////////////////////////////////
//  TestDataGenerator.cpp
//  Implementation of the Class TestDataGenerator
//  Created on:      24-maj-2017 11:01:53
//  Original author: Kim Bjerge
///////////////////////////////////////////////////////////

#include <stdlib.h>
#include <math.h>
#include "FreeRTOS.h"
#include "semphr.h"
#include "hpp.h"
#include "HPPDataGenerator.h"

extern SemaphoreHandle_t xHPP_Spike_Detect_Sem;

HPPDataGenerator::HPPDataGenerator()
{
	m_initialized = false;
}

HPPDataGenerator::~HPPDataGenerator()
{
}

int HPPDataGenerator::InitHPPDataGenerator(int ttl_output_bitnum = 4)
{
	u8 portnum = 0;
	u8 bitnum = 0;
	u8 status = 0;

	//Initialize HPP with required control messages
	IdentifyDIOParams(ttl_output_bitnum, &portnum, &bitnum);

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

	for(portnum = 0; portnum < 4; portnum++)
	{
		//Assert HPP control over MB TTL lines
		status = SetDIOCtrl(portnum, HPP_Ctrl);
		if(status != 0)
		{
			xil_printf("\n\rProblem with SetDigitalIOCtrl(%d, HPP_Ctrl), Status = %d", portnum, status);
		}

		//Set TTLs to outputs
		status = SetDIOPortDir(portnum, DIO_PORT_DIR_OUTPUT);
		if(status != 0)
		{
			xil_printf("\n\rProblem with SetDigitalIOPortDir(%d, DIO_PORT_DIR_OUTPUT), Status = %d", portnum, status);
		}
	}

	SetPSReady(PS_Ctrl);

	xil_printf("\n\rHPP Data generator successfully initialized.\n\r");

	//Assume LED Control
	status = SetFPLEDCtrl(HPP_Ctrl);
	if(status != 0)
	{
		xil_printf("\n\rProblem with SetFPLEDCtrl(LED_S1), Status = %d", status);
	}

	return 0;
}

void HPPDataGenerator::SetLEDOn(bool on)
{
	u8 status = 0;

	if (on) {
		//Toggle each LED once for verification of functionality before template matching
		status = SetFPLED(LED_S1, LED_On);
		if(status != 0)
		{
			xil_printf("\n\rProblem with SetFPLED(LED_S1, LED_On), Status = %d", status);
		}
	} else {
		status = SetFPLED(LED_S1, LED_Off);
		if(status != 0)
		{
			xil_printf("\n\rProblem with SetFPLED(LED_S1, LED_Off), Status = %d", status);
		}
	}
}

void HPPDataGenerator::GenerateSampleRecord(LRECORD *pLxRecord)
{
	u32 cur_index = 0;

	pLxRecord->header.packetId = 0;
	pLxRecord->header.timestampHigh = 0;
	pLxRecord->header.timestampLow = 0;
	pLxRecord->header.ttlIO = 0;
	pLxRecord->header.systemStatus = 1;

	if (m_generatePulse)
	{
		if (xHPP_Spike_Detect_Sem != NULL)
		{
			if (!m_initialized) {
				printf("Calling InitHPPDataGenerator\r\n");
				InitHPPDataGenerator(4);
				printf("InitHPPDataGenerator initialized\r\n");
				m_initialized = true;
			}

			if (xSemaphoreTake(xHPP_Spike_Detect_Sem, portMAX_DELAY) == pdTRUE)
			{
				if (num_data_buffers_loaded > 31)
				{
					cur_index = num_data_buffers_loaded & AVAILABLE_BUFFERS_MASK; // & is faster than %, so the mask is setup for &

					// Set time stamps
					pLxRecord->header.packetId = m_n;
					pLxRecord->header.timestampHigh = HPP_Data[cur_index].TimeStamp_High;
					pLxRecord->header.timestampLow = HPP_Data[cur_index].TimeStamp_Low;
					pLxRecord->header.ttlIO = HPP_Data[cur_index].TTL_Port_Values;
					pLxRecord->header.systemStatus = 0;

					// Collect HPP data channels 0-31 from DDR memory
					for (int j = 0; j < NUM_BOARDS; j++)
						for (int ch = 0; ch < NUM_CHANNELS; ch++)
							pLxRecord->board[j].data[ch] = HPP_Data[cur_index].AD[ch+(j*NUM_CHANNELS)];

					if (cur_index != (num_data_buffers_loaded & AVAILABLE_BUFFERS_MASK)) // & is faster than %, so the mask is setup for &
					{
						xil_printf("\n\rHPP Algorithm is too slow for real time...\n\r");
					}
					m_n++;
				}
			}
		}
	}
	AddCheckSum(pLxRecord);
}



