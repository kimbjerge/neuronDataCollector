#pragma once
#include "stdafx.h"
#include <string.h>
#include "SOCK_WrapperFacade.h"

#define BLOCK_SIZE 1024 // Block size used for upload of files
#define DATA_SIZE  4096 // Data block size used for download of files
#define MSG_SIZE 100

class FileTransfer
{
public:
	FileTransfer(void);
	~FileTransfer(void);
	int sendFile(SOCK_Stream* stream, char *FileCmd);
	int receiveFile(SOCK_Stream* stream, char *FileCmd);

private:
	char fileBuffer[BLOCK_SIZE];
	char msgBuffer[MSG_SIZE];
	char dataBuffer[DATA_SIZE];
};

