///////////////////////////////////////////////////////////
//  TestDataGenerator.h
//  Implementation of the Class TestDataGenerator
//  Created on:      24-maj-2017 11:01:53
//  Original author: Kim Bjerge
///////////////////////////////////////////////////////////

#if !defined(TESTDATAGENERATOR_INCLUDED_)
#define TESTDATAGENERATOR_INCLUDED_

#include "NeuronData.h"

class TestDataGenerator : public NeuronData
{

public:
	TestDataGenerator();
	virtual ~TestDataGenerator();

	virtual void GenerateSampleRecord(LRECORD *pLxRecord);

protected:
	int32_t GenSine(int channel);

	double m_omega;
};

#endif // !defined(TESTDATAGENERATOR_INCLUDED_)
