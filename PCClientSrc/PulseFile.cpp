#include "PulseFile.h"

PulseFile::PulseFile(void)
{
}


PulseFile::~PulseFile(void)
{
}

// Dumping pulse event samples in text file
void PulseFile::DumpPulseEventsAsText(char *FileName, short *pPulseEvent, int size)
{
	int sample;
	FILE *fp = NULL;
	fp = fopen(FileName, "wb");
    if (fp == NULL) 
	{
    	return;
    }

	for (int i = 0; i < size; i++)
	{
		sample = (int)pPulseEvent[i];
		fprintf(fp, "%d,\r\n", sample);
	}
	fflush(fp);
	fclose(fp);
}

// Dumping pulse event samples in text file
void PulseFile::DumpAudioStreamAsText(char *FileName, int *pAudioStream, int size)
{
	int sample;
	FILE *fp = NULL;
	fp = fopen(FileName, "wb");
    if (fp == NULL) 
	{
    	return;
    }

	for (int i = 0; i < size; i++)
	{
		sample = (int)pAudioStream[i];
		fprintf(fp, "%d,\r\n", sample);
	}
	fflush(fp);
	fclose(fp);
}

// Dumping pulse event samples in text file
void PulseFile::DumpTimestampsAsText(char *FileName, ULONG64 *pTimeStamps, int size)
{
	FILE *fp = NULL;
	fp = fopen(FileName, "wb");
    if (fp == NULL) 
	{
    	return;
    }

	for (int i = 0; i < size; i++)
	{
		fprintf(fp, "%llu,\r\n", pTimeStamps[i]);
	}
	fflush(fp);
	fclose(fp);
}

void PulseFile::SavePulseParameters(char *FileName, char *parameters, int len)
{
	FILE *fp = NULL;
	fp = fopen(FileName, "wb");
    if (fp == NULL) 
	{
    	return;
    }
	
	printf(parameters);

	fwrite(parameters, len, 1, fp);
	fflush(fp);
	fclose(fp);
}

int PulseFile::ReadPulseFile(char *FileName, short *pPulseBuf, int len)
{
	int idx = 0;
	FILE *fp = NULL;
	int sample;

	memset(pPulseBuf, 0, len*2);

	fp = fopen(FileName, "r");
    if (fp == NULL)  
	{
		printf("Error open pulse file %s\n", FileName);
		return 0;
    }
	
	while (idx < len) {
		if (fscanf(fp, "%d,\n", &sample) != EOF)
			pPulseBuf[idx++] = (short)sample;
		else 
			break;
	}
	fclose(fp);

	return idx;
}
