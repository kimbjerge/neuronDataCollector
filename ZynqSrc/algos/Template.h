/*
 * Template.h
 *
 *  Created on: 25. dec. 2018
 *      Author: Kim Bjerge
 */

#ifndef SRC_TEMPLATE_H_
#define SRC_TEMPLATE_H_

#include <string>
using namespace std;
#include "FileSDCard.h"

#define TEMP_WIDTH  	8	// Template width
#define TEMP_LENGTH     16  // Template length
#define TEMP_SIZE       (TEMP_WIDTH*TEMP_LENGTH)

class Template
{
public:
	Template() : mMean(0.0), mVariance(0.0), mChOffset(0),  m_file((char *)"0:/") { }

	int loadTemplate(std::string name);
	int *getTemplate(void) { return mTemplate; }
	float getVariance(void) { return mVariance; }
	float getMean(void) { return mMean; }
	int getChOffset(void) { return mChOffset; }

private:
	void calcMeanVariance(void);
	void readChOffset(string name);
    int mTemplate[TEMP_SIZE];
    float mMean;
    float mVariance;
    int mChOffset;
    FileSDCard m_file;
};



#endif /* SRC_TEMPLATE_H_ */
