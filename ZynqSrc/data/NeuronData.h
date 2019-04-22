//  NeuronData.h
//  Implementation of the Class NeuronData
//  Created on:      7-december-2018 11:01:53
//  Original author: Kim Bjerge
///////////////////////////////////////////////////////////

#if !defined(NEURON_DATA_INCLUDED_)
#define NEURON_DATA_INCLUDED_

#include "LxRecord.h"

class NeuronData
{

public:
	NeuronData();
	virtual ~NeuronData();

	void SetPulseActive(bool generate) {
		m_generatePulse = generate;
	}

	virtual void reset(void) { m_n = 0; }
	virtual void GenerateSampleRecord(LRECORD *pLxRecord);
	virtual int16_t *GenerateSamples(void);
	virtual void GetTimeStamp(uint32_t *pHigh, uint32_t *pLow);

protected:
	void AddCheckSum(LRECORD *pLxRecord);
	bool m_generatePulse;
	uint32_t m_n;
	int16_t nextDataSamples[NUM_CHANNELS];
	uint32_t m_TimeStampHigh;
	uint32_t m_TimeStampLow;
};

#endif // !defined(NEURON_DATA_INCLUDED_)
