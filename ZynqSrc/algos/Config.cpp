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

int Config::loadConfig(string name)
{
	int result;

	// Clear config text buffer
	memset(mConfigTxt, 0, sizeof(mConfigTxt));

	// Update template from file and compute variance and mean
	result = m_file.open((char *)name.c_str(), FA_OPEN_EXISTING | FA_READ);
	if (result != XST_SUCCESS) printf("Failed open file %s for reading\r\n", name.c_str());

	result = m_file.read((void *)mConfigTxt, sizeof(mConfigTxt));
	if (result != XST_SUCCESS) printf("Failed reading from file %s\r\n", name.c_str());

	result = m_file.close();
	if (result != XST_SUCCESS) printf("Failed closing file %s\r\n", name.c_str());

	mFileName = name;
	parseConfig();

	return result;
}

bool Config::getNextLine(void)
{
  int posStart;
  int pos;
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

	mPos = 0;
	mNumTemplates = 0;
	while (getNextLine()) {
		sscanf(mLineTxt, "%s %f %d %d %d", tempName, &threshold, &min, &max, &counter);
		if (mNumTemplates < MAX_CFG_TEMPLATES) {
			tempConfig[mNumTemplates].name = tempName;
			tempConfig[mNumTemplates].threshold = threshold;
			tempConfig[mNumTemplates].min = min;
			tempConfig[mNumTemplates].max = max;
			tempConfig[mNumTemplates].counter = counter;
		}
		mNumTemplates++;
		printf("Using template %d file %s\r\n", mNumTemplates, tempName);
	}
}

