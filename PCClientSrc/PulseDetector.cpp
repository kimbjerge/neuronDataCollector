// PulseDetector.cpp : Defines the entry point for the console application.

#include "stdafx.h"
#include <string.h>
#include "SOCK_WrapperFacade.h"
#include "PulseCollector.h"
#include "AudioFormat.h"
#include "FileTransfer.h"

#define MAX_RECORD_TIME         240         // Maxium recording time in seconds (4 minutes)
//#define MAX_RECORD_TIME         180         // Maxium recording time in seconds (3 minutes)
#define DEFAULT_PORT			7			// ZedBoard socket service port 
#define IP_ADDR					0xC0A8010A	// 192.168.1.10

// Possible commands
#define CMD_PULSE_CE            "ce"  // Collect number of pulse events from channels
#define CMD_PULSE_CP            "cp"  // Collect pulse parameters from channels (Digital RTL Pulse Detector)
#define CMD_PULSE_CH            "ch"  // Collect pulse history (Digital SW Pulse Detector)
#define CMD_PULSE_TE            "te"  // Sending pulse for ennergy measure 

#define CMD_PULSE_READ          'r'
#define CMD_PULSE_READ_CH       'c'
#define CMD_AUDIO_STREAM        '+'
#define CMD_QUIT                'q'
#define CMD_RESTART             'w'

// Audio Recorder
#define CMD_SET_RATE            "s,r,%d"
#define CMD_SET_SKIP_DOWN       "s,d,%d"
#define CMD_SET_SKIP_AVG        "s,a,%d"
#define CMD_SET_CHANNELS        "s,c,%d"

// File commands
#define CMD_FILE_UPLOAD         "f,u"
#define CMD_FILE_DOWNLOAD       "f,w"

PulseCollector plsCollector;
PulseFile plsFile;
PulseTransfer plsTransfer;
AudioFormat audioFormat;
FileTransfer fileTransfer;

int sampleRes = 32;
//int sampleFreq = 192000;
//int sampleFreq = 768000;
int sampleFreq = 1536000;
int skipDown = 0; // Downsample and average
int skipAvg = 0; // Downsample by skipping samples in MERGE IP Core
int channels = 1; // Number of channels

int createWaveFile();

int calcSampleFreq(void)
{
	return  (sampleFreq / (skipAvg + 1)) / (skipDown + 1);
}

void updateAudioFormat(void)
{
	int frequency = calcSampleFreq();
	printf("Set wave file sample rate to %d Hz, channels %d (ConvFs=%d,Sa=%d,Sd=%d)\r\n", frequency, channels, sampleFreq, skipAvg, skipDown);
	audioFormat.SetFormat(frequency, sampleRes, channels);
}

