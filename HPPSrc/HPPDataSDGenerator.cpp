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
#include "HPPDataSDGenerator.h"

extern SemaphoreHandle_t xHPP_Data_Sem;

HPPDataSDGenerator::HPPDataSDGenerator(TestDataSDCard *pDataSDCard)
{
	m_pTestDataSDCard = pDataSDCard;
	m_pCliCommand = 0;
}

HPPDataSDGenerator::~HPPDataSDGenerator()
{
}

int16_t *HPPDataSDGenerator::GenerateSamples(void)
{
	u32 cur_index = 0;
	int16_t* nextSamples = 0;

	if (!m_initialized) {
		//printf("Calling InitHPPDataGenerator\r\n");
		InitHPPDataGenerator(4);
		printf("Interface to SX motherboard initialized\r\n");
		m_initialized = true;
		m_MissedSamples = 0;
		last_cur_index = num_data_buffers_loaded;
		//SetLEDOn(true); // Turn LED S1 on
	}

	if ((last_cur_index+3) <= num_data_buffers_loaded) // & is faster than %, so the mask is setup for &
	{
		m_MissedSamples++;
		xil_printf("!!Missed %d/%d!!\n\r", last_cur_index, num_data_buffers_loaded);
		//xil_printf("!!Missed sample!!\n\r");
	}

	if (xSemaphoreTake(xHPP_Data_Sem, portMAX_DELAY) == pdTRUE)
	{
		last_cur_index = num_data_buffers_loaded-1;
		cur_index = last_cur_index & AVAILABLE_BUFFERS_MASK; // & is faster than %, so the mask is setup for &

		if (m_pCliCommand->getMode() == 2) { // Mode 2 - used data from SD card
			// Data from SD card
			nextSamples = m_pTestDataSDCard->GenerateSamples(); // Use samples from SD card instead of HPP
		} else {
			// Data from Digital Lynx
			for (int ch = 0; ch < NUM_CHANNELS; ch++) {

#if VERSION_LO == 0
				//m_Samples[ch] = HPP_Data[cur_index].AD[ch] >> 2; // Truncate and remove 2 bits of precision
				m_Samples[ch] = (HPP_Data[cur_index].AD[ch]+2) >> 2; // Round and remove 2 bits of precision V.3.0 Max. values are +/-132.000 (18 bit)
#else
				m_Samples[ch] = HPP_Data[cur_index].AD[ch]; //  Version 3.1 and above - don't scale input
#endif

			}
			m_TimeStampHigh = HPP_Data[cur_index].TimeStamp_High;
			m_TimeStampLow = HPP_Data[cur_index].TimeStamp_Low;
			nextSamples = m_Samples;
		}
	}

	return nextSamples;
}

