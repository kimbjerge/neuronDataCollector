///////////////////////////////////////////////////////////
//  TestDataSDCard.cpp
//  Implementation of the Class TestDataSDCard
//  Created on:      7-december-2018 11:01:53
//  Original author: Kim Bjerge
///////////////////////////////////////////////////////////
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "TestDataSDCard.h"

TestDataSDCard::TestDataSDCard() : m_file((char *)"0:/")
{
}

TestDataSDCard::~TestDataSDCard()
{

}

int TestDataSDCard::readFile(char * name)
{
	int result;
	result = m_file.mount();
	if (result != XST_SUCCESS) printf("Failed to mount SD card\r\n");

	result = m_file.open(name, FA_OPEN_EXISTING | FA_READ);
	if (result != XST_SUCCESS) printf("Failed open file %s for reading\r\n", name);

	if (result == XST_SUCCESS) {
		result = m_file.read((void *)m_data, sizeof(m_data));
		if (result != XST_SUCCESS) printf("Failed reading from file %s\r\n", name);

		result = m_file.close();
		if (result != XST_SUCCESS) printf("Failed closing file %s\r\n", name);
	}
	return result;
}

void TestDataSDCard::GenerateSampleRecord(LRECORD *pLxRecord)
{
	pLxRecord->header.packetId = 1;
	pLxRecord->header.timestampHigh = 1;
	pLxRecord->header.timestampLow = 2;
	pLxRecord->header.ttlIO = 0;
	pLxRecord->header.systemStatus = 0;

	if (m_generatePulse) {

		// Generate pulse
		for (int j = 0; j < NUM_BOARDS; j++) {
			for (int i = 0; i < NUM_CHANNELS; i++)
				pLxRecord->board[j].data[i] = (int32_t)m_data[m_n][i];
		}
		if (++m_n >= NUM_SAMPLES) // Turn around sample buffer
			m_n = 0;

	} else
		m_n = 0;
	AddCheckSum(pLxRecord);
}

