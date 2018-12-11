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

	int InitHPPDataGenerator(int ttl_output_bitnum);

private:
	bool m_initialized;
};


#endif /* SRC_HPPDATAGENERATOR_H_ */
