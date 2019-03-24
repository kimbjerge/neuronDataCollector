#include "FileTransfer.h"

FileTransfer::FileTransfer(void)
{
}


FileTransfer::~FileTransfer(void)
{
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
				printf("%s-%06d\r", msgBuffer, --blockCnt);
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