int _tmain(int argc, char* argv[])
{	
	bool finish = false;
	//short *pPulseEvents;
	unsigned long address = IP_ADDR;

	int sampleBytes = sampleRes/8;
	int byteSizeSec = channels * sampleFreq * sampleBytes;
	int size = MAX_RECORD_TIME * byteSizeSec; // Allocate memory for sample stream

#if 0
	char *streamBuffer = (char *)malloc(size);
	if (streamBuffer == NULL) {
		printf("Could not allocate memory of %d bytes\n", size);
		return -1;
	}
#endif

	audioFormat.SetFormat(sampleFreq, sampleRes, channels);

	/* FOR TESTING
	int fileSize = 2 * byteSizeSec;
	if (sampleRes == 32)
		audioFormat.Create32BitStereoSineWave(streamBuffer, fileSize);
	else
		audioFormat.Create16BitStereoSineWave(streamBuffer, fileSize);
	audioFormat.SaveWaveFile("SineWave.wav", streamBuffer, fileSize);
	//createWaveFile();
	*/

	// Convert server address to hexadecimal base.
	if (argc > 1) {
		printf("Connect to Neuron Analyzer: IP Address %s, Port 7\n", argv[1]);
		address = ntohl(inet_addr(argv[1]));
	} else
		printf("Connect to Neuron Analyzer: IP Address 192.168.1.10, Port 7\n");
	
	int pulseNr = 1;
	while (!finish) {

		// On Windows start socket 2.2 system
		WSADATA wsa_data;
		WORD version_requested = MAKEWORD(2, 2);
		int error = WSAStartup(version_requested, &wsa_data);
		if (error != 0) return -1;

		INET_Addr addr(DEFAULT_PORT, address);
		SOCK_Connector client(addr); 
		SOCK_Stream new_stream;

	    printf("-------------------------------------------------------------------------\n");
		printf("PC Client to setup and control Neuron Analyzer Version 2.0               \n");
		printf("-------------------------------------------------------------------------\n");
		
		char message[4000];

		client.connecting(new_stream);

		// Connected processing commands
		if (new_stream.get_handle() != 0) {
			bool restart = false;
			int pos;
			int bytes;
			//printf("ce <channels> <events> - Collects pulse events \n");
			//printf("cp <channels> <time> - Collects pulse parameters (RTL) \n");
			//printf("ch <channels> <events> - Collects pulse histograms (SW) \n");
			//printf("te <filname.txt> - Send pulse file for energy meassurment \n");
			printf("w - Reconnect to Neuron Analyser \n");
			printf("? - Display commands on Neuron Analyzer \n");
			printf("q - Quit \n");
			printf("\nClient connected to: ");
			strcpy(message, "Neuron Analyser (ZedBoard + HPP)");
			pos = strlen(message)+1;
			new_stream.send_n(message, pos, 0);
			new_stream.setBlocking(false);
			Sleep(200);

			while (!restart) {

				// Receive and print answer from board
				bytes = new_stream.recv_n(message, sizeof(message), 0);
				if (bytes > 0 && bytes < sizeof(message)) {
					message[bytes] = 0;
					printf(message);
				}
				printf("\n> ");

				// Input command for boad
				scanf("%s", message);
				
				if (!strncmp(message, CMD_FILE_UPLOAD, 3)) {
					// Special handling of file upload command
					fileTransfer.sendFile(&new_stream, message);
				}
				else if (!strncmp(message, CMD_FILE_DOWNLOAD, 3)) {
					// Special handling of file download command
					fileTransfer.receiveFile(&new_stream, message);
				} else {
					pos = strlen(message);
					// Send command to board
					message[pos++] = '\r';
					message[pos++] = 0;
					new_stream.send_n(message, pos, 0);
					Sleep(500); // Allow server to wakeup to receive and execute message

					// Process answer from board 
					switch (message[0]) {
					case CMD_QUIT:
						restart = true;
						finish = true;
						break;
					case CMD_RESTART:
						restart = true;
						break;
					}
				}
			}
			new_stream.close();
			client.close();
		}
		Sleep(3);
		WSACleanup();

	} // !finish

#if 0	
	free(streamBuffer);
#endif
    return 0;
}

