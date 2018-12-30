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

#define MAX_CONFIG_SIZE 5000
#define MAX_CFG_TEMPLATES 4

typedef struct TemplateCfg
{
	string name;
	float threshold;
	int min;
	int max;
	int counter;
} TEMPLATECFG;

class Config
{
public:
	Config() : m_file((char *)"0:/") { }

	int loadConfig(string name);
	int loadCoeff(string name); // TODO implement reading coefficients from file
	const char *getConfigName(void) { return mFileName.c_str(); }
	const char *getTemplateName(int idx) { return tempConfig[idx].name.c_str(); }
	float getThreshold(int idx) { return tempConfig[idx].threshold; }
	int getMin(int idx) { return tempConfig[idx].min; }
	int getMax(int idx) { return tempConfig[idx].max; }
	int getCounter(int idx) { return tempConfig[idx].counter; }
	int getNumTemplates(void) { return mNumTemplates; }

private:
	void parseConfig(void);
	void parseCoeff(void);
	bool getNextLine(void);

    string mFileName;
    FileSDCard m_file;
    int mNumTemplates;
    TemplateCfg tempConfig[MAX_CFG_TEMPLATES];

    char mConfigTxt[MAX_CONFIG_SIZE];
    int mPos;
    char mLineTxt[500];
};



#endif /* SRC_CONFIG_H_ */
