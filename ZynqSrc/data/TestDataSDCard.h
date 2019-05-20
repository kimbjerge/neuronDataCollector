///////////////////////////////////////////////////////////
//  TestDataSDCard.h
//  Implementation of the Class TestDataSDCard
//  Created on:      7-december-2018 11:01:53
//  Original author: Kim Bjerge
///////////////////////////////////////////////////////////

#if !defined(TESTDATA_SDCARD_INCLUDED_)
#define TESTDATA_SDCARD_INCLUDED_

#include "Semaphore.h"
#include "Thread.h"
using namespace AbstractOS;
#include "NeuronData.h"
#include "FileSDCard.h"
#include "Gpio.h"
#include "Leds.h"
#include "Switch.h"

#define MAX_NUM_SAMPLES 				60*30000   // Maximum 60 Seconds of samples
//#define NUM_SAMPLES 				30000   // 1 Seconds of samples
#define BLOCK_SAMPLES               1000

class TestDataSDCard;

// Helper thread to store collected data on SD Card
class StoreDataSDCard : public Thread
{
public:
	StoreDataSDCard(TestDataSDCard *pDataSDCard) {
		mpCollector = pDataSDCard;
	}
	virtual ~StoreDataSDCard() {};

	// Thread to collect and save data in the "HPPDATA.BIN" file
	virtual void run();
private:
	TestDataSDCard *mpCollector;

};

class TestDataSDCard : public NeuronData, public Thread
{
friend StoreDataSDCard;
public:
	TestDataSDCard();
	virtual ~TestDataSDCard();

	int readFile(char *name);
	virtual void GenerateSampleRecord(LRECORD *pLxRecord);
	virtual int16_t *GenerateSamples(void);
	int getNumSamples(void) { return mNumDataSamples; }
	void resetDataBuffer(void) { m_pWriteData = &(m_data[0][0]); mNumDataSamples = 0; }
	void appendDataSamples(float *pData, int length);

	// Thread to collect data and save in "HPPDATA.BIN" file
	virtual void run();
	void setNumSamplesCollect(int num) { mNumSamplesCollect = num; }
	void stopRunning(void) { mCounter = 0; }
	void stopAndKill(void) { stopRunning(); mStoreDataThread.kill(); kill(); m_file.close(); mRunning = false; };
	bool isRunning(void) { return mRunning; };
	// Callback function to generate sample data must be assigned before running thread
	void setFuncToGenSamples( int16_t* (*Func)(void)) { mFuncGenSamples = Func; };

protected:
	// Used by Thread and helper thread to store collected sample data on SD Card (run)
    Leds leds;
    Switch sw;
    bool mRunning;
	int mCounter;
	int mNumSamplesCollect; // Number of data samples to collect
	int mIdxBlock;
	int16_t m_collectData[BLOCK_SAMPLES*2][NUM_CHANNELS];
	int16_t* (*mFuncGenSamples)(void);
	Semaphore mSemaNewBuffer;
	StoreDataSDCard mStoreDataThread;

	// Used by reading from SD Card
    FileSDCard m_file;
    // Sample buffer read from file
    int mNumDataSamples;
    short mScaleBits;
    float *m_pWriteData;
    //float m_testData[NUM_CHANNELS];
	float m_data[MAX_NUM_SAMPLES][NUM_CHANNELS];
};


#endif // !defined(TESTDATA_SDCARD_INCLUDED_)

