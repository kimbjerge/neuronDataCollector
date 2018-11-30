/*
 * CliCommand.cpp
 *
 *  Created on: 31/03/2015
 *      Author: kbe
 */
#include "CliCommand.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "Gpio.h"
#include "Switch.h"

#define CMD_DELIMITER    ","

CliCommand::CliCommand(NeuronChannels *pNeuronChannels, DataUDPThread *pDataThread)
{
	m_pNeuronChannels = pNeuronChannels;
	m_pDataThread = pDataThread;

	majorVer_ = VERSION_HI;
	minorVer_ = VERSION_LO;
}

//------------------------------------------------------------------------------------------------
// Parsing Commands
// -----------------------------------------------------------------------------------------------
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

int CliCommand::setParameter(char *paramStr, char *answer)
{
	int value;
	char *param = strtok(paramStr, CMD_DELIMITER);
	int ok = 0;
	//int sampleRate;

	if (param != NULL) {
		switch (param[0]) {

			case 'a': //  Set sample average, skipping each N samples
				if (parseCmd1(&value)) {
					printf("Set skip each %d sample and enabling average\n", value);
					m_pNeuronChannels->setAverage(value);
					ok = 1;
				}
				break;

			case 'b': //  Set sample block size
				if (parseCmd1(&value)) {
					printf("Set sample stream duration %d seconds\n", value);
					m_pNeuronChannels->setStreamSizeSec(value);
					ok = 1;
				}
				break;

			case 'c': //  Set number of channels to enable
				if (parseCmd1(&value)) {
					printf("Set set number of channels to enable %d\n", value);
					if (value == 1) {
						m_pNeuronChannels->enableChannel(0, true);  // Audio channel 1 = enabled
						m_pNeuronChannels->enableChannel(1, false); // Audio channel 2 = disabled
					} else {
						m_pNeuronChannels->enableChannel(0, true);  // Audio channel 1 = enabled
						m_pNeuronChannels->enableChannel(1, true);  // Audio channel 2 = enabled
					}
					ok = 1;
				}
				break;

			case 'd': //  Set down sampling by skipping each N samples
				if (parseCmd1(&value)) {
					printf("Set skip each %d sample by down sampling\n", value);
					m_pNeuronChannels->setDownSample(value);
					ok = 1;
				}
				break;

			case 'e': //  Enable/disable audio streaming
				if (parseCmd1(&value)) {
					printf("Enable ADC %d\n", value);
					m_pNeuronChannels->enable(value);
					ok = 1;
				}
				break;

			case 'm': //  Set audio streaming mode
				if (parseCmd1(&value)) {
					printf("Set audio stream mode %d\n", value);
					m_pNeuronChannels->setTestMode(value);
					ok = 1;
				}
				break;

			case 'r': //  Set sample rate
				if (parseCmd1(&value)) {
					printf("Set sample rate %d\n", value);
					m_pNeuronChannels->setSampleRate(value);
					ok = 1;
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
	//int sampleRate;

	switch (param[0]) {

		case 'a': // Read skip sample with average
			value = m_pNeuronChannels->getAverage();
			printf("Skip each %d sample and average\n", value);
			sprintf(answer, "SkipSampleAverage,%d\n", value);
			ok = 1;
			break;

		case 'b': //  Read sample block size
			value = m_pNeuronChannels->getStreamSizeSec();
			printf("Sample stream duration %d seconds\n", value);
			sprintf(answer, "SampleStreamSize,%d,%d\n", m_pNeuronChannels->getStreamSize(), value);
			ok = 1;
			break;

		case 'c': // Read number of channels disabled
			value = m_pNeuronChannels->getChannelsDisabled();
			printf("Channels disabled 0x%02x\n", value);
			sprintf(answer, "ChannelsDisabled,0x%02x\n", value);
			ok = 1;
			break;

		case 'd': // Read down sampling number of skipped samples
			value = m_pNeuronChannels->getDownSample();
			printf("Skip each %d sample - down sampling\n", value);
			sprintf(answer, "DownSample,%d\n", value);
			ok = 1;
			break;

		case 'e': //  Enable/disable audio streaming
			value = m_pNeuronChannels->getEnable();
			printf("Audio stream enabled %d\n", value);
			sprintf(answer, "AudioEnabled,%d\n", value);
			ok = 1;
			break;

		case 'm': //  Read audio stream mode
			value = m_pNeuronChannels->getTestMode();
			printf("Audio stream mode %d\n", value);
			sprintf(answer, "StreamMode,%d\n", value);
			ok = 1;
			break;

		case 'r': //  Read sample rate
			value = m_pNeuronChannels->getSampleRate();
			printf("Sample rate %d, skip %d average, skip %d down sample\n", value, m_pNeuronChannels->getAverage(), m_pNeuronChannels->getDownSample());
			sprintf(answer, "SampleRate,%d,%d,%d\n", value, m_pNeuronChannels->getAverage(), m_pNeuronChannels->getDownSample());
			ok = 1;
			break;

		case 's': //  Read switch settings on ZedBoard
			//value = IO::getIOInst()->readSwitches();
			value = sw.readio();
			printf("Switches 0x%02X\n", value);
			sprintf(answer, "Switches,0x%02X\n", value);
			ok = 1;
			break;

		case 'v': //  Read version number
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

	switch (cmd[0]) {

		case 's': // Set parameter
			length = setParameter(&cmd[1], pAnswer);
			break;

		case 'g': // Get parameter
			length = getParameter(&cmd[1], pAnswer);
			break;

		case 'b': // Start UDP stream of neuron samples
			m_pDataThread->runThread(Thread::PRIORITY_NORMAL, "DataUDPThread");
			strcpy(pAnswer, "ok\n");
			length = 4;
			break;

		case 'e': // Stop UDP Stream of neuron samples
			m_pDataThread->setStreaming(false);
			strcpy(pAnswer, "ok\n");
			length = 4;
			break;

		case '?':
			length = printCommands();
			printf(commandsText);
			strcpy(pAnswer, commandsText);
			break;
	}

	return length;
}

int CliCommand::printCommands(void)
{
	char string[200];
	commandsText[0] = 0;

	sprintf(string, "\n\rNeuron Data Collector Version %d.%d:\n\r", majorVer_, minorVer_);
	strcat(commandsText, string);
	sprintf(string, "-----------------------------------\n\r");
	strcat(commandsText, string);
	sprintf(string, "? - display this command menu\n\r");
	strcat(commandsText, string);
	sprintf(string, "b - begin UDP stream of neuron samples\n\r");
	strcat(commandsText, string);
	sprintf(string, "e - end UDP stream of neuron samples\n\r");
	strcat(commandsText, string);

	sprintf(string, "\n\rSet(s)/Get(g) parameters:\n\r");
	strcat(commandsText, string);
	sprintf(string, "-------------------------\n\r");
	strcat(commandsText, string);

	sprintf(string, "s,a,<samples> set number of samples to skip (average filter)\n\r");
	strcat(commandsText, string);
	sprintf(string, "g,a read number of skipped samples (average filter)\n\r");
	strcat(commandsText, string);

	sprintf(string, "s,b,<seconds> set stream sample duration\n\r");
	strcat(commandsText, string);
	sprintf(string, "g,b read stream sample size, duration\n\r");
	strcat(commandsText, string);

	sprintf(string, "s,c,<channels> set number of channels 1/2\n\r");
	strcat(commandsText, string);
	sprintf(string, "g,c read status for channels disabled\n\r");
	strcat(commandsText, string);

	sprintf(string, "s,d,<samples> set number of samples to skip (down sample)\n\r");
	strcat(commandsText, string);
	sprintf(string, "g,d read number of skipped samples (down sample)\n\r");
	strcat(commandsText, string);

	sprintf(string, "s,e,<start 1|0> enable sampling audio\n\r");
	strcat(commandsText, string);
	sprintf(string, "g,e read audio enable sampling\n\r");
	strcat(commandsText, string);

	sprintf(string, "s,m,<mode 0|1> set audio test mode\n\r");
	strcat(commandsText, string);
	sprintf(string, "g,m read audio test mode\n\r");
	strcat(commandsText, string);

	sprintf(string, "s,r,<rate> set sample rate before down sampling (Conv freq)\n\r");
	strcat(commandsText, string);
	sprintf(string, "g,r read sample rate after down sampling (g,a + g,d)\n\r");
	strcat(commandsText, string);

	sprintf(string, "g,s read switch settings on ZedBoard\n\r");
	strcat(commandsText, string);

	sprintf(string, "g,v read software version\n\r");
	strcat(commandsText, string);

	return strlen(commandsText)+1;
}
