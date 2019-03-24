#pragma once
#include "stdafx.h"
#include <string.h>
#include "SOCK_WrapperFacade.h"

#define BLOCK_SIZE 1024
#define MSG_SIZE 100

class FileTransfer
{
public:
	FileTransfer(void);
	~FileTransfer(void);
	int sendFile(SOCK_Stream* stream, char *FileCmd);

private:
	char fileBuffer[BLOCK_SIZE];
	char msgBuffer[MSG_SIZE];
};

