/*
 * CliCmdTemplates.h
 *
 *  Created on: 21/03/2017
 *      Author: kbe
 */

#ifndef CLICOMMAND_H_
#define CLICOMMAND_H_

#include "TemplateMatch.h"
#include "DataUDPThread.h"
#include "FileSDCard.h"
#include "TestDataSDCard.h"

#define VERSION_HI		1
#define VERSION_LO		7
#define CMD_BUF_SIZE    4096

class CliCommand {
public:
	CliCommand(TemplateMatch *pTemplateMatch, DataUDPThread *pDataThread, TestDataSDCard *pTestDataSDCard = 0);
	int execute(char *cmd, char *pAnswer, int len);
	int printCommands(void);
	int getMode(void) { return m_executeMode; }
private:
	TemplateMatch *m_pTemplateMatch;
	DataUDPThread *m_pDataThread;
	TestDataSDCard *m_pTestDataSDCard;
	int parseTemplate(int *nr, int *W, int *L);
	int parseShortArray(int *nr);
	int parseStrCmd2(char *name, int *value);
	int parseCmd2(int *id, int *value);
	int parseCmd2(int *id, float *value);
	int parseCmd1(int *value);
	int okAnswer(char *pAnswer);
	int errorAnswer(char *pAnswer);
	int setParameter(char *paramStr, char *answer);
	int getParameter(char *paramStr, char *answer);
	int openFile(char *name);
	int writeToFile(char *data, int len);
	bool writeToSampleData(char *data, int len);
	bool checkNr(int nr);
	int majorVer_;
	int minorVer_;
	int m_dataSize;
	int m_fileSize;
	int m_executeMode; // 0 = UDP transfer data, 1 = Template matching, 2 = Template matching from SD card
	int m_numSamples;
	int m_blockCnt;
	char m_fileName[50];
	char commandsText[CMD_BUF_SIZE];
	FileSDCard m_file;
    float mTemplate[TEMP_SIZE];
    short mShortArray[TEMP_WIDTH];
};

#endif /* AUDIOCOMMAND_H_ */
