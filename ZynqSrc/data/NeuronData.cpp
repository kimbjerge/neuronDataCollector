///////////////////////////////////////////////////////////
//  NeuronData.cpp
//  Implementation of the Class NeuronData
//  Created on:      7-december-2018 11:01:53
//  Original author: Kim Bjerge
///////////////////////////////////////////////////////////

#include "NeuronData.h"

NeuronData::NeuronData()
{
	m_generatePulse = true;
	m_n = 0;
}

NeuronData::~NeuronData()
{

}

int16_t *NeuronData::GenerateSamples(void)
{
	for (int i = 0; i < NUM_CHANNELS; i++)
		nextDataSamples[i] = 0;
	m_n++;
	return nextDataSamples;
}

// Computes and verifies checksum of record
void NeuronData::AddCheckSum(LRECORD *pLxRecord)
{
	uint32_t *pStart = (uint32_t *)pLxRecord;
	uint32_t checksum = 0;
	for (int i = 0; i < (int)(sizeof(LxRecord)/sizeof(uint32_t))-1; i++)
		checksum = checksum ^ pStart[i];
	pLxRecord->checksum = checksum;
}

void NeuronData::GenerateSampleRecord(LRECORD *pLxRecord)
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
				pLxRecord->board[j].data[i] = 0;
		}
		m_n++;
	} else
		m_n = 0;

	AddCheckSum(pLxRecord);
}

