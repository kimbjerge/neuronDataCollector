/*
 * FileSDCard.cpp
 *
 *  Created on: 2. dec. 2018
 *      Author: Kim Bjerge
 */
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
				 &NumBytesRead);
	if (Res) {
		return XST_FAILURE;
	}
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
						  &NumBytesWritten);
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
