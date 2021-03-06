///////////////////////////////////////////////////////////
//  TestDataGenerator.cpp
//  Implementation of the Class TestDataGenerator
//  Created on:      24-maj-2017 11:01:53
//  Original author: Kim Bjerge
///////////////////////////////////////////////////////////

#include <stdlib.h>
#include <math.h>
#include "TestDataGenerator.h"

#define FC     300 //Hz
#define NOISE  40  //amplitude

TestDataGenerator::TestDataGenerator()
{
	const double fs = 30000;
	const double fc = FC;
	m_omega = 2 * 3.14159265359 * fc / fs; 
}

TestDataGenerator::~TestDataGenerator()
{

}

int32_t TestDataGenerator::GenSine(int channel)
{
	double sample = (100+(channel*10))*sin(m_omega*m_n);
	return int32_t(round(sample));
}

void TestDataGenerator::GenerateSampleRecord(LRECORD *pLxRecord)
{
	pLxRecord->header.packetId = 1;
	pLxRecord->header.timestampHigh = 1;
	pLxRecord->header.timestampLow = 2;
	pLxRecord->header.ttlIO = 0;
	pLxRecord->header.systemStatus = 0;

	if (m_generatePulse) {

		// Generate pulse
		for (int j = 0; j < NUM_BOARDS; j++)
			for (int i = 0; i < NUM_CHANNELS; i++)
				pLxRecord->board[j].data[i] = GenSine(NUM_CHANNELS*j + i) + (NOISE * rand() / RAND_MAX);
		m_n++;
	}
	else {
		m_n = 0;
		// Generate random
		for (int j = 0; j < NUM_BOARDS; j++)
			for (int i = 0; i < NUM_CHANNELS; i++)
				pLxRecord->board[j].data[i] = NOISE * rand() / RAND_MAX;
				//pLxRecord->board[j].data[i] = 10 * rand() / RAND_MAX;
	}
	AddCheckSum(pLxRecord);
}
