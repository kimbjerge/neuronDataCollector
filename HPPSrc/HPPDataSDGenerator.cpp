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

extern SemaphoreHandle_t xHPP_Spike_Detect_Sem;

HPPDataSDGenerator::HPPDataSDGenerator(TestDataSDCard *pDataSDCard)
{
	m_pTestDataSDCard = pDataSDCard;
	m_dataFromSDCard = true;
	m_pCliCommand = 0;
}

HPPDataSDGenerator::~HPPDataSDGenerator()
{
}

int16_t *HPPDataSDGenerator::GenerateSamples(void)
{
	u32 cur_index = 0;
	int16_t* nextSamples = 0;

	if (m_generatePulse)
	{
		if (xHPP_Spike_Detect_Sem != NULL)
		{
			if (!m_initialized) {
				//printf("Calling InitHPPDataGenerator\r\n");
				InitHPPDataGenerator(4);
				xil_printf("Interface to SX motherboard initialized\r\n");
				m_initialized = true;
				//SetLEDOn(true); // Turn LED S1 on
			}
			if (xSemaphoreTake(xHPP_Spike_Detect_Sem, portMAX_DELAY) == pdTRUE)
			{
				if (num_data_buffers_loaded > 31)
				{
					cur_index = num_data_buffers_loaded & AVAILABLE_BUFFERS_MASK; // & is faster than %, so the mask is setup for &
					if (m_pCliCommand != 0)
						m_dataFromSDCard = (m_pCliCommand->getMode() == 2); // Mode 2 - used data from SD card
					if (m_dataFromSDCard) {
		 				// Data from SD card
						nextSamples = m_pTestDataSDCard->GenerateSamples(); // Use samples from SD card instead of HPP
					} else {
						// Data from Digital Lynx
						for (int ch = 0; ch < NUM_CHANNELS; ch++)
							m_Samples[ch] = HPP_Data[cur_index].AD[ch];
						nextSamples = (int16_t *)m_Samples;
					}

					if (cur_index != (num_data_buffers_loaded & AVAILABLE_BUFFERS_MASK)) // & is faster than %, so the mask is setup for &
					{
						xil_printf("!!!HPP Algorithm is too slow for real time!!!\n\r");
					}
				}
			}
		}
	}

	return nextSamples;
}


