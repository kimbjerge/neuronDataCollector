/*
 * Template.cpp
 *
 *  Created on: 25. dec. 2018
 *      Author: Kim Bjerge
 */
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "Template.h"
#include "LxRecord.h"

#define NXCOR_CONVOLUTION // Flip template and perform convolution like MATLAB does

// NOT USED AFTER Ver. 1.1
void Template::readChOffset(string name)
{
	int i, posOffset = name.find("_") + 1;
	char asciiOffset[3];
	for (i = 0; i < 2; i++)
		asciiOffset[i] = name[posOffset++];
	asciiOffset[i] = '\0';
	mChOffset = atoi(asciiOffset);
	if (mChOffset > NUM_CHANNELS-TEMP_WIDTH) {
		printf("Invalid channel offset %d in file name %s\r\n", mChOffset, name.c_str());
		mChOffset = 0;
	}
}

void Template::convertData(void)
{
	float abs_max_value = 0;

	// Search for maximum absolute template value
	for (int i = 0; i < TEMP_LENGTH; i++) {
		for (int j = 0; j < TEMP_WIDTH; j++) {
			if (i < mLength && j < mWidth)
				 if (abs_max_value < abs(mTemplate[j + i*mWidth]))
						 abs_max_value =  abs(mTemplate[j + i*mWidth]) ;
		}
	}

	// Find data format to scale template
	int dataFormat  =  0; // Don't scale template values if absolute max value > 255
	if (abs_max_value < 0) {
		dataFormat = DATA_FORMAT_SHORT; // Template values between -1.0 and 1.0 - then multiply with 2^23
	} else if (abs_max_value < 256) { // Template values between -256 and 256 - then multiply with 2^7
		dataFormat = DATA_FORMAT_CHAR;
	}
	printf("Template %s - all values scaled with 2^%d = %.0f\r\n", mFileName.c_str(), dataFormat, pow(2, dataFormat));


#ifdef NXCOR_CONVOLUTION
	// Reverse template - Convolution - MATLAB NXCOR
	for (int i = 0; i < TEMP_LENGTH; i++) {
		for (int j = 0; j < TEMP_WIDTH; j++) {
			if (i < mLength && j < mWidth)
				mTemplateInt[j + i*TEMP_WIDTH] = round(mTemplate[j + (mLength-1-i)*mWidth]*pow(2, dataFormat));
			else
				mTemplateInt[j + i*TEMP_WIDTH] = 0;
		}
	}
#else
	/*
	for(int i = 0; i < TEMP_SIZE; i++)
		mTemplateInt[i] = round(mTemplate[i]*pow(2, dataFormat));
	*/
	// Cross correlation
	for (int i = 0; i < TEMP_LENGTH; i++) {
		for (int j = 0; j < TEMP_WIDTH; j++) {
			if (i < mLength && j < mWidth)
				mTemplateInt[j + i*TEMP_WIDTH] = round(mTemplate[j + i*mWidth]*pow(2, dataFormat));
			else
				mTemplateInt[j + i*TEMP_WIDTH] = 0; // Clear part of template not used
		}
	}
#endif

	//readChOffset(name); // NOT USED ANY MORE
	calcMeanVariance();
}

void Template::updateData(float *data, int length, int width, int nr)
{
	char name[20];
	clearTemplate();

	// Limitation of input parameters
	if (length < TEMP_LENGTH)
		mLength = length;
	else
		mLength = TEMP_LENGTH;

	if (width < TEMP_WIDTH)
		mWidth = width;
	else
		mWidth = TEMP_WIDTH;

	memcpy(mTemplate, data, sizeof(mTemplate));

    sprintf(name, "T%d", nr);
	mFileName = name;

	convertData();
}


int Template::loadTemplate(string name, int length, int width)
{
	int result;
	unsigned int fileSize;

	clearTemplate();

	// Limitation of input parameters
	if (length < TEMP_LENGTH)
		mLength = length;
	else
		mLength = TEMP_LENGTH;

	if (width < TEMP_WIDTH)
		mWidth = width;
	else
		mWidth = TEMP_WIDTH;

	// Update template from file and compute variance and mean
	result = m_file.open((char *)name.c_str(), FA_OPEN_EXISTING | FA_READ);
	if (result != XST_SUCCESS) printf("Failed open file %s for reading\r\n", name.c_str());

	if (result == XST_SUCCESS) {
		result = m_file.read((void *)mTemplate, sizeof(mTemplate));
		if (result != XST_SUCCESS) printf("Failed reading from file %s\r\n", name.c_str());

		fileSize = m_file.getReadSize();
		if (fileSize != mLength*mWidth*sizeof(float)) {
			printf("Wrong size of binary coefficients file %s\r\n", name.c_str());
			printf("The size of template length %d width %d and file size %d is wrong!!!\r\n", length, width, fileSize);
		}

		result = m_file.close();
		if (result != XST_SUCCESS) printf("Failed closing file %s\r\n", name.c_str());

		mFileName = name;
		convertData();
	}

	return result;
}

void Template::clearTemplate(void)
{
	for(int i = 0; i < TEMP_SIZE; i++) {
		mTemplateInt[i] = 0;
		mTemplate[i] = 0;
	}
}

void Template::calcMeanVariance(void)
{
	mMean = 0.0;
	for (int i = 0; i < mLength; i++)
		for (int j = 0; j < mWidth; j++)
			mMean += mTemplateInt[j + i*TEMP_WIDTH];
	mMean /= (mLength*mWidth);

	mVariance = 0;
	for (int i = 0; i < mLength; i++)
		for (int j = 0; j < mWidth; j++)
			mVariance += pow((mTemplateInt[j + i*TEMP_WIDTH] - mMean), 2);
}
