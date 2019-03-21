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
#define MAX_CHANNELS        32

typedef struct TemplateCfg {
	string name;
	int width;
	int length;
	float threshold;
	int counter;
	int max;
	int min;
	int minGradient;
	string tempCfg;
} TEMPLATECFG;

class Config
{
public:
	Config() : m_file((char *)"0:/"), mNumTemplates(0), mTabsValid(false), mTabsValidAll(false)
	{
		for (int i = 0; i < MAX_CFG_TEMPLATES; i++) {
			tempConfig[i].width = TEMP_WIDTH;
			tempConfig[i].length = TEMP_LENGTH;
			tempConfig[i].threshold = 0.6;
			tempConfig[i].counter = 0;
			tempConfig[i].max = 30000;
			tempConfig[i].min = -30000;
			tempConfig[i].minGradient = 0;
		}
	}

	int loadConfig(string name);
	int loadCoeff(string name);
	int loadCoeffBin(string name);
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
	int getMinGradient(int idx) { return tempConfig[idx].minGradient; }
	short *getPeakMaxLimits(int idx) { return mPeakMaxLimits[idx]; }
	short *getPeakMinLimits(int idx) { return mPeakMinLimits[idx]; }
	short *getChannelMap(int idx) { return mChannelMap[idx]; }
	int getNumTemplates(void) { return mNumTemplates; }
	bool isTabsValid(void) { return mTabsValid; }
	bool isAllTabsValid(void) { return mTabsValidAll; }
	float *getCoeffs(void) { return mCoeffs; }
	float *getCoeffsAll(int ch) { return mCoeffsAll[ch]; }

	void setMinGradient(int idx, int gradient) { tempConfig[idx].minGradient = gradient; };
    void setPeakMaxLimits(int idx, short *limits) { memcpy(mPeakMaxLimits[idx], limits, sizeof(short)*TEMP_WIDTH); };
    void setPeakMinLimits(int idx, short *limits) { memcpy(mPeakMinLimits[idx], limits, sizeof(short)*TEMP_WIDTH); };
    void setChannelMap(int idx, short *map) { memcpy(mChannelMap[idx], map, sizeof(short)*TEMP_WIDTH); };
    void setNumTemplates(int num) { mNumTemplates = num; };
    void setThreshold(int idx, float threshold) { tempConfig[idx].threshold = threshold; }

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
    bool mTabsValidAll;
    float mCoeffs[MAX_TAPS];
    float mCoeffsAll[MAX_CHANNELS][MAX_TAPS];
    short mPeakMaxLimits[MAX_CFG_TEMPLATES][TEMP_WIDTH];
    short mPeakMinLimits[MAX_CFG_TEMPLATES][TEMP_WIDTH];
    short mChannelMap[MAX_CFG_TEMPLATES][TEMP_WIDTH];

    char mConfigTxt[MAX_CONFIG_SIZE];
    int mPos;
    char mLineTxt[500];
};



#endif /* SRC_CONFIG_H_ */
