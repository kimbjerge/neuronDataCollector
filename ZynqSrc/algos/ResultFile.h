/*
 * ResultFile.h
 *
 *  Created on: 26. dec. 2018
 *      Author: Kim Bjerge
 */

#ifndef SRC_RESULT_FILE_H_
#define SRC_RESULT_FILE_H_

#include <string>
using namespace std;
#include "FileSDCard.h"

template <class T>
class ResultFile
{
public:
	ResultFile() : mFileContent(NULL), mIdxWrite(0), mContentSize(0), m_file((char *)"0:/") { }
	~ResultFile() { if (mFileContent != 0) free(mFileContent); }
	int allocateContent(int size);
	int appendData(T *data, int size);
	int saveContent(string name);
	void reset(void) { mIdxWrite = 0; }

private:
    T *mFileContent;
    int mIdxWrite;
    int mContentSize;
    FileSDCard m_file;
};


template <class T>
int ResultFile<T>::allocateContent(int size)
{
	mFileContent = (T*)malloc(size*sizeof(T));
	if (mFileContent)
		return 0;
	else {
		printf("Failed to allocate content of size %d for test file\r\n", size);
		return -1;
	}
}

template <class T>
int ResultFile<T>::appendData(T *data, int size)
{
	int result = -1;
	if ((mIdxWrite + size) <= mContentSize) {
		memcpy(&mFileContent[mIdxWrite], data, size*sizeof(T));
		mIdxWrite += size;
		result = 0;
	}
	return result;
}

template <class T>
int ResultFile<T>::saveContent(string name)
{
	int result;

	// Update template from file and compute variance and mean
	result = m_file.open((char *)name.c_str(), FA_CREATE_ALWAYS | FA_WRITE);
	if (result != XST_SUCCESS) printf("Failed open file %s for reading\r\n", name.c_str());

	result = m_file.write((void *)mFileContent, mIdxWrite*sizeof(T));
	if (result != XST_SUCCESS) printf("Failed writing to file %s\r\n", name.c_str());

	result = m_file.close();
	if (result != XST_SUCCESS) printf("Failed closing file %s\r\n", name.c_str());

	return result;
}


#endif /* SRC_RESULT_FILE_H_ */
