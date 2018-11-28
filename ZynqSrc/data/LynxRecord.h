/*********************************************
** Digital Lynx SX, UDP Record format handler
** Created: 3/4 2017 by Kim Bjerge, AU
** Modified:
** 4/4 2017 Added creation of header file
** 14/4 2017 Added memory pool and fast file IO
**********************************************/
#pragma once
#include <stdio.h>
#include <string.h>
using namespace std;
#include "LxRecord.h"
#include "Semaphore.h"

class LynxRecord 
{

public:
	LynxRecord () : semaWaitForData(1, 0, "SemaRecord")
	{
		// Clear lxRecord
		memset(&lxRecord, 0, sizeof(LxRecord));
		MemPoolSize = 0;
		pMemPoolStart = 0;
		pMemPoolReadPtr = 0;
		pMemPoolWritePtr = 0;
		RecordSize = sizeof(LxBoardsData)/sizeof(int32_t);
	};

	~LynxRecord ()
	{
		ClearMemPool();
	};

	// -------------------------------------------------------------------------------------
	// Methods to handle lynx record data and storing records in memory pool
	// -------------------------------------------------------------------------------------

	// Returns char pointer to lxRecord and size of buffer
	LxRecord *GetLxRecord(void) {
		return &lxRecord;
	}

	int GetBuffer(char **pBuffer) 
	{
		*pBuffer = (char *)&lxRecord;
		return sizeof(LxRecord);
	}

	// Handling of memory pool
	// Allocates memory pool in size of samples (int)
	bool CreateMemPool(uint32_t size)
	{
		pMemPoolStart = (int32_t *)calloc(size, sizeof(int));
		MemPoolSize = size;
		ResetMemPtr();
		return (pMemPoolStart != 0);
	}

	void ClearMemPool(void)
	{
		if (pMemPoolStart != 0)
			free(pMemPoolStart);
	}

	// Set writing of samples to start of memory pool
	void ResetMemPtr(void)
	{
		pMemPoolWritePtr = pMemPoolStart;
		pMemPoolReadPtr = pMemPoolStart;
		printf("Reset memory read/write pointers");
	}

	void SignalNewData(void)
	{
		semaWaitForData.signal();
	}

	bool AppendDataToMemPool()
	{
		if (pMemPoolWritePtr == 0) return false;

		if ( pMemPoolWritePtr < (pMemPoolStart+MemPoolSize-RecordSize) )
		{
			/*
			for (int j = 0; j < NUM_BOARDS; j++)
				for (int i = 0; i < NUM_CHANNELS; i++)
				{
					*pMemPoolWritePtr = lxRecord.board[j].data[i];
					pMemPoolWritePtr++;
				}
			*/
			memcpy(pMemPoolWritePtr, lxRecord.board, sizeof(LxBoardsData));
			pMemPoolWritePtr += RecordSize;

		    // Signal new data record in memory pool
			SignalNewData();
			return true;
		}
		else 
			ResetMemPtr();

		return false;
	}

	// Computes and verifies checksum of record
	void AddCheckSum(void)
	{
		uint32_t *pStart = (uint32_t *)&lxRecord;
		uint32_t checksum = 0;
		for (int i = 0; i < (int)(sizeof(LxRecord)/sizeof(uint32_t))-1; i++)
			checksum = checksum ^ pStart[i];
		lxRecord.checksum = checksum;
	}

	bool CheckSumOK(void) 
	{
		bool result = false;
		uint32_t *pStart = (uint32_t *)&lxRecord;
		uint32_t checksum = 0;
		for (int i = 0; i < (int)(sizeof(LxRecord)/sizeof(uint32_t)); i++)
			checksum = checksum ^ pStart[i];
		if (checksum == 0) 
			result = true;
		return result;
	}

	void CreatTestData(int start)
	{
		lxRecord.header.packetId = 1;
		lxRecord.header.timestampHigh = 6789;
		lxRecord.header.timestampLow = 12345;
		lxRecord.header.ttlIO = 0x5A5A5A5A;
		lxRecord.header.systemStatus = 2222;

		for (int j = 0; j < NUM_BOARDS; j++)
			for (int i = 0; i < NUM_CHANNELS; i++)
				lxRecord.board[j].data[i] = 10000 * (j + 1) + (start + i);

		AddCheckSum();
	}

	bool isMemPoolEmpty(bool print = false) 
	{
		if (print) 
			printf("Read 0x%016llX - Write 0x%016llX\r\n", (long long)pMemPoolReadPtr, (long long)pMemPoolWritePtr);
		return (pMemPoolReadPtr == pMemPoolWritePtr);
	}

	// -------------------------------------------------------------------------------------
	// Methods to handle writing lynx record data to files
	// -------------------------------------------------------------------------------------

	void AppendMemPoolIntConsole()
	{
		char dataStr[1024];
		dataStr[0] = 0;
		semaWaitForData.wait();
		LxBoardsData *pBoardsData = (LxBoardsData *) pMemPoolReadPtr;
		LxBoardsData *pMemPoolEndPtr = (LxBoardsData *)(pMemPoolWritePtr-RecordSize);
		while (pBoardsData <= pMemPoolEndPtr) {
			for (int j = 0; j < NUM_BOARDS; j++)
				sprintf(dataStr, "%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, \n",
						pBoardsData->board[j].data[0],
						pBoardsData->board[j].data[1],
						pBoardsData->board[j].data[2],
						pBoardsData->board[j].data[3],
						pBoardsData->board[j].data[4],
						pBoardsData->board[j].data[5],
						pBoardsData->board[j].data[6],
						pBoardsData->board[j].data[7],
						pBoardsData->board[j].data[8],
						pBoardsData->board[j].data[9],
						pBoardsData->board[j].data[10],
						pBoardsData->board[j].data[11],
						pBoardsData->board[j].data[12],
						pBoardsData->board[j].data[13],
						pBoardsData->board[j].data[14],
						pBoardsData->board[j].data[15],
						pBoardsData->board[j].data[16],
						pBoardsData->board[j].data[17],
						pBoardsData->board[j].data[18],
						pBoardsData->board[j].data[19],
						pBoardsData->board[j].data[20],
						pBoardsData->board[j].data[21],
						pBoardsData->board[j].data[22],
						pBoardsData->board[j].data[23],
						pBoardsData->board[j].data[24],
						pBoardsData->board[j].data[25],
						pBoardsData->board[j].data[26],
						pBoardsData->board[j].data[27],
						pBoardsData->board[j].data[28],
						pBoardsData->board[j].data[29],
						pBoardsData->board[j].data[30],
						pBoardsData->board[j].data[31]
						);
			printf(dataStr);
			pBoardsData++;
			pMemPoolReadPtr = (int32_t *)pBoardsData;
		}
	}

private:
	Semaphore semaWaitForData;
	LxRecord lxRecord;
	char dummy[10]; // Just for safety, could be removed
	uint32_t RecordSize;
	uint32_t MemPoolSize;
	int32_t *pMemPoolReadPtr;
	int32_t *pMemPoolWritePtr;
	int32_t *pMemPoolStart;
};
