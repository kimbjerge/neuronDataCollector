/*
 * Config.h
 *
 *  Created on: 30. dec. 2018
 *      Author: Kim Bjerge
 */

#ifndef SRC_CONFIG_H_
#define SRC_CONFIG_H_

#include "Template.h"

#define MAX_CONFIG_SIZE 	5000
#define MAX_CFG_TEMPLATES 	10
#define MAX_TAPS			60

typedef struct TemplateCfg {
	string name;
	int width;
	int length;
	float threshold;
	int counter;
	int max;
	int min;
	string tempCfg;
} TEMPLATECFG;

class Config
{
public:
	Config() : m_file((char *)"0:/"), mNumTemplates(0), mTabsValid(false) { }

	int loadConfig(string name);
	int loadCoeff(string name);
	void loadTemplateConfig(void);
	const char *getConfigName(void) { return mCfgName.c_str(); }
	const char *getTemplateName(int idx) { return tempConfig[idx].name.c_str(); }
	const char *getTempCfgName(int idx) { return tempConfig[idx].tempCfg.c_str(); }
	int getWidth(int idx) { return tempConfig[idx].width; }
	int getLength(int idx) { return tempConfig[idx].length; }
	float getThreshold(int idx) { return tempConfig[idx].threshold; }
	int getMax(int idx) { return tempConfig[idx].max; }
	int getMin(int idx) { return tempConfig[idx].min; }
	int getCounter(int idx) { return tempConfig[idx].counter; }
	short *getPeakMaxLimits(int idx) { return mPeakMaxLimits[idx]; }
	short *getPeakMinLimits(int idx) { return mPeakMinLimits[idx]; }
	short *getChannelMap(int idx) { return mChannelMap[idx]; }
	int getNumTemplates(void) { return mNumTemplates; }
	bool isTabsValid(void) { return mTabsValid; }
	float *getCoeffs(void) { return mCoeffs; }

private:
	void parseConfig(void);
	void parseCoeff(void);
	void parseTempConfig(int idx);
	bool getNextLine(void);
	int loadTxtFile(string name);

    string mCfgName;
    string mTabsName;
    FileSDCard m_file;
    int mNumTemplates;
    TemplateCfg tempConfig[MAX_CFG_TEMPLATES];
    bool mTabsValid;
    float mCoeffs[MAX_TAPS];
    short mPeakMaxLimits[MAX_CFG_TEMPLATES][TEMP_WIDTH];
    short mPeakMinLimits[MAX_CFG_TEMPLATES][TEMP_WIDTH];
    short mChannelMap[MAX_CFG_TEMPLATES][TEMP_WIDTH];

    char mConfigTxt[MAX_CONFIG_SIZE];
    int mPos;
    char mLineTxt[500];
};



#endif /* SRC_CONFIG_H_ */
