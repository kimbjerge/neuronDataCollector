#include "PulseCollector.h"


PulseCollector::PulseCollector(void)
{
    dirTPENumber = 0;
	dirTEHNumber = 0;
}

PulseCollector::~PulseCollector(void)
{
}

void PulseCollector::SendCmd(SOCK_Stream *pStream, char *cmd)
{
	char message[MSG_LEN];
	int len;

	strcpy(message, cmd); 
	len = strlen(message)+1;
	pStream->send_n(message, len, 0);
}

void PulseCollector::CollectEnergyHistogram(SOCK_Stream *pStream, int count, int triggers, int channels)
{
	char pulseName[MSG_LEN];
	int iterations, cnt, bytes, samples, channel, pulseNr = 1;
	short *pPulseEvents;
	char message[MSG_LEN];
	char dirName[MSG_LEN];
	char systemCmd[MSG_LEN];
	char *pValue;

	// Move all old collected files to a directory TPE<nr>
	sprintf(dirName, "TEH%d", dirTEHNumber++);
	sprintf(systemCmd, "mkdir %s", dirName);
	system(systemCmd);
    sprintf(systemCmd, "move TEH*.txt %s", dirName);
	system(systemCmd);
    sprintf(systemCmd, "move TEI*.txt %s", dirName);
	printf("Moving old TEH/I*.txt files to directory .\\%s\r\n", dirName);
	system(systemCmd);

	pStream->setBlocking(true);

	SendCmd(pStream, "s,r\r"); // Clears rejected pulse counter
	bytes = pStream->recv_n(message, sizeof(message), 0);
	if (bytes > 0) {
		message[bytes] = 0;
		printf("Rejected cleared: %s", message); // Print answer from ZedBoard
	} else {
		printf("No answer from Pulse Detector (s,r)\r\n");
		pStream->setBlocking(false);
		return;
	}

	SendCmd(pStream, "g,n\r"); // Read number of interations
	bytes = pStream->recv_n(message, sizeof(message), 0);
	if (bytes > 0) {
		message[bytes] = 0;
		// Expected answer g,n,<value>
		pValue = strtok(message, ",");
		pValue = strtok(NULL, ",");
		iterations = atoi(pValue);
		printf("Number of pulse events in each iteration: %d\r\n", iterations); // Print answer from ZedBoard
	} else {
		printf("No answer from Pulse Detector (g,n)\r\n");
		pStream->setBlocking(false);
		return;
	}

	for (cnt = 1; cnt <= count; cnt++)
	{
		SendCmd(pStream, "s,h\r"); // Clears histogram
		bytes = pStream->recv_n(message, sizeof(message), 0);
		if (bytes > 0) {
			message[bytes] = 0;
			printf("Histogram cleared: %s", message); // Print answer from ZedBoard
		} else {
			printf("No answer from Pulse Detector (s,n)\r\n");
			break;
		}

		for (int i = 0; i < triggers; i++) {
			SendCmd(pStream, "h\r"); // Collects energy histogram on all channels 1-12
			bytes = pStream->recv_n(message, sizeof(message), 0);
			if (bytes > 0) {
				message[bytes] = 0;
				printf("Pulse event triggers %d %s", (i+1)*iterations, message); // Print answer from ZedBoard, histogram collected for 50 pulses
			} else {
				printf("\r\nNo answer from Pulse Detector (h)\r\n");
				break;
			}
		}
		if (bytes > 0) {

			for (channel = 1; channel <= channels; channel++)
			{
				sprintf(message, "b%d\r", channel); // Read binary energy histogram
				SendCmd(pStream, message);
				samples = plsTransfer.receivePulseEventsBlocking(pStream, &pPulseEvents); // Collect energy histogram
				if (samples > 1) {
					// Only save file if a pulse was received
					sprintf(pulseName, "TEH-%d-%d.txt", cnt, channel);
					plsFile.DumpPulseEventsAsText(pulseName, pPulseEvents, samples); // Write samples to text file
					printf("Energy histogram file %s created\n", pulseName);
				}
				sprintf(message, "i%d\r", channel); // Read binary timestamps
				SendCmd(pStream, message);
				samples = plsTransfer.receiveTimestampsBlocking(pStream, (ULONG64 *)timeBuffer); // Collect timestamps
				if (samples > 1) {
					// Only save file if a pulse was received
					sprintf(pulseName, "TEI-%d-%d.txt", cnt, channel);
					plsFile.DumpTimestampsAsText(pulseName, (ULONG64 *)timeBuffer, samples); // Write samples to text file
					printf("Timestamp file %s created\n", pulseName);
				}
			}
		}
	}

	pStream->setBlocking(false);

}

