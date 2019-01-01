///////////////////////////////////////////////////////////
//  TestDataSDCard.h
//  Implementation of the Class TestDataSDCard
//  Created on:      7-december-2018 11:01:53
//  Original author: Kim Bjerge
///////////////////////////////////////////////////////////

#if !defined(TESTDATA_SDCARD_INCLUDED_)
#define TESTDATA_SDCARD_INCLUDED_

#include "NeuronData.h"
#include "FileSDCard.h"

#define MAX_NUM_SAMPLES 				60*30000   // Maximum 60 Seconds of samples
//#define NUM_SAMPLES 				30000   // 1 Seconds of samples

class TestDataSDCard : public NeuronData
{

public:
	TestDataSDCard();
	virtual ~TestDataSDCard();

	int readFile(char *name);
	virtual void GenerateSampleRecord(LRECORD *pLxRecord);
	int getNumSamples(void) { return mNumDataSamples; }

protected:
    FileSDCard m_file;
    // Sample buffer read from file
    int mNumDataSamples;
	float m_data[MAX_NUM_SAMPLES][NUM_CHANNELS];
};

#endif // !defined(TESTDATA_SDCARD_INCLUDED_)

