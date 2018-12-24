/*
 * FirHLS.h
 *
 *  Created on: 16. aug. 2017
 *      Author: Kim Bjerge
 */

#ifndef SRC_HAL_FIRHLS_H_
#define SRC_HAL_FIRHLS_H_

#include "Thread.h"
using namespace AbstractOS;

#include "FIRFilter_HLS.h"
#include "NXCOR_HLS.h"
#include "NeuronData.h"

#define FIR_TAPS   		60  // Number of FIR taps
#define FIR_SIZE		8   // Number of FIR filters in each FIRFilter HLS class
#define FIR_NUM         (NUM_CHANNELS/FIR_SIZE)  // Number of FIRFilter classes
#define TEMP_WIDTH  	8	// Template width
#define TEMP_LENGTH     16  // Template length
#define TEMP_SIZE       (TEMP_WIDTH*TEMP_LENGTH)
#define TEMP_NUM		2   // Number of templates


class TemplateMatch : public Thread
{
public:
	TemplateMatch(ThreadPriority pri, string name, NeuronData *pData) :
					Thread(pri, name), pNeuronData(pData) { }

	int Init(IRQ* pIrq = 0);

	int updateCoefficients();
	int updateTemplates();
	void processResults(void);

	virtual void run();

private:
	NeuronData *pNeuronData;
	FirFilter *pFirFilter[FIR_NUM];
    NXCOR *pNXCOR[TEMP_NUM];
	LRECORD lxRecord;
    int mCoeff[FIR_TAPS];
    int mTemplates[TEMP_SIZE*TEMP_NUM];
    int mFiltered[NUM_CHANNELS];
    float mNXCORRes[TEMP_NUM];
};



#endif /* SRC_HAL_FIRHLS_H_ */
