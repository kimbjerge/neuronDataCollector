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

extern SemaphoreHandle_t xHPP_Data_Sem;

//static bool mHPPInitialized = false; // Global variable to ensure InitHPPDataGenerator only called once

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

	//KBE??? if (mHPPInitialized) return 1;

	//Initialize HPP with required control messages
	IdentifyDIOParams(ttl_output_bitnum, &portnum, &bitnum);

	//Assert PS control (as opposed to fabric control) over the commands sent to the motherboard
	status = SetPSControl(PS_Ctrl);
	if(status != 0)
	{
		xil_printf("\n\rProblem with SetPSControl(PS_Ctrl)");
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

	//xil_printf("\n\rHPP Data generator successfully initialized.\n\r");

	//Assume LED Control
	status = SetFPLEDCtrl(HPP_Ctrl);
	if(status != 0)
	{
		xil_printf("\n\rProblem with SetFPLEDCtrl(LED_S1), Status = %d", status);
	}

	//mHPPInitialized = true;
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

void HPPDataGenerator::CopySampleRecord(LRECORD *pLxRecord, u32 cur_index)
{
	// Set time stamps
	pLxRecord->header.start = 0x5A5A5A5A; // Start Pattern
	pLxRecord->header.packetId = m_n;
	pLxRecord->header.size = 32; // Number of channels
	pLxRecord->header.timestampHigh = HPP_Data[cur_index].TimeStamp_High;
	pLxRecord->header.timestampLow = HPP_Data[cur_index].TimeStamp_Low;
	pLxRecord->header.ttlIO = HPP_Data[cur_index].TTL_Port_Values;
	pLxRecord->header.systemStatus = 1;

	// Collect HPP data channels 0-31 from DDR memory
	//memcpy(&(pLxRecord->board[0].data[0]), &(HPP_Data[cur_index].AD[0]), NUM_CHANNELS*sizeof(int32_t));

	//for (int j = 0; j < NUM_BOARDS; j++)
	for (int ch = 0; ch < NUM_CHANNELS; ch++)
		pLxRecord->board[0].data[ch] = HPP_Data[cur_index].AD[ch];

	m_n++;
}

void HPPDataGenerator::GenerateSampleRecord(LRECORD *pLxRecord)
{
	u32 cur_index = 0;

	if (!m_initialized) {
		printf("Calling InitHPPDataGenerator\r\n");
		InitHPPDataGenerator(4);
		printf("Interface to SX motherboard initialized\r\n");
		m_MissedSamples = 0;
		m_initialized = true;
		last_cur_index = num_data_buffers_loaded;
	}

	if ((last_cur_index+3) <= num_data_buffers_loaded) // & is faster than %, so the mask is setup for &
	{ // Data left in sample buffer - read next sample
		last_cur_index++;
		cur_index = last_cur_index & AVAILABLE_BUFFERS_MASK; // & is faster than %, so the mask is setup for &
		CopySampleRecord(pLxRecord, cur_index);
		m_MissedSamples++;
		if (m_MissedSamples%10000 == 0) // Print message for each 10000 buffered samples
			xil_printf("Samples %d/%d\n\r", m_MissedSamples, m_n);
	} else { // Wait for new sample
		if (xSemaphoreTake(xHPP_Data_Sem, portMAX_DELAY) == pdTRUE)
		{
			last_cur_index = num_data_buffers_loaded-1;
			cur_index = last_cur_index & AVAILABLE_BUFFERS_MASK; // & is faster than %, so the mask is setup for &
			CopySampleRecord(pLxRecord, cur_index);
		}
	}

	AddCheckSum(pLxRecord);
}