void PulseCollector::CollectRTLPulseParamters(SOCK_Stream *pStream, int channels, int count)
{
	char pulseName[MSG_LEN];
	int cnt, bytes, samples, channel, size, pulseNr = 1;
	short *pPulseEvents;
	char message[MSG_LEN];
	char dirName[MSG_LEN];
	char systemCmd[MSG_LEN];

	// Move all old collected files to a directory TPE<nr>
	sprintf(dirName, "TPE%d", dirTPENumber++);
	sprintf(systemCmd, "mkdir %s", dirName);
	system(systemCmd);
    sprintf(systemCmd, "move TPE*.txt %s", dirName);
	system(systemCmd);
 	printf("Moved old TPE*.txt files to directory .\\%s\r\n", dirName);

	pStream->setBlocking(true);

	SendCmd(pStream, "k\r"); // Start collecting pulse parameters
	bytes = pStream->recv_n(message, sizeof(message), 0);
	message[bytes] = 0;
	printf(message); // Print answer from ZedBoard
	if (bytes == 0) {
		printf("No answer from Pulse Detector (x)\r\n");
	} else {
		for (cnt = 1; cnt <= count; cnt++)
		{	
			Sleep(1); // Wait 1 ms for collecting pulse event parameters
			printf("%d\r", cnt);
			for (channel = 1; channel <= channels; channel++)
			{
				sprintf(message, "c%d\r", channel); // Read binary pulse event samples
				SendCmd(pStream, message);
				pPulseEvents = plsChannels[channel].getBlock(size); // Get next free block
				samples = plsTransfer.receivePulseParamBlocking(pStream, pPulseEvents, size); // Collect samples
				if (samples > 1) {
					printf("Block %d.%d\r\n", channel, cnt);
					if (samples != plsChannels[channel].getBlockSize())
						printf("Wrong pulse parameter block size received %d<>%d\n", samples, plsChannels[channel].getBlockSize());
					if (plsChannels[channel].incBlock() == 1) {
						pPulseEvents = plsChannels[channel].getBlocks(size);
						// Only save file if a pulse was received and pulse block buffer is full
						printf("%d, %d, %d, %d\n", pPulseEvents[0], pPulseEvents[1], pPulseEvents[2], pPulseEvents[3]);
						sprintf(pulseName, "TPE-%d-%d.txt", cnt, channel);
						plsFile.DumpPulseEventsAsText(pulseName, pPulseEvents, size); // Write samples to text file
						printf("Pulse event file %s created\n", pulseName);
						plsChannels[channel].clearBlocks();
					}
				}
			}
		}
		// Complete collecting pulse event parameters
		for (channel = 1; channel <= channels; channel++)
		{
			if (plsChannels[channel].getNumBlocks() > 0) 
			{ // Saves parametes left in pluse block buffers
				pPulseEvents = plsChannels[channel].getBlocks(size);
				sprintf(pulseName, "TPE-%d-%d.txt", cnt, channel);
				plsFile.DumpPulseEventsAsText(pulseName, pPulseEvents, size); // Write samples to text file
				printf("Pulse event file %s created\n", pulseName);
			}
			plsChannels[channel].clearBlocks();
		}
		SendCmd(pStream, "l\r"); // Stop collecting pulse parameters
		bytes = pStream->recv_n(message, sizeof(message), 0);
		message[bytes] = 0;
		printf(message); // Print answer from ZedBoard
	}

	pStream->setBlocking(false);

}

void PulseCollector::ContinueCollectPulses(SOCK_Stream *pStream, int count, int channels)
{
	char pulseName[MSG_LEN];
	int cnt, bytes, samples, channel, pulseNr = 1;
	short *pPulseEvents;
	char message[MSG_LEN];
	char dirName[MSG_LEN];
	char systemCmd[MSG_LEN];
	char parameters[1000];

	// Move all old collected files to a directory TPE<nr>
	sprintf(dirName, "TPE%d", dirTPENumber++);
	sprintf(systemCmd, "mkdir %s", dirName);
	system(systemCmd);
    sprintf(systemCmd, "move TPE*.txt %s", dirName);
	system(systemCmd);
    sprintf(systemCmd, "move TPP*.txt %s", dirName);
	system(systemCmd);
	printf("Moved old TPE/P*.txt files to directory .\\%s\r\n", dirName);

	pStream->setBlocking(true);
	for (cnt = 1; cnt <= count; cnt++)
	{
		//SendCmd(pStream, "z\r"); // Trigger pulse event on all channels 1-12
		SendCmd(pStream, "x\r"); // Trigger pulse event on all channels 1-12, using fast parallel version
		bytes = pStream->recv_n(message, sizeof(message), 0);
		if (bytes > 0) {
			message[bytes] = 0;
			printf(message); // Print answer from ZedBoard

			for (channel = 1; channel <= channels; channel++)
			{
				sprintf(message, "c%d\r", channel); // Read binary pulse event samples
				SendCmd(pStream, message);
				samples = plsTransfer.receivePulseEventsBlocking(pStream, &pPulseEvents); // Collect samples
				if (samples > 1) {
					// Only save file if a pulse was received
					sprintf(pulseName, "TPE-%d-%d.txt", cnt, channel);
					plsFile.DumpPulseEventsAsText(pulseName, pPulseEvents, samples); // Write samples to text file
					printf("Pulse event file %s created\n", pulseName);
					
					// Get pulse parameters containing baseline and energy
					sprintf(message, "g,p,%d\r", channel); // Get pulse parameters
					SendCmd(pStream, message);
					bytes = pStream->recv_n(parameters, sizeof(parameters), 0);
					if (bytes > 0) {
						sprintf(pulseName, "TPP-%d-%d.txt", cnt, channel);
						printf("Pulse parameter file %s created\n", pulseName);
						plsFile.SavePulseParameters(pulseName, parameters, bytes);
					}
				}
			}
		} else {
			printf("No answer from Pulse Detector (x)\r\n");
			break;
		}
	}
	pStream->setBlocking(false);
}

