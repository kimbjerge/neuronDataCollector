/*
 * HPPDataGenerator.h
 *
 *  Created on: 11. dec. 2018
 *      Author: au288681
 */

#ifndef SRC_HPPDATASDGENERATOR_H_
#define SRC_HPPDATASDGENERATOR_H_


#include "HPPDataGenerator.h"
#include "TestDataSDCard.h"
#include "CliCmdTemplates.h"

class HPPDataSDGenerator : public HPPDataGenerator
{

public:
	HPPDataSDGenerator(TestDataSDCard *pDataSDCard);
	virtual ~HPPDataSDGenerator();

	virtual int16_t *GenerateSamples(void);
	virtual void reset(void) { m_initialized = false; m_n = 0; m_pTestDataSDCard->reset(); m_MissedSamples = 0; }

	void addCliCommand(CliCommand *pCliCommand) { m_pCliCommand = pCliCommand; };

private:
	TestDataSDCard *m_pTestDataSDCard;
	CliCommand *m_pCliCommand;
	int16_t m_Samples[NUM_CHANNELS];
	//int32_t m_SamplesInt[NUM_CHANNELS];
};


#endif /* SRC_HPPDATASDGENERATOR_H_ */
