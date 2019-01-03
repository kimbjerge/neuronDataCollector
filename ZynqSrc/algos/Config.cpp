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

int Config::loadCoeff(string name)
{
	int result;

	// Clear coefficients buffer
	memset(mCoeffs, 0, sizeof(mCoeffs));
	// Clear config text buffer
	memset(mConfigTxt, 0, sizeof(mConfigTxt));
	mTabsValid = false;

	// Update template from file and compute variance and mean
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

	mTabsName = name;
	parseCoeff();
	mTabsValid = true;
	return result;
}

int Config::loadConfig(string name)
{
	int result;

	// Clear config text buffer
	memset(mConfigTxt, 0, sizeof(mConfigTxt));

	// Update template from file and compute variance and mean
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

	mCfgName = name;
	parseConfig();

	return result;
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
	float threshold;
	int min;
	int max;
	int counter;
	int offset;

	mPos = 0;
	mNumTemplates = 0;
	while (getNextLine()) {
		sscanf(mLineTxt, "%s %f %d %d %d %d", tempName, &threshold, &min, &max, &counter, &offset);
		if (mNumTemplates < MAX_CFG_TEMPLATES) {
			tempConfig[mNumTemplates].name = tempName;
			tempConfig[mNumTemplates].threshold = threshold;
			tempConfig[mNumTemplates].min = min;
			tempConfig[mNumTemplates].max = max;
			tempConfig[mNumTemplates].counter = counter;
			// TODO change to load different values for each channel and channel map
			for (int ch = 0; ch < TEMP_WIDTH; ch++) {
				 mPeakMaxLimits[mNumTemplates][ch] = max;
				 mPeakMinLimits[mNumTemplates][ch] = min;
				 mChannelMap[mNumTemplates][ch] = ch+offset; // Use offset to map channels
			}
			mNumTemplates++;
		}
		printf("Using template %d file %s\r\n", mNumTemplates, tempName);
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