int PulseCollector::SendTestPulse(SOCK_Stream *pStream, short *pulseSamples, int samples)
{
	int bytes;
	char message[MSG_LEN];
	short *pStart = (short *)&pulseBlock[2];
	short *pLenght = (short *)&pulseBlock[4];
	short *pBlock = (short *)&pulseBlock[6];
	short blocks = samples/PULSE_BLOCK_SIZE + 1;

	pulseBlock[0] = 'w';
	pulseBlock[1] = 0;

	// Sending pulse blocks
	for (int i = 0; i < blocks; i++) {
		*pStart = i*PULSE_BLOCK_SIZE;
		if (i == blocks-1) // Last block
			*pLenght = samples%PULSE_BLOCK_SIZE;
		else
			*pLenght = PULSE_BLOCK_SIZE;
		memcpy(pBlock, &pulseSamples[PULSE_BLOCK_SIZE*i], PULSE_BLOCK_SIZE*2);
		//for (int j = 0; j < 40; j++)
		//	printf("%d, ", pBlock[j]);
		//printf("\n");
		pStream->send_n(pulseBlock, sizeof(pulseBlock), 0);
		bytes = pStream->recv_n(message, sizeof(message), 0);
		if (bytes < 1) {
			printf("Failed to transmit pulse test file to Pulse Detector\n");
			return 0;
		}
	}
	return 1;

}

void PulseCollector::TestPulseEnergy(SOCK_Stream *pStream, char *name)
{
	int samples, bytes;
	int channel = 1;
	short *pulseSamples = (short *)pulseBuffer;
	short *pPulseEvents;
	char message[MSG_LEN];
	char parameters[1000];
	char pulseName[MSG_LEN];

	samples = plsFile.ReadPulseFile(name, pulseSamples, PULSE_SIZE*4);
	//samples = ReadPulseFile(name, pulseSamples, 12130);
	//printf("[%d] %d, %d, %d, %d, %d, %d\n", samples,
	//			pulseSamples[0],
	//			pulseSamples[1],
	//			pulseSamples[2],
	//			pulseSamples[3],
	//			pulseSamples[4],
	//			pulseSamples[5]);
	
	if (samples > 0)
	{
		pStream->setBlocking(true);

		if (SendTestPulse(pStream, pulseSamples, samples)) {

			SendCmd(pStream, "e\r"); // Execute energy command to test DTS energy  
			bytes = pStream->recv_n(message, sizeof(message), 0);
			if (bytes > 0) {
				message[bytes] = 0;
				printf("%s", message); // Print answer from ZedBoard
			}

			sprintf(message, "c%d\r", channel); // Read binary pulse event samples
			SendCmd(pStream, message);

			samples = plsTransfer.receivePulseEventsBlocking(pStream, &pPulseEvents); // Collect samples
			if (samples > 1) {
				// Only save file if a pulse was received
				sprintf(pulseName, "TPE-T-%d.txt", channel);
				plsFile.DumpPulseEventsAsText(pulseName, pPulseEvents, samples); // Write samples to text file
				printf("Pulse event file %s created\n", pulseName);
					
				// Get pulse parameters containing baseline and energy
				sprintf(message, "g,p,%d\r", channel); // Get pulse parameters
				SendCmd(pStream, message);

				bytes = pStream->recv_n(parameters, sizeof(parameters), 0);
				if (bytes > 0) {
					sprintf(pulseName, "PPE-T-%d.txt", channel);
					plsFile.SavePulseParameters(pulseName, parameters, bytes);
				}
			}
		}

		pStream->setBlocking(false);
	}

}
