/*
 * Config.cpp
 *
 *  Created on: 30. dec. 2018
 *      Author: Kim Bjerge
 */
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "Config.h"

int Config::loadTxtFile(string name)
{
	int result;

	// Clear config text buffer
	memset(mConfigTxt, 0, sizeof(mConfigTxt));

	result = m_file.mount();
	if (result != XST_SUCCESS) printf("Failed to mount SD card\r\n");

	// Read contents of text file
	result = m_file.open((char *)name.c_str(), FA_OPEN_EXISTING | FA_READ);
	if (result != XST_SUCCESS) {
		printf("Failed open file %s for reading\r\n", name.c_str());
		return result;
	}

	result = m_file.read((void *)mConfigTxt, sizeof(mConfigTxt));
	if (result != XST_SUCCESS) {
		printf("Failed reading from file %s\r\n", name.c_str());
		return result;
	}

	result = m_file.close();
	if (result != XST_SUCCESS) {
		printf("Failed closing file %s\r\n", name.c_str());
		return result;
	}

	return result;
}

int Config::loadCoeffBin(string name)
{
	int result;
	unsigned int fileSize;

	// Read contents of binary coefficients file
	result = m_file.open((char *)name.c_str(), FA_OPEN_EXISTING | FA_READ);
	if (result != XST_SUCCESS) {
		return result;
	}

	mTabsValidAll = false;
	result = m_file.read((void *)mCoeffsAll, sizeof(mCoeffsAll));
	if (result != XST_SUCCESS) {
		printf("Failed reading from file %s\r\n", name.c_str());
		return result;
	}

	fileSize = m_file.getReadSize();
	if (fileSize != sizeof(mCoeffsAll))
	{
		printf("Wrong size of binary coefficients file %s size %d expected 60 TABS in each 32 channels!!!\r\n", name.c_str(), fileSize);
		m_file.close();
		return -1;
	}

	result = m_file.close();
	if (result != XST_SUCCESS) {
		printf("Failed closing file %s\r\n", name.c_str());
		return result;
	}

	mTabsValidAll = true;
	printf("FIR coefficients loaded from file %s\r\n", name.c_str());
	return result;
}

int Config::loadCoeff(string name)
{
	int result;

	// Clear coefficients buffer
	memset(mCoeffs, 0, sizeof(mCoeffs));
	mTabsValid = false;

	result = loadTxtFile(name);
    if (result == XST_SUCCESS) {
    	mTabsName = name;
    	parseCoeff();
    	mTabsValid = true;
    }

	return result;
}

int Config::loadConfig(string name)
{
	int result;

	result = loadTxtFile(name);
    if (result == XST_SUCCESS) {
		mCfgName = name;
		parseConfig();
    }

	return result;
}

void Config::loadTemplateConfig(void)
{
	int result;

	for (int i = 0; i < mNumTemplates; i++) {
		string tempCfg = tempConfig[i].tempCfg;
		if (tempCfg.compare("NONE")) {
			result = loadTxtFile(tempConfig[i].tempCfg);
		    if (result == XST_SUCCESS) {
				printf("Configuration for template %d found in %s\r\n", i+1, tempCfg.c_str());
		    	parseTempConfig(i);
		    }
		}
	}
}

bool Config::getNextLine(void)
{
  int posStart;
  int pos;

  memset(mLineTxt, 0, sizeof(mLineTxt));

  do {
	  pos = mPos;
	  posStart = mPos;
	  int i = 0;
	  while (mConfigTxt[pos] != 0 && // End of buffer
			 mConfigTxt[pos] != '\n' && // New line
			 mConfigTxt[pos] != EOF) {
		  if (mConfigTxt[pos] != '\r' && i < (int)sizeof(mLineTxt)) // Ignore returns
			  mLineTxt[i++] = mConfigTxt[pos];
		  pos++;
	  }
	  mPos = pos+1;
	  mLineTxt[i] = 0;
  } while (mConfigTxt[posStart] == '%'); // Ignore commented lines

  if (mConfigTxt[pos] == '\n')
	  return true;
  else
	  return false;
}

void Config::parseConfig(void)
{
	char tempName[20];
	char tempCfg[20];
	int width;
	int length;
	float threshold;
	int min;
	int max;
	int counter;
	int offset;
	int minGradient;

	mPos = 0;
	mNumTemplates = 0;
	while (getNextLine()) {
		sscanf(mLineTxt, "%s %d %d %f %d %d %d %d %d %s",
				tempName, &width, &length, &threshold, &counter,
				&max, &min, &offset, &minGradient, tempCfg);
		if (mNumTemplates < MAX_CFG_TEMPLATES) {
			tempConfig[mNumTemplates].name = tempName;
			tempConfig[mNumTemplates].width = width;
			tempConfig[mNumTemplates].length = length;
			tempConfig[mNumTemplates].threshold = threshold;
			tempConfig[mNumTemplates].counter = counter;
			tempConfig[mNumTemplates].max = max;
			tempConfig[mNumTemplates].min = min;
			tempConfig[mNumTemplates].minGradient = minGradient;
			tempConfig[mNumTemplates].tempCfg = tempCfg;
			for (int ch = 0; ch < TEMP_WIDTH; ch++) {
				 mPeakMaxLimits[mNumTemplates][ch] = max;
				 mPeakMinLimits[mNumTemplates][ch] = min;
				 mChannelMap[mNumTemplates][ch] = ch+offset; // Use offset to map channels
			}
			printf("Using template %d file %s cfg %s\r\n", mNumTemplates, tempName, tempCfg);
			mNumTemplates++;
		}
	}
}

void Config::parseCoeff(void)
{
	int idx = 0;
	mPos = 0;
	while (getNextLine()) {
		sscanf(mLineTxt, "%g", &mCoeffs[idx++]);
		if (idx == MAX_TAPS) break;
	}
	printf("Loaded FIR coefficients in total %d\r\n", idx);
}

void Config::parseTempConfig(int idx)
{
	int value[TEMP_WIDTH];
	int i, lineNumber = 1;

	mPos = 0;
	while (getNextLine()) {
		sscanf(mLineTxt, "%d %d %d %d %d %d %d %d %d",
				          &value[0],
				          &value[1],
				          &value[2],
				          &value[3],
				          &value[4],
				          &value[5],
				          &value[6],
				          &value[7],
				          &value[8]
						  );
		switch(lineNumber) {
			case 1: // Maximum peaks
				for (i = 0; i < TEMP_WIDTH; i++)
					mPeakMaxLimits[idx][i] = value[i];
				break;
			case 2: // Minimum peaks
				for (i = 0; i < TEMP_WIDTH; i++)
					mPeakMinLimits[idx][i] = value[i];
				break;
			case 3: // Channel mapping
				for (i = 0; i < TEMP_WIDTH; i++)
					mChannelMap[idx][i] = value[i];
				break;
			default:
				// Ignore more lines
				break;
		}
		lineNumber++;
	}
}