/* NOT USED - From Audio Recorder and Pulse Detector
if (!strcmp(message, CMD_PULSE_CH))
{
	scanf("%d", &numChannels);
	scanf("%d", &numEvents);
	scanf("%d", &numTriggers);
	if (numChannels > 12) {
		numChannels = 12;
		printf("Maximum %d channels \r\n", numChannels);
	}
	if (numEvents < 1) numEvents = 1;
	if (numTriggers < 1) numTriggers = 1;
	if (numTriggers > 16) {
		numTriggers = 16; // Maximum space in ZedBoard for 8192 pulses, each trigger default collects 500 pulses
		printf("Maximum %d triggers \r\n", numTriggers);
	}
	// Collect pulse event samples in files for each 1-12 channels
	// repeat collecting file for numEvents*numTriggers*50 pulse triggers files named TPE_<numEvents>_<ch>.txt
	plsCollector.CollectEnergyHistogram(&new_stream, numEvents, numTriggers, numChannels);

} else if (!strcmp(message, CMD_PULSE_CE))
{
	scanf("%d", &numChannels);
	scanf("%d", &numEvents);
	if (numChannels > 12) {
		numChannels = 12;
		printf("Maximum %d channels \r\n", numChannels);
	}
	// Collect pulse event samples in files for each 1-12 channels
	// repeat collecting file for numEvents => numEvents*12 files named TPE_<num>_<ch>.txt
	plsCollector.ContinueCollectPulses(&new_stream, numEvents, numChannels);

} else if (!strcmp(message, CMD_PULSE_CP))
{
	scanf("%d", &numChannels);
	scanf("%d", &numEvents);
	if (numChannels > 12) {
		numChannels = 12;
		printf("Maximum %d channels \r\n", numChannels);
	}
	// Collect pulse event samples in files for each 1-12 channels
	// repeat collecting file for time set by burst size (g,b) files named TPE_<num>_<ch>.txt
	plsCollector.CollectRTLPulseParamters(&new_stream, numChannels, numEvents);

} else if (!strcmp(message, CMD_PULSE_TE))
{
	scanf("%s", &pulseName);
	plsCollector.TestPulseEnergy(&new_stream, pulseName);

} else { // Processing of single commands

	// Commands to change sample rate and number of channels
	if (!strncmp(message, CMD_SET_RATE, 4))
	{
		sscanf(message, CMD_SET_RATE, &sampleFreq);
		updateAudioFormat();
	}
	if (!strncmp(message, CMD_SET_SKIP_DOWN, 4))
	{
		sscanf(message, CMD_SET_SKIP_DOWN, &skipDown);
		updateAudioFormat();
	}
	if (!strncmp(message, CMD_SET_SKIP_AVG, 4))
	{
		sscanf(message, CMD_SET_SKIP_AVG, &skipAvg);
		updateAudioFormat();
	}
	if (!strncmp(message, CMD_SET_CHANNELS, 4))
	{
		sscanf(message, CMD_SET_CHANNELS, &channels);
		updateAudioFormat();
	}


	// Send command to board
	message[pos++] = '\r';
	message[pos++] = 0;
	new_stream.send_n(message, pos, 0);
	Sleep(500); // Allow server to wakeup to receive and execute message

	// Process answer from board
	switch (message[0]) {
		case CMD_PULSE_READ:
			// Specify max number of samples to receive
			samples = plsTransfer.receivePulseEvents(&new_stream, &pPulseEvents);
			sprintf(pulseName, "PE%02d.txt", pulseNr++);
			plsFile.DumpPulseEventsAsText(pulseName, pPulseEvents, samples);
			printf("Pulse event file %s created\n", pulseName);
			break;
		case CMD_PULSE_READ_CH:
			// Specify max number of samples to receive
			samples = plsTransfer.receivePulseEvents(&new_stream, &pPulseEvents);
			sprintf(pulseName, "PEC%c.txt", message[1]);
			plsFile.DumpPulseEventsAsText(pulseName, pPulseEvents, samples);
			printf("Pulse event file %s created\n", pulseName);
			break;
		case CMD_AUDIO_STREAM:
#if 1
							sprintf(pulseName, "AS%02d.wav", pulseNr++);
							printf("Creating audio file %s sampling at frequency %d Hz\n", pulseName, calcSampleFreq());
							audioFormat.SaveWaveHeader(pulseName);
							samples = plsTransfer.receiveAndStore(&new_stream, &audioFormat, (sampleBytes*calcSampleFreq()*channels*MAX_RECORD_TIME));
							printf("Received and stored audio wave file of size %d bytes, recorded duration %.2f sec\n", samples, (double)samples / (sampleBytes*calcSampleFreq()));
							audioFormat.SaveWaveTrailer();
#else
							samples = plsTransfer.receiveStreamBuffer(&new_stream, streamBuffer, size);
							sprintf(pulseName, "AS%02d.wav", pulseNr++);
							printf("Creating audio stream file %s of size %d, frequency %d, duration %.2f sec\n", pulseName, samples, calcSampleFreq(), (double)samples/(sampleBytes*calcSampleFreq()));
							//audioFormat.Gain32Bit((int *)streamBuffer, samples/sampleBytes, 200000); // For testing only
							audioFormat.SaveWaveFile(pulseName, streamBuffer, samples);
							break;
#endif
*/
