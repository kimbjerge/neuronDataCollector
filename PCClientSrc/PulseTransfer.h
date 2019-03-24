#pragma once
#include "stdafx.h"
#include <string.h>
#include "SOCK_WrapperFacade.h"
#include "AudioFormat.h"

class PulseTransfer
{
public:
	PulseTransfer(void);
	~PulseTransfer(void);
	int receivePulseEvents(SOCK_Stream* stream, short **ppPulseBuffer);
	int receivePulseEventsBlocking(SOCK_Stream* stream, short **ppPulseBuffer);
	int receiveTimestampsBlocking(SOCK_Stream* stream, ULONG64 *pPulseBuffer);
	int receivePulseParamBlocking(SOCK_Stream* stream, short *pPulseBuffer, int size);
    int receiveStreamBuffer(SOCK_Stream* stream, char *pStreamBuffer, int size);
	long int receiveAndStore(SOCK_Stream* stream, AudioFormat *audioFormat, long int size);

private:
	short pulseBuffer[PULSE_SIZE*4];
};

