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

TestDataSDCard::TestDataSDCard() :
    mSemaNewBuffer(1, 0, "DataSDCard"),
	mStoreDataThread(this),
	m_file((char *)"0:/"),
	mNumDataSamples(0)
{
	mNumSamplesCollect = MAX_NUM_SAMPLES;
	mFuncGenSamples = 0;
}

TestDataSDCard::~TestDataSDCard()
{

}

int TestDataSDCard::readFile(char * name)
{
	int result;
	unsigned int fileSize;
	result = m_file.mount();
	if (result != XST_SUCCESS) printf("Failed to mount SD card\r\n");

	fileSize = m_file.size(name);
	result = m_file.open(name, FA_OPEN_EXISTING | FA_READ);
	if (result != XST_SUCCESS) printf("Failed open file %s for reading\r\n", name);

	if (result == XST_SUCCESS) {
		if (fileSize > sizeof(m_data)) fileSize = sizeof(m_data);
		result = m_file.read((void *)m_data, sizeof(m_data));
		if (result != XST_SUCCESS) printf("Failed reading from file %s\r\n", name);

		mNumDataSamples = fileSize/(sizeof(float)*NUM_CHANNELS);
		float time = mNumDataSamples/30000; // Sample rate 30 kHz
		printf("%dx32 data samples found in %s equal to %0.3f seconds\r\n", mNumDataSamples, name, time); //

		result = m_file.close();
		if (result != XST_SUCCESS) printf("Failed closing file %s\r\n", name);
	}
	return result;
}

void TestDataSDCard::appendDataSamples(float *pData, int length)
{
	if (mNumDataSamples+length <= MAX_NUM_SAMPLES*NUM_CHANNELS)
	{
		//memcpy(m_testData, pData, NUM_CHANNELS*sizeof(float)); // Debug
		memcpy(m_pWriteData, pData, length*sizeof(float));
		m_pWriteData += length;
		mNumDataSamples += length;
	}
}

int16_t *TestDataSDCard::GenerateSamples(void)
{
	for (int i = 0; i < NUM_CHANNELS; i++)
		nextDataSamples[i] = (int16_t)m_data[m_n][i];

	if (++m_n >= mNumDataSamples) // Turn around sample buffer
		m_n = 0;

	return nextDataSamples;
}


void TestDataSDCard::GenerateSampleRecord(LRECORD *pLxRecord)
{
	pLxRecord->header.packetId = m_n;
	pLxRecord->header.timestampHigh = 1;
	pLxRecord->header.timestampLow = 2;
	pLxRecord->header.ttlIO = 0;
	pLxRecord->header.systemStatus = 1;

	if (m_generatePulse) {

		// Generate pulse
		for (int j = 0; j < NUM_BOARDS; j++) {
			for (int i = 0; i < NUM_CHANNELS; i++)
				pLxRecord->board[j].data[i] = (int32_t)m_data[m_n][i];
		}
		if (++m_n >= mNumDataSamples) // Turn around sample buffer
			m_n = 0;

	} else
		m_n = 0;

	AddCheckSum(pLxRecord);
}

void TestDataSDCard::run()
{
	int result, idx;
    int16_t *pSampleData;
    int start_tick, end_tick;
    string fileName("HPPDATA.BIN");

	mRunning = true;

    if (mFuncGenSamples == 0) {
		printf("Function to generate sample data not assigned\r\n");
		mRunning = false;
	}

	result = m_file.open((char *)fileName.c_str(), FA_CREATE_ALWAYS | FA_WRITE);
	if (result != XST_SUCCESS) {
		printf("Failed open file %s for reading\r\n", fileName.c_str());
		mRunning = false;
	}

	if (mRunning) {
		mStoreDataThread.runThread(Thread::PRIORITY_NORMAL, "StoreDataThread");
		Sleep(100); // Wait to started
	}

	while (mRunning)
	{
		printf("Turn SW0 on (ZedBoard) or start acquisition on Digital Lynx SX\r\n");
		while (!sw.isOn(Switch::SW0) && mRunning) {
			Sleep(100);
		}
		printf("Start collecting %d samples saved in file HPPDATA.BIN on SD Card\r\n", mNumSamplesCollect);

		leds.setOn(Leds::LED6, true);
		idx = 0;
        mCounter = mNumSamplesCollect;
		start_tick = xTaskGetTickCount();
		while (mCounter > 0) {

			// Get next sample from data generator
			pSampleData = mFuncGenSamples();
			memcpy(&m_collectData[idx][0], pSampleData, NUM_CHANNELS*sizeof(int16_t));
			idx++;

			if (idx%BLOCK_SAMPLES == 0) {
				mIdxBlock = 0; // First block full
				if (idx > BLOCK_SAMPLES) { // Second block full
					mIdxBlock = BLOCK_SAMPLES;
					idx = 0;
				}
				mSemaNewBuffer.signal();
			}

			mCounter--;
#ifdef ZEDBOARD_DEBUG
			vTaskDelay( pdMS_TO_TICKS( 0.03333 ) ); // Sample rate 30 kHz Ts=0.0333 ms
#endif
		}
		end_tick = xTaskGetTickCount();
		leds.setOn(Leds::LED6, false);

		if (mIdxBlock == 0)
			mIdxBlock = BLOCK_SAMPLES;
		else mIdxBlock = 0;
		mRunning = false;
		// Save last block
		mSemaNewBuffer.signal();

		Sleep(3000); // Wait 3 sec. for TestDataSDCard to terminate

		printf("Tick start %d and tick end %d, duration = %d ms\r\n", start_tick, end_tick, (1000*(end_tick-start_tick))/configTICK_RATE_HZ);

	} // while (mRunning)


    printf("Collecting data shutting down and exit from collecting data thread\r\n");
    vTaskDelete(NULL);
}

// Helper thread to save data on SD Card
void StoreDataSDCard::run()
{
	int result;
	int16_t *pData;

	printf("Thread running storing data on SD Card\r\n");

	while (mpCollector->mRunning)
	{
		mpCollector->mSemaNewBuffer.wait();
		xil_printf("%9d\r", mpCollector->mCounter); // Print number of samples collected

		// Save sample data block in collector file "HPPDATA.BIN"
		pData = &mpCollector->m_collectData[mpCollector->mIdxBlock][0];
		result = mpCollector->m_file.write((void*)pData, BLOCK_SAMPLES*NUM_CHANNELS*sizeof(int16_t), true);
		if (result != XST_SUCCESS) {
			printf("Failed writing to file %s\r\n", mpCollector->m_file.getFileName());
			mpCollector->mCounter = 0;
		}
	}

	result =  mpCollector->m_file.close();
	if (result != XST_SUCCESS)
		printf("Failed closing file %s\r\n",  mpCollector->m_file.getFileName());

	xil_printf("Data saved on SD Card exit from storing data thread\r\n");
    vTaskDelete(NULL);
}


