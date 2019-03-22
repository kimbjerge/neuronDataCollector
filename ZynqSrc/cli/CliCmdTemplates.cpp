/*
 * CliCmdTemplates.cpp
 *
 *  Created on: 21/03/2019
 *      Author: kbe
 */
#include "CliCmdTemplates.h"
#include "TestDataSDCard.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "Gpio.h"
#include "Switch.h"

void TestFileSDCard(void);

#define CMD_DELIMITER    ","

CliCommand::CliCommand(TemplateMatch *pTemplateMatch, DataUDPThread *pDataThread) : m_file((char *)"0:/")
{
	m_pTemplateMatch = pTemplateMatch;
	m_pDataThread = pDataThread;
	m_numSamples = MAX_NUM_SAMPLES; // Default 60 seconds
	majorVer_ = VERSION_HI;
	minorVer_ = VERSION_LO;
	m_fileSize = 0;
	m_executeMode = 1;
}

//------------------------------------------------------------------------------------------------
// Parsing Commands
// -----------------------------------------------------------------------------------------------
int CliCommand::parseTemplate(int *nr, int *W, int *L)
{
	int w, l, ok = 0;
	char *nrStr, *WStr, *LStr, *dStr;
	nrStr = strtok(NULL, CMD_DELIMITER);
	WStr = strtok(NULL, CMD_DELIMITER);
	LStr = strtok(NULL, CMD_DELIMITER);
	if ((nrStr != 0) && (WStr != 0) && (LStr != 0)) {
		*nr = atoi(nrStr);
		w = atoi(WStr);
		*W = w;
		l = atoi(LStr);
		*L = l;
		memset(mTemplate, 0, sizeof(mTemplate));
		ok = 1;
		for (int i = 0; i < w*l && i < TEMP_SIZE; i++) {
			dStr = strtok(NULL, CMD_DELIMITER);
			if (dStr != 0)
				mTemplate[i] = atof(dStr);
			else {
				printf("Insufficient data for template %d of size %d*%d end %d\r\n", *nr, w, l, i);
				ok = 0;
				break;
			}
		}
	}
	return ok;
}

int CliCommand::parseShortArray(int *nr)
{
	int ok = 0;
	char *nrStr, *dStr;
	nrStr = strtok(NULL, CMD_DELIMITER);
	if (nrStr != 0) {
		*nr = atoi(nrStr);
		memset(mShortArray, 0, sizeof(mShortArray));
		ok = 1;
		for (int i = 0; i < TEMP_WIDTH; i++) {
			dStr = strtok(NULL, CMD_DELIMITER);
			if (dStr != 0)
				mShortArray[i] = (short)atoi(dStr);
			else
				break;
		}
	}
	return ok;
}

int CliCommand::parseStrCmd2(char *name, int *value)
{
	int ok = 0;
	char *idStr, *valueStr;
	idStr = strtok(NULL, CMD_DELIMITER);
	valueStr = strtok(NULL, CMD_DELIMITER);
	if ((idStr != 0) && (valueStr != 0)) {
		strcpy(name, idStr);
		*value = atoi(valueStr);
		ok = 1;
	}
	return ok;
}

int CliCommand::parseCmd2(int *id, int *value)
{
	int ok = 0;
	char *idStr, *valueStr;
	idStr = strtok(NULL, CMD_DELIMITER);
	valueStr = strtok(NULL, CMD_DELIMITER);
	if ((idStr != 0) && (valueStr != 0)) {
		*id = atoi(idStr);
		*value = atoi(valueStr);
		ok = 1;
	}
	return ok;
}

int CliCommand::parseCmd2(int *id, float *value)
{
	int ok = 0;
	char *idStr, *valueStr;
	idStr = strtok(NULL, CMD_DELIMITER);
	valueStr = strtok(NULL, CMD_DELIMITER);
	if ((idStr != 0) && (valueStr != 0)) {
		*id = atoi(idStr);
		*value = atof(valueStr);
		ok = 1;
	}
	return ok;
}

int CliCommand::parseCmd1(int *value)
{
	int ok = 0;
	char *valueStr;
	valueStr = strtok(NULL, CMD_DELIMITER);
	if (valueStr != 0) {
		*value = atoi(valueStr);
		ok = 1;
	}
	return ok;
}

int CliCommand::openFile(char *name)
{
	int result;

	result = m_file.mount();
	if (result != XST_SUCCESS) printf("Failed to mount SD card\r\n");

	// Create a new file
	result = m_file.open(name, FA_CREATE_ALWAYS | FA_WRITE);
	if (result != XST_SUCCESS) printf("Failed open file for writing\r\n");

	return result;
}

int CliCommand::writeToFile(char *data, int len)
{
	int result;

	if (m_fileSize > len) {
		m_fileSize -= len;
	} else {
		m_fileSize = 0;
	}

	// Write to file
	result = m_file.write((void *)data, len);
	if (result != XST_SUCCESS) {
		printf("Failed writing to file\r\n");
		m_file.close();
	}

	if (m_fileSize == 0) {
		result = m_file.close();
		if (result != XST_SUCCESS)
			printf("Failed closing file\r\n");
	}

	return result;
}

