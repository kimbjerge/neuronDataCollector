/*
 * Config.h
 *
 *  Created on: 30. dec. 2018
 *      Author: Kim Bjerge
 */

#ifndef SRC_CONFIG_H_
#define SRC_CONFIG_H_

#include <string>
using namespace std;
#include "FileSDCard.h"

#define MAX_CONFIG_SIZE 	5000
#define MAX_CFG_TEMPLATES 	4
#define MAX_TAPS			60

typedef struct TemplateCfg {
	string name;
	float threshold;
	int min;
	int max;
	int counter;
} TEMPLATECFG;

class Config
{
public:
	Config() : m_file((char *)"0:/"), mNumTemplates(0), mTabsValid(false) { }

	int loadConfig(string name);
	int loadCoeff(string name);
	const char *getConfigName(void) { return mCfgName.c_str(); }
	const char *getTemplateName(int idx) { return tempConfig[idx].name.c_str(); }
	float getThreshold(int idx) { return tempConfig[idx].threshold; }
	int getMin(int idx) { return tempConfig[idx].min; }
	int getMax(int idx) { return tempConfig[idx].max; }
	int getCounter(int idx) { return tempConfig[idx].counter; }
	int getNumTemplates(void) { return mNumTemplates; }
	bool isTabsValid(void) { return mTabsValid; }
	float *getCoeffs(void) { return mCoeffs; }

private:
	void parseConfig(void);
	void parseCoeff(void);
	bool getNextLine(void);

    string mCfgName;
    string mTabsName;
    FileSDCard m_file;
    int mNumTemplates;
    TemplateCfg tempConfig[MAX_CFG_TEMPLATES];
    bool mTabsValid;
    float mCoeffs[MAX_TAPS];

    char mConfigTxt[MAX_CONFIG_SIZE];
    int mPos;
    char mLineTxt[500];
};



#endif /* SRC_CONFIG_H_ */
