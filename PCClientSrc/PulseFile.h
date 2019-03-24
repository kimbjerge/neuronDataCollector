#pragma once
#include "stdafx.h"
#include <string.h>
#include "SOCK_WrapperFacade.h"

class PulseFile
{
public:
	PulseFile(void);
	~PulseFile(void);
	void DumpPulseEventsAsText(char *FileName, short *pPulseEvent, int size);
	void DumpAudioStreamAsText(char *FileName, int *pAudioStream, int size);
	void DumpTimestampsAsText(char *FileName, ULONG64 *pTimeStamps, int size);
	void SavePulseParameters(char *FileName, char *parameters, int len);
	int ReadPulseFile(char *FileName, short *pPulseBuf, int len);
};

