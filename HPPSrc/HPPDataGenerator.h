/*
 * HPPDataGenerator.h
 *
 *  Created on: 11. dec. 2018
 *      Author: au288681
 */

#ifndef SRC_HPPDATAGENERATOR_H_
#define SRC_HPPDATAGENERATOR_H_


#include "NeuronData.h"

class HPPDataGenerator : public NeuronData
{

public:
	HPPDataGenerator();
	virtual ~HPPDataGenerator();

	virtual void GenerateSampleRecord(LRECORD *pLxRecord);
	virtual void reset(void) { m_initialized = false; m_n = 0; m_MissedSamples = 0; }

	int InitHPPDataGenerator(int ttl_output_bitnum);
	void SetLEDOn(bool on);

protected:
	void CopySampleRecord(LRECORD *pLxRecord, u32 idx);
	bool m_initialized;
	u32 last_cur_index;
};


#endif /* SRC_HPPDATAGENERATOR_H_ */
