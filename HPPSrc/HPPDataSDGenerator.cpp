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
	pTestDataSDCard = pDataSDCard;
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
					nextSamples = pTestDataSDCard->GenerateSamples(); // Use samples from SD card instead of HPP

					if (cur_index != (num_data_buffers_loaded & AVAILABLE_BUFFERS_MASK)) // & is faster than %, so the mask is setup for &
					{
						xil_printf("\n\rHPP Algorithm is too slow for real time...\n\r");
					}
				}
			}
		}
	}

	return nextSamples;
}


