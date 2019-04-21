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

	FileSDCard(char* path)
	{
		strcpy(pathName, path);
		strcpy(fileName, "");
		m_Opened = false;
	}

	int mount(bool remount=false); // Mount SD card to FAT file system

    int open(char *name, BYTE mode); // mode = FA_OPEN_EXISTING | FA_CREATE_ALWAYS | FA_WRITE | FA_READ - see ff.h
    int read(void *buffer, int size, bool fromStart=true); // Default read from start of file (fromStart=true)
    unsigned int getReadSize(void) { return mNumBytesRead; }; // Updated by read
    int write(const void* buffer, int size, bool append=false); // Default write to start of file (append=false)
    int close();
    unsigned int size(char *name);
    int list(char *fileList, int len); // Returns files in directory (max. numFiles)
    int del(char *name); // Delete file
    int rename(char *oldName, char *newName);
    char *getFileName(void) { return fileName; }

	static bool m_mounted;

private:
	TCHAR fileName[256];
	TCHAR pathName[256];
	unsigned int mNumBytesWritten;
	unsigned int mNumBytesRead;
	FIL m_fil;
	DIR m_dir;
	FILINFO m_fileInfo;
	bool m_Opened;
};

#endif /* SRC_FILESDCARD_H_ */
