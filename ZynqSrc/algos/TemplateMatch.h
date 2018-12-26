/*
 * TemplateMatch.h
 *
 *  Created on: 25. dec. 2018
 *      Author: Kim Bjerge
 */

#ifndef SRC_TEMPLATE_MATCH_H_
#define SRC_TEMPLATE_MATCH_H_

#include "Thread.h"
using namespace AbstractOS;

#include "FIRFilter_HLS.h"
#include "NXCOR_HLS.h"
#include "NeuronData.h"
#include "Template.h"
#include "ResultFile.h"

#define FIR_FORMAT      15  // Number of bits used for fixed point coefficients
#define FIR_TAPS   		60  // Number of FIR taps
#define FIR_SIZE		8   // Number of FIR filters in each FIRFilter HLS class
#define FIR_NUM         (NUM_CHANNELS/FIR_SIZE)  // Number of FIRFilter classes
#define TEMP_NUM		2   // Number of templates


class TemplateMatch : public Thread
{
public:
	TemplateMatch(ThreadPriority pri, string name, NeuronData *pData) :
					Thread(pri, name), mNumSamples(0), pNeuronData(pData) { }
	~TemplateMatch();

	int Init(string *pTempNames[TEMP_NUM], int numSamples, IRQ* pIrq = 0);

	virtual void run();

private:
	int updateCoefficients();
	int updateTemplates();
	void processResults(void);

	int mNumSamples;
	NeuronData *pNeuronData;
	FirFilter *pFirFilter[FIR_NUM];
    NXCOR *pNXCOR[TEMP_NUM];
    Template *pTemplate[TEMP_NUM];
	LRECORD lxRecord;
    int mCoeff[FIR_TAPS];
    int mFiltered[NUM_CHANNELS];
    float mNXCORRes[TEMP_NUM];
    ResultFile<float> *pResultNXCOR[TEMP_NUM];
    ResultFile<int> *pResultFIR;
};

#endif /* SRC_TEMPLATE_MATCH_H_ */