bool CliCommand::checkNr(int nr)
{
	return (nr > 0 && nr <= TEMP_NUM);
}

int CliCommand::setParameter(char *paramStr, char *answer)
{
	int value,nr,W,L;
	float floatValue;
	char *param = strtok(paramStr, CMD_DELIMITER);
	int ok = 0;

	if (param != NULL) {

		switch (param[0]) {

			case 'd': // Update template data
				if (parseTemplate(&nr, &W, &L)) {
					if (checkNr(nr)) {
						m_pTemplateMatch->updateTemplateData(nr-1, mTemplate, L, W);
						printf("Template %d of size %d*%d updated\n", nr, W, L);
						ok = 1;
					}
				}
				break;

			case 'e': // Set duration of experiment in seconds
				if (parseCmd1(&value)) {
					m_numSamples = value*30000; // Sample rate = 30 kHz
					printf("Duration of experiment set to %d sec. processing %d samples\n", value, m_numSamples);
					ok = 1;
				}
				break;

			case 'g': // Set gradient for template
				if (parseCmd2(&nr, &value)) {
					if (checkNr(nr)) {
						m_pTemplateMatch->getConfig()->setMinGradient(nr-1, value);
						printf("Template %d gradient set to %d\n", nr, value);
						ok = 1;
					}
				}
				break;

			case 'h': // Set peak high limits
				if (parseShortArray(&nr)) {
					if (checkNr(nr)) {
						m_pTemplateMatch->getConfig()->setPeakMaxLimits(nr-1, mShortArray);
						printf("Template %d peak high limits updated [%d, %d, %d, %d..]\n",
								nr, mShortArray[0], mShortArray[1], mShortArray[2], mShortArray[3]);
						ok = 1;
					}
				}
				break;

			case 'l': // Set peak low limits
				if (parseShortArray(&nr)) {
					if (checkNr(nr)) {
						m_pTemplateMatch->getConfig()->setPeakMinLimits(nr-1, mShortArray);
						printf("Template %d peak low limits updated [%d, %d, %d, %d..]\n",
								nr, mShortArray[0], mShortArray[1], mShortArray[2], mShortArray[3]);
						ok = 1;
					}
				}
				break;

			case 'm': // Set channel mapping
				if (parseShortArray(&nr)) {
					if (checkNr(nr)) {
						m_pTemplateMatch->getConfig()->setChannelMap(nr-1, mShortArray);
						printf("Template %d channel map updated [%d, %d, %d, %d..]\n",
								nr, mShortArray[0], mShortArray[1], mShortArray[2], mShortArray[3]);
						ok = 1;
					}
				}
				break;

			case 'n': // Set number of template to use
				if (parseCmd1(&value)) {
					if (0 < value && value <= TEMP_NUM) {
						m_pTemplateMatch->getConfig()->setNumTemplates(value);
						printf("Number of templates to use %d\n", value);
						ok = 1;
					}
				}
				break;

			case 'p': // Set processing mode
				if (parseCmd1(&value)) {
					if (0 <= value && value <= 2) {
						printf("Processing mode %d\n", value);
						m_executeMode = value;
						ok = 1;
					}
				}
				break;

			case 't': // Set threshold for template
				if (parseCmd2(&nr, &floatValue)) {
					if (checkNr(nr)) {
						m_pTemplateMatch->getConfig()->setThreshold(nr-1, floatValue);
						printf("Template %d threshold set to %f\n", nr, floatValue);
						ok = 1;
					}
				}
				break;

			case 'u': // Upload file name of file size
				if (parseStrCmd2(m_fileName, &m_fileSize)) {
					if (strlen(m_fileName) < 13 && m_fileSize < 1048577) { // Filenames max. 8 chars + extension 4 and max. 1 Mbyte
						printf("Upload file %s of size %d\n", m_fileName, m_fileSize);
						if (openFile(m_fileName) == XST_SUCCESS)
							ok = 1;
					}  else {
						printf("Invalid file %s of size %d\n", m_fileName, m_fileSize);
						m_fileSize = 0;
					}
				}
				break;

		}
	}

	// Return answer to set command
	if (ok) strcpy(answer, "s,ok\n");
	else strcpy(answer, "s,error\n");
	return strlen(answer)+1;
}

int CliCommand::getParameter(char *paramStr, char *answer)
{
	int value;
	char *param = strtok(paramStr, CMD_DELIMITER);
	int ok = 0;
	Switch sw;

	switch (param[0]) {

		case 'c': // Read configuration
			m_pTemplateMatch->updateConfig(m_numSamples);
			m_pTemplateMatch->printSettings();
			strcpy(answer, "Printed on USB-UART\n");
			ok = 1;
			break;


		case 's': // Read switch settings on ZedBoard
			//value = IO::getIOInst()->readSwitches();
			value = sw.readio();
			printf("Switches 0x%02X\n", value);
			sprintf(answer, "Switches,0x%02X\n", value);
			ok = 1;
			break;

		case 'v': // Read version number
			printf("Software version number %d.%d\n", majorVer_, minorVer_);
			sprintf(answer, "Version,%d.%d\n", majorVer_, minorVer_);
			ok = 1;
			break;

	}

	// Return answer to get command
	if (!ok) strcpy(answer, "g,error\n");
	return strlen(answer)+1;
}

