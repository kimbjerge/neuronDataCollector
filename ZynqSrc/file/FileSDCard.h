/*
 * FileSDCard.h
 *
 *  Created on: 2. dec. 2018
 *      Author: Kim Bjerge
 */

#ifndef SRC_FILESDCARD_H_
#define SRC_FILESDCARD_H_
#include <string.h>
#include "xsdps.h"		/* SD device driver */
#include "ff.h"

class FileSDCard
{
public:

	FileSDCard(char* name)
	{
		strcpy(pathName, "0:/");
		strcpy(fileName, name);
	}

	int mount(bool remount); // Mount SD card to FAT file system

    int open(BYTE mode); // mode = FA_OPEN_EXISTING | FA_CREATE_ALWAYS | FA_WRITE | FA_READ - see ff.h
    int read(void *buffer, int size, bool fromStart); // Default read from start of file (fromStart=true)
    int write(const void* buffer, int size, bool append); // Default write to start of file (append=true)
    int close();

	static bool m_mounted;

private:
	TCHAR fileName[256];
	TCHAR pathName[256];
	unsigned int NumBytesWritten;
	unsigned int NumBytesRead;
	FIL m_fil;
};

#endif /* SRC_FILESDCARD_H_ */
