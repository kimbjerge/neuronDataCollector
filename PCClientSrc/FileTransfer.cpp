#include "FileTransfer.h"

FileTransfer::FileTransfer(void)
{
}


FileTransfer::~FileTransfer(void)
{
}

int FileTransfer::receiveFile(SOCK_Stream* stream, char *FileCmd)
{
	int size, wrBytes, bytes, blockCnt;
	int total = 0;
	FILE *fp = NULL;
	char FileName[20];

	strcpy(FileName, &FileCmd[4]); // Extract file name from command

	fp = fopen(FileName, "wb"); // Open file for binary write
	if (fp == NULL)
	{
		printf("Could not open file %s for writing\r\n", FileName);
		return total;
	}

	// Create and send file command
	sprintf(msgBuffer, "%s\r", FileCmd);
	stream->send_n(msgBuffer, strlen(msgBuffer), 0);
	memset(msgBuffer, 0, sizeof(msgBuffer));
	// Wait for reply
	Sleep(100); // Delay 100 ms (NON BLOCKING socket)
	bytes = stream->recv_n(msgBuffer, MSG_SIZE, 0);
	printf(msgBuffer); // Print reply
	if (strncmp(msgBuffer, "f,ok", 4)) {
		printf("No reply from board\r\n");
		size = 0; // Check ok reply
	} else {
		size = atoi(&msgBuffer[5]);
		if (size <= 0) {
			printf("Invalid file size %d\r\n", size);
			size = 0; // Check ok reply
		}
	}

	stream->setBlocking(true);
	blockCnt = size / DATA_SIZE;
	while (size > 0) {

		// Send reply when still more data to transfer
		sprintf(msgBuffer, "f,ok,%d\r", size);
		stream->send_n(msgBuffer, strlen(msgBuffer), 0);

		// Wait for file data block from board
		bytes = stream->recv_n(dataBuffer, DATA_SIZE, 0);
		if (bytes > 0) {
			printf("%06d-%06d x%dkBytes \r",  ++total, --blockCnt, DATA_SIZE / 1024);
			// Write received data buffer to file
			wrBytes = fwrite(dataBuffer, 1, bytes, fp);
			if (wrBytes != bytes)
				printf("Error writing to file %s block size %d/%d\r\n", FileName, wrBytes, bytes);
			if (size >= bytes) // More data to transfer
				size -= bytes;
			else
				size = 0;
		}
		else {
			printf("Error receiving file %s - blocks transfered %d\r\n", FileName, total);
			size = 0;
		}
	}

	stream->setBlocking(false);
	fclose(fp); // Close file

	return total;
}

int FileTransfer::sendFile(SOCK_Stream* stream, char *FileCmd)
{
	int size, send,  bytes, blockCnt, endPos;
	int total = 0;
	FILE *fp = NULL;
	char FileName[20];

	strcpy(FileName, &FileCmd[4]); // Extract file name from command
	
	fp = fopen(FileName, "rb"); // Open file for binary read
	if (fp == NULL)
	{
		printf("Could not open file %s\r\n", FileName);
		return total;
	}
	// Get file size
	fseek(fp, 0, SEEK_END); // End of file
	size = ftell(fp); // Get size of file
	fseek(fp, 0, SEEK_SET); // Start of file

	if (size > 0) {
		// Create and send file command
		sprintf(msgBuffer, "%s,%d\r", FileCmd, size);
		send = stream->send_n(msgBuffer, strlen(msgBuffer), 0);
		memset(msgBuffer, 0, sizeof(msgBuffer));
		// Wait for reply
		Sleep(100); // Delay 100 ms (NON BLOCKING socket)
		bytes = stream->recv_n(msgBuffer, MSG_SIZE, 0);
		printf(msgBuffer); // Print reply
		if (strncmp(msgBuffer, "f,ok", 4)) {
			printf("No reply from board\r\n");
			size = 0; // Check ok reply
		}
	}

	stream->setBlocking(true);
	blockCnt = size/ BLOCK_SIZE;
	while (size > 0) {
		// Read file contents
		size = fread(fileBuffer, 1, BLOCK_SIZE, fp);
		if (size > 0) {
			// Send file contents in blocks
			send = stream->send_n(fileBuffer, size, 0);
			//if (send != size)
			//	printf("Error sending file %s block size %d/%d\r\n", FileName, size, send);
			total += size;
			// Wait for block acknowledge
			bytes = stream->recv_n(msgBuffer, MSG_SIZE, 0);
			if (bytes > 0) {
				endPos = strlen(msgBuffer) - 1;
				msgBuffer[endPos] = 0; // Remove CR
				printf("%s-%06d kBytes \r", msgBuffer, --blockCnt);
			} else {
				printf("Error sending file %s - bytes transfered %d\r\n", FileName, total);
				break;
			}
		}
	}
	stream->setBlocking(false);
	fclose(fp); // Close file

	return total;
}