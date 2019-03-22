/*
 * TemplateMatch.h
 *
 *  Created on: 25. dec. 2018
 *      Author: Kim Bjerge
 */

#ifndef SRC_TEMPLATE_MATCH_H_
#define SRC_TEMPLATE_MATCH_H_

#include "Gpio.h"
#include "TestIO.h"
#include "Leds.h"
#include "Switch.h"
#include "Thread.h"
using namespace AbstractOS;

#include "FIRFilter_HLS.h"
#include "NXCOR_HLS.h"
#include "NeuronData.h"
#include "Template.h"
#include "ResultFile.h"
#include "Config.h"

#define FIR_FORMAT      23  // Number of bits used for fixed point coefficients
#define FIR_TAPS   		60  // Number of FIR taps
#define FIR_SIZE		8   // Number of FIR filters in each FIRFilter HLS class
#define FIR_NUM         (NUM_CHANNELS/FIR_SIZE)  // Number of FIRFilter classes
#define TEMP_NUM		6   // Number of templates

#define SAMPLES_SAVED   60*30000 // Maximum number of samples to save in files for debugging

class TemplateMatch : public Thread
{
public:

	TemplateMatch(NeuronData *pData) :  mNumSamples(0), pNeuronData(pData) { mNumCfgTemplates = TEMP_NUM; }
	TemplateMatch(ThreadPriority pri, string name, NeuronData *pData) :
					Thread(pri, name), mNumSamples(0), pNeuronData(pData) { mNumCfgTemplates = TEMP_NUM; }
	~TemplateMatch();

	int Init(Config *pConfig, int numSamples, IRQ* pIrq = 0);
	void updateConfig(int numSamples);
	void stopRunning(void) { mRunning = false; };
	void printSettings(void);
	void updateTemplateData(int id, float *data, int length, int width);
	Config *getConfig(void) { return mpConfig; }

	virtual void run();

private:
	void updateCoefficients();
	void updateTemplates();
	void clearIPCoresMemory();
	void reset(void);
	inline void processResults(void);
	inline void triggerTemplate12(void);

    int mNumCfgTemplates;
	int mNumSamples;
	int mCount;
	int mCounter;
	NeuronData *pNeuronData;
	FirFilter *pFirFilter[FIR_NUM];
    NXCOR *pNXCOR[TEMP_NUM];
    Template *pTemplate[TEMP_NUM];
	float *pCoeffFloat;
    int mCoeff[FIR_TAPS];
    STYPE mFiltered[NUM_CHANNELS];
    ResultFile<float> *pResultNXCOR[TEMP_NUM];
    ResultFile<STYPE> *pResultFIR;
    Leds leds;
    TestIO testOut;
    Switch sw;
    Config *mpConfig;

    // Trigger used to control digital TestIO::JB9
    // Digital output will be high if neuron template 1 and 2 are activated
    // Within mTemplate12NumCounts updated from counter in configuration from template 1
    int mTemplate12Counter;
    int mTemplate12Trigger;
    int mTemplate12TriggerIdx;
    bool mRunning;
};

#endif /* SRC_TEMPLATE_MATCH_H_ */
