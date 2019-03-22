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

	void setFromSDCard (bool fromSDCard) { m_dataFromSDCard = fromSDCard; };
	void addCliCommand(CliCommand *pCliCommand) { m_pCliCommand = pCliCommand; };

private:
	TestDataSDCard *m_pTestDataSDCard;
	CliCommand *m_pCliCommand;
	int16_t m_Samples[NUM_CHANNELS];
	bool m_dataFromSDCard;
};


#endif /* SRC_HPPDATASDGENERATOR_H_ */
