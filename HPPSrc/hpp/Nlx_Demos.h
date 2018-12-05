/*
 * Nlx_Demos.h
 *
 *  Created on: Jun 18, 2014
 *      Author: stevem
 */

#ifndef NLX_DEMOS_H_
#define NLX_DEMOS_H_

#include "xil_types.h"
#include "FreeRTOS.h"

extern struct Pulse_Train_Params pulse_params;

//We should probably add a parameter to specify whether or not the spike needs to be above or below the threshold for it to count
struct Burst_Analysis_Params
{
	u16		num_channels; //Number of channels in the ch_to_match array (max 512)
	u16		ch_to_match[512]; //A list of the channel numbers to test against the templates
	u8		ttl_output_bitnum; //The Digital I/O signal to toggle upon a match (input by user)
	u8		portnum; //Set inside of SW
	u8		bitnum; //Set inside of SW
	u8		pulse_type; //Input by user, type = enum TTLPulseType (0 = DIO_PULSE_LOW, 1 = DIO_PULSE_HIGH)
	u8		number_of_bursts; //Input by user to specify how many threshold ascents need to occur within the duration time for a successful burst.
	u64		duration; //Input by user.  It is the allowed amount of time to occur from first match/timestamp to last match/timestamp to determine a successful burst.
	s32		threshold; //Threshold
};


struct Neural_Ensemble_Params
{
	u16		num_channels; //Number of channels in the ch_to_match array (max 512)
	u8		num_templates; //Number of templates provided in templates array
	u16		ch_to_match[512]; //A list of the channel numbers to test against the templates
	u8		ttl_output_bitnum; //The Digital I/O signal to toggle upon a match
	u8		portnum; //Set inside of SW (it can be, but doesn't need to be assigned by user)
	u8		bitnum; //Set inside of SW (it can be, but doesn't need to be assigned by user)
	u8		pulse_type; //Input by user, type = enum TTLPulseType (0 = DIO_PULSE_LOW, 1 = DIO_PULSE_HIGH)
	u64		ensemble_duration; //Input by user.  It is the allowed amount of time to occur from first match/timestamp to last match/timestamp to determine a successful ensemble.
	s32		templates[2048]; //32 Templates of 32 values low, 32 values high...all low templates listed first, then all highs
};


struct Spike_Detect_Params
{
	u16		num_channels; //Number of channels in the ch_to_match array (max 512)
	u8		num_templates; //Number of templates provided in templates array
	u16		ch_to_match[512]; //A list of the channel numbers to test against the templates
	u8		ttl_output_bitnum; //The Digital I/O signal to toggle upon a match
	u8		portnum; //Set inside of SW (it can be, but doesn't need to be assigned by user)
	u8		bitnum; //Set inside of SW (it can be, but doesn't need to be assigned by user)
	u8		pulse_type; //Input by user, type = enum TTLPulseType (0 = DIO_PULSE_LOW, 1 = DIO_PULSE_HIGH)
	s32		templates[2048]; //32 Templates of 32 values low, 32 values high...all low templates listed first, then all highs
};

void vAnalogVerification(void *params);
void vSetupAnalogVerification(void);

void vBurstAnalysis(void *params);
void vSetupBurstAnalysis(struct Burst_Analysis_Params *params);

void vNeuralEnsembles(void *params);
void vSetupNeuralEnsembles(struct Neural_Ensemble_Params *params);

void vSpikeDetect(void *params);
void vSetupSpikeDetect(struct Spike_Detect_Params *params);

void vTimeStampCheck(void);
void vSetupTimeStampCheck(void);
void vPrintTimeStamp(void);


portBASE_TYPE Start_Spike_Detect(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
portBASE_TYPE Stop_Spike_Detect(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
portBASE_TYPE Start_Ensemble_Detect(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
portBASE_TYPE Stop_Ensemble_Detect(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
portBASE_TYPE Start_Burst_Analysis(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
portBASE_TYPE Stop_Burst_Analysis(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
portBASE_TYPE Start_PL_Demos(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
portBASE_TYPE Stop_PL_Demos(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
portBASE_TYPE exp_ctrl_test(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
portBASE_TYPE print_num_ints(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);

#endif /* NLX_DEMOS_H_ */
