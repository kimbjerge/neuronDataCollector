/*
 * CliCommand.h
 *
 *  Created on: 31/03/2015
 *      Author: kbe
 */

#ifndef CLICOMMAND_H_
#define CLICOMMAND_H_

#include "NeuronChannels.h"
#include "DataUDPThread.h"

#define VERSION_HI		1
#define VERSION_LO		6
#define CMD_BUF_SIZE    4096

class CliCommand {
public:
	CliCommand(NeuronChannels *pNeuronChannels, DataUDPThread *pDataThread);
	int execute(char *cmd, char *pAnswer, int len);
	int printCommands(void);
private:
	NeuronChannels *m_pNeuronChannels;
	DataUDPThread *m_pDataThread;
	int parseCmd2(int *id, int *value);
	int parseCmd1(int *value);
	int setParameter(char *paramStr, char *answer);
	int getParameter(char *paramStr, char *answer);
	int majorVer_;
	int minorVer_;
	char commandsText[CMD_BUF_SIZE];
};

#endif /* AUDIOCOMMAND_H_ */
