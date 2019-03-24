#pragma once
#include "stdafx.h"
#include <string.h>
#include "SOCK_WrapperFacade.h"
#include "PulseFile.h"
#include "PulseTransfer.h"
#include "PulseParameters.h"

#define PULSE_BLOCK_SIZE 512
#define BLOCK_HEADER	 6
#define MSG_LEN          100

class PulseCollector
{
public:
	PulseCollector(void);
	~PulseCollector(void);

	void CollectEnergyHistogram(SOCK_Stream *pStream, int count, int triggers, int channels);
	void CollectRTLPulseParamters(SOCK_Stream *pStream, int channels, int count);
	void ContinueCollectPulses(SOCK_Stream *pStream, int count, int channels);

	int SendTestPulse(SOCK_Stream *pStream, short *pulseSamples, int samples);
	void TestPulseEnergy(SOCK_Stream *pStream, char *name);

private:
	void SendCmd(SOCK_Stream *pStream, char *cmd);

	short pulseBuffer[PULSE_SIZE*4];
	ULONG64 timeBuffer[PULSE_SIZE*4];
	char pulseBlock[BLOCK_HEADER + PULSE_BLOCK_SIZE*2];
	int dirTPENumber; // Directory number to store pulse data
	int dirTEHNumber; // Directory number to store pulse history data
	PulseFile  plsFile;
	PulseTransfer plsTransfer;
	PulseParameters plsChannels[NUM_CHANNELS];
};

