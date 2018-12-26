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

#define NUM_SAMPLES 				30*30000   // 30 Minutes of samples

class TestDataSDCard : public NeuronData
{

public:
	TestDataSDCard();
	virtual ~TestDataSDCard();

	int readFile(char *name);
	virtual void GenerateSampleRecord(LRECORD *pLxRecord);

protected:
    FileSDCard m_file;
    // Sample buffer read from file
	float m_data[NUM_CHANNELS][NUM_SAMPLES];
};

#endif // !defined(TESTDATA_SDCARD_INCLUDED_)
