/*
 * FileSDCard.cpp
 *
 *  Created on: 2. dec. 2018
 *      Author: Kim Bjerge
 */
#include "stdio.h"
#include "FileSDCard.h"

static FATFS m_fatfs;
bool FileSDCard::m_mounted = false;

int FileSDCard::mount(bool remount)
{
	FRESULT Res;
	if (!m_mounted or remount) {
		Res = f_mount(&m_fatfs, pathName, 0);

		if (Res != FR_OK) {
			return XST_FAILURE;
		}
		m_mounted = true;
	}
	return XST_SUCCESS;
}

int FileSDCard::del(char *name)
{
	FRESULT Res;
	mount(false);
	Res = f_unlink(name);
	if (Res != FR_OK) {
		return XST_FAILURE;
	}
	return XST_SUCCESS;
}

int FileSDCard::rename(char *oldName, char *newName)
{
	FRESULT Res;
	char oldFilePath[50] = {0};
	char newFilePath[50] = {0};

	mount(false);

	strcpy(oldFilePath, pathName);
	strcat(oldFilePath, oldName);
	strcpy(newFilePath, pathName);
	strcat(newFilePath, newName);

	Res = f_rename(oldFilePath, newFilePath);
	if (Res != FR_OK) {
		return XST_FAILURE;
	}
	return XST_SUCCESS;
}


int FileSDCard::list(char *fileList, int len)
{
	FRESULT Res;
	int num = 0;
	char text[40];

	memset(fileList, 0, len);
    mount(false);

    Res = f_opendir(&m_dir, pathName);
	if (Res) {
		return num;
	}
	while (Res == FR_OK) {
		Res = f_readdir(&m_dir, &m_fileInfo);
		if (Res == FR_OK) {
			// List files and sizes
			if (strlen(fileList) < unsigned(len-sizeof(text)) && strlen(m_fileInfo.fname) > 0) {
				if ((m_fileInfo.fattrib & AM_SYS) != AM_SYS) {    // Don't display system files
					if ((m_fileInfo.fattrib & AM_DIR) == AM_DIR)
						sprintf(text, "%15s            <DIR>\r\n", m_fileInfo.fname); // Directory
					else
						sprintf(text, "%15s %10d bytes\r\n", m_fileInfo.fname, m_fileInfo.fsize); // File
					strcat(fileList, text);
					num++;
				}
			} else
				break;
		}
	}
	f_closedir(&m_dir);

	return num;
}


int FileSDCard::open(char *name, BYTE mode)
{
	FRESULT Res;
	if (!m_mounted) {
		return XST_FAILURE;
	}
	strcpy(fileName, name);

	Res = f_open(&m_fil, fileName, mode);
	if (Res) {
		return XST_FAILURE;
	}
	Res = f_lseek(&m_fil, 0);
	if (Res) {
		return XST_FAILURE;
	}
	return XST_SUCCESS;
}

int FileSDCard::read(void *buffer, int size, bool fromStart)
{
	FRESULT Res;
	if (fromStart) {
		// Pointer to beginning of file .
		Res = f_lseek(&m_fil, 0);
		if (Res) {
			return XST_FAILURE;
		}
	}
	Res = f_read(&m_fil, (void*)buffer, size,
				 &mNumBytesRead);
	if (Res) {
		return XST_FAILURE;
	}
	//printf("%d number of bytes read\r\n", NumBytesRead);
	return XST_SUCCESS;
}

int FileSDCard::write(const void* buffer, int size, bool append)
{
	FRESULT Res;
	if (!append) {
		// Pointer to beginning of file .
		Res = f_lseek(&m_fil, 0);
		if (Res) {
			return XST_FAILURE;
		}
	}
	Res = f_write(&m_fil, (const void*)buffer, size,
						  &mNumBytesWritten);
	if (Res) {
		return XST_FAILURE;
	}
	return XST_SUCCESS;
}

int FileSDCard::close()
{
	FRESULT Res;
	Res = f_close(&m_fil);
	if (Res) {
		return XST_FAILURE;
	}
	return XST_SUCCESS;
}

unsigned int FileSDCard::size(char *name)
{
	FRESULT Res;
	FILINFO info;

	Res = f_stat(name, &info);
	if (Res) {
		return 0;
	}
	return info.fsize;
}

