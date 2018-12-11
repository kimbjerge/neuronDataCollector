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

	//params[0].portnum = portnum;
	//params[0].bitnum = bitnum;

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

	SetPSReady(PS_Ctrl);

	xil_printf("\n\rHPP Data generator successfully initialized.\n\r");

	return 0;
}

/* From Spike Detector Demo
int HPPDataGenerator::InitHPPDataGenerator(int ttl_output_bitnum = 4)
{
	u8	i = 0;
	u8 portnum = 0;
	u8 bitnum = 0;
	u8 status = 0;

	//Initialize HPP with required control messages
	IdentifyDIOParams(ttl_output_bitnum, &portnum, &bitnum);

	//params[0].portnum = portnum;
	//params[0].bitnum = bitnum;

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

	//Added for Cheetah Debug
	//Assert HPP control over MB TTL lines
	status = SetDIOCtrl(1, HPP_Ctrl);
	if(status != 0)
	{
		xil_printf("\n\rProblem with SetDigitalIOCtrl(%d, HPP_Ctrl), Status = %d", portnum, status);
	}
	//Set TTLs to outputs
	status = SetDIOPortDir(1, DIO_PORT_DIR_OUTPUT);
	if(status != 0)
	{
		xil_printf("\n\rProblem with SetDigitalIOPortDir(%d, DIO_PORT_DIR_OUTPUT), Status = %d", portnum, status);
	}

	//For SfN Demos
	for(i = 0; i < 4; i++)
	{
		//Assert HPP control over MB TTL lines
		status = SetDIOCtrl(i, HPP_Ctrl);
		if(status != 0)
		{
			xil_printf("\n\rProblem with SetDigitalIOCtrl(%d, HPP_Ctrl), Status = %d", i, status);
		}

		//Set TTLs to outputs
		status = SetDIOPortDir(i, DIO_PORT_DIR_OUTPUT);
		if(status != 0)
		{
			xil_printf("\n\rProblem with SetDigitalIOPortDir(%d, DIO_PORT_DIR_OUTPUT), Status = %d", i, status);
		}
	}
	xil_printf("\n\rAll digital I/O ports setup with HPP control and as outputs\n\r");

	SetPSReady(PS_Ctrl);

	return 0;
}
*/

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
				InitHPPDataGenerator(4);
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



