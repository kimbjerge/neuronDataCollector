#include "PulseTransfer.h"

PulseTransfer::PulseTransfer(void)
{
}


PulseTransfer::~PulseTransfer(void)
{
}

// Receive raw samples(short) of size, allocates buffer using malloc
int PulseTransfer::receivePulseEvents(SOCK_Stream* stream, short **ppPulseBuffer)
{
	int left, bytes, bsize = PULSE_SIZE*8, samples;
	*ppPulseBuffer = (short *)pulseBuffer;
	char *pBuffer = (char *)*ppPulseBuffer;

	left = bsize;
	bytes = 1;
	while (left > 0 &&  bytes > 0 && pBuffer != NULL)
	{
		bytes = stream->recv_n(pBuffer, left, 0);
		if (bytes > 0) {
			pBuffer += bytes;
			left -= bytes;
		} 
		Sleep(20);
	}
	samples = (bsize - left)/2;
	return samples;
}

int PulseTransfer::receivePulseEventsBlocking(SOCK_Stream* stream, short **ppPulseBuffer)
{
	int bytes, bsize = PULSE_SIZE*8, samples;
	*ppPulseBuffer = (short *)pulseBuffer;
	char *pBuffer = (char *)*ppPulseBuffer;

	bytes = stream->recv_n(pBuffer, bsize, 0);
	samples = bytes/2; // One sample is 2 bytes
	return samples;
}

int PulseTransfer::receivePulseParamBlocking(SOCK_Stream* stream, short *pPulseBuffer, int size)
{
	int bytes, samples, bsize = 2*size;

	bytes = stream->recv_n((char *)pPulseBuffer, bsize, 0);
	samples = bytes/2; // One sample is 2 bytes
	return samples;
}

int PulseTransfer::receiveTimestampsBlocking(SOCK_Stream* stream, ULONG64 *pPulseBuffer)
{
	int bytes, bsize = PULSE_SIZE*8, samples;
	char *pBuffer = (char *)pPulseBuffer;

	bytes = stream->recv_n(pBuffer, bsize, 0);
	samples = bytes/8; // One timestamp is 8 bytes
	return samples;
}

#define NUM_RETRIES 300

// Receive raw bytes of size, allocates buffer using malloc
int PulseTransfer::receiveStreamBuffer(SOCK_Stream* stream, char *pStreamBuffer, int size)
{
	int left, bytes, bsize = size, rxBytes;
	char *pBuffer = pStreamBuffer;
	int retries = NUM_RETRIES;

	left = bsize;
	rxBytes = 0;
	bytes = 1;
	//while (bytes > 0 && pBuffer != NULL)
	//while (left > 0 && pBuffer != NULL)
	while (left > 0 &&  bytes > 0 && pBuffer != NULL)
	{
		bytes = stream->recv_n(pBuffer, left, 0);
		if (bytes > 0) {
			pBuffer += bytes;
			left -= bytes;
			rxBytes += bytes;
			retries = NUM_RETRIES;
		} else {
			if (--retries > 0) {
				bytes = 1;
				Sleep(10);
			}
		}
	    Sleep(5);
	}
	return rxBytes;
}

#define BUFFER_SIZE 65536

long int PulseTransfer::receiveAndStore(SOCK_Stream* stream, AudioFormat *audioFormat, long int size)
{
	long int left, rxBytes;
	int bytes;
	char pBuffer[BUFFER_SIZE];
	int retries = NUM_RETRIES;

	left = size;
	rxBytes = 0;
	bytes = 1;
	while (left > 0 && bytes > 0)
	{
		bytes = stream->recv_n(pBuffer, BUFFER_SIZE, 0);
		if (bytes > 0) {
			audioFormat->SaveWaveBody(pBuffer, bytes);
			left -= bytes;
			rxBytes += bytes;
			retries = NUM_RETRIES;
		}
		else {
			if (--retries > 0) {
				Sleep(15); // Wait for data to be received
				bytes = 1;
			}
		}
	}
	return rxBytes;
}

