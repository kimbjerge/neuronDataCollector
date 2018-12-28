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

int Template::loadTemplate(string name)
{
	int result;

	// Update template from file and compute variance and mean
	result = m_file.open((char *)name.c_str(), FA_OPEN_EXISTING | FA_READ);
	if (result != XST_SUCCESS) printf("Failed open file %s for reading\r\n", name.c_str());

	result = m_file.read((void *)mTemplate, sizeof(mTemplate));
	if (result != XST_SUCCESS) printf("Failed reading from file %s\r\n", name.c_str());

	result = m_file.close();
	if (result != XST_SUCCESS) printf("Failed closing file %s\r\n", name.c_str());

	// Reverse template - Convolution - MATLAB NXCOR
	for (int i = 0; i < TEMP_LENGTH; i++) {
		for (int j = 0; j < TEMP_WIDTH; j++) {
			mTemplateInt[j + i*TEMP_WIDTH] = round(mTemplate[j + (TEMP_LENGTH-1-i)*TEMP_WIDTH]*pow(2, DATA_FORMAT));
		}
	}

	// Cross correlation
	//for(int i = 0; i < TEMP_SIZE; i++)
	//	mTemplateInt[i] = round(mTemplate[i]*pow(2, DATA_FORMAT));

	readChOffset(name);
	calcMeanVariance();
	return result;
}

void Template::calcMeanVariance(void)
{
	mMean = 0.0;
	for (int i = 0; i < TEMP_LENGTH; i++)
		for (int j = 0; j < TEMP_WIDTH; j++)
			mMean += mTemplateInt[j + i*TEMP_WIDTH];
	mMean /= TEMP_SIZE;

	mVariance = 0;
	for (int i = 0; i < TEMP_LENGTH; i++)
		for (int j = 0; j < TEMP_WIDTH; j++)
			mVariance += pow((mTemplateInt[j + i*TEMP_WIDTH] - mMean), 2);
}