//------------------------------------------------------------------------------------------------
// Parse and execute commands
// -----------------------------------------------------------------------------------------------
int CliCommand::execute(char *cmd, char *pAnswer, int len)
{
	int length = len;
	//TimeMeasure time;

	if (m_fileSize > 0) {
		// Handling of file transfer
		if (writeToFile(cmd, len) == XST_SUCCESS) {
			printf("File written to SD card\r\n");
			strcpy(pAnswer, "ok\n");
			length = 4;
		} else {
			m_fileSize = 0;
			strcpy(pAnswer, "error\n");
			length = 7;
		}
	} else {

		switch (cmd[0]) {

			case 's': // Set parameter
				length = setParameter(&cmd[1], pAnswer);
				break;

			case 'g': // Get parameter
				length = getParameter(&cmd[1], pAnswer);
				break;

			case 'b': // Start UDP stream of neuron samples
				//m_pDataThread->runThread(Thread::PRIORITY_NORMAL, "DataUDPThread"); //KBE?? not blocking priority
				if (m_executeMode == 0)
					m_pDataThread->runThread(Thread::PRIORITY_ABOVE_NORMAL, "DataUDPThread");
				else {
					m_pTemplateMatch->updateConfig(m_numSamples);
					m_pTemplateMatch->runThread(Thread::PRIORITY_ABOVE_NORMAL, "TemplateMatch");
				}
				strcpy(pAnswer, "ok\n");
				length = 4;
				break;

			case 'e': // Stop UDP Stream of neuron samples
				if (m_executeMode == 0)
					m_pDataThread->setStreaming(false);
				else
					m_pTemplateMatch->stopRunning();
				strcpy(pAnswer, "ok\n");
				length = 4;
				break;

			case '?':
				length = printCommands();
				//printf(commandsText);
				strcpy(pAnswer, commandsText);
				break;
		}

	}

	return length;
}

int CliCommand::printCommands(void)
{
	char string[200];
	commandsText[0] = 0;

	sprintf(string, "\r\nNeuron Data Collector Version %d.%d:\r\n", majorVer_, minorVer_);
	strcat(commandsText, string);
	sprintf(string, "-----------------------------------\r\n");
	strcat(commandsText, string);
	sprintf(string, "? - display this command menu\r\n");
	strcat(commandsText, string);
	sprintf(string, "b - begin processing neuron samples\r\n");
	strcat(commandsText, string);
	sprintf(string, "e - end processing neuron samples\r\n");
	strcat(commandsText, string);

	sprintf(string, "\r\nSet(s)/Get(g) parameters:\r\n");
	strcat(commandsText, string);
	sprintf(string, "-------------------------\r\n");
	strcat(commandsText, string);

	sprintf(string, "s,d,<nr>,<W>,<L>,<d1>,<d2>..<dN> - update template (1-6) of size N=W*L with flattered data d1..dN using floats (dX=12.1234) \r\n");
	strcat(commandsText, string);
	sprintf(string, "s,e,<sec> - set duration of experiment in seconds\r\n");
	strcat(commandsText, string);
	sprintf(string, "s,g,<nr>,<grad> - set gradient for template (1-6) where min. peak and peak(n-4) must be greater than <grad> for all channels in match\r\n");
	strcat(commandsText, string);
	sprintf(string, "s,h,<nr>,<h0>,<h1>,<h2>..<h8> - set template (1-6) peak high limits for mapped channels (h0-h8)\r\n");
	strcat(commandsText, string);
	sprintf(string, "s,l,<nr>,<l0>,<l1>,<l2>..<l8> - set template (1-6) peak low limits for mapped channels (l0-l8)\r\n");
	strcat(commandsText, string);
	sprintf(string, "s,m,<nr>,<c0>,<c1>,<c2>..<c8> - set template (1-6) channel mapping to neuron channels (cX = 0-31)\r\n");
	strcat(commandsText, string);
	sprintf(string, "s,n,<num> - set number (1-6) of templates to be used\r\n");
	strcat(commandsText, string);
	sprintf(string, "s,p,<mode> - set processing mode: (0) transmit UDP samples to computer, (1) real-time TTL trigger of activations, (2) SD Card\r\n");
	strcat(commandsText, string);
	sprintf(string, "s,t,<nr>,<thres> - set threshold for template (1-6) used to trigger neuron activation using normalized cross correlation (NXCOR)\r\n");
	strcat(commandsText, string);
	sprintf(string, "s,u,<filename>,<size> - upload file to SD card of size in bytes\r\n");
	strcat(commandsText, string);

	sprintf(string, "g,c - read configuration for template matching using NXCOR\r\n");
	strcat(commandsText, string);
	sprintf(string, "g,s - read switch settings only on ZedBoard\r\n");
	strcat(commandsText, string);
	sprintf(string, "g,v - read software version\r\n");
	strcat(commandsText, string);

	return strlen(commandsText)+1;
}
