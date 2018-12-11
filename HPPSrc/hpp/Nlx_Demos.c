/*
 * hpp.c
 *
 *  Created on: Feb 18, 2014
 *      Author: stevem
 */

#include "hpp.h"
#include "Nlx_Demos.h"
#include <stdio.h>
#include "platform.h"
#include "xscugic.h"
#include "xparameters.h"
#include "xstatus.h"
#include "xgpiops.h"
#include "freertos.h"
#include "semphr.h"
#include "task.h"
#include "portmacro.h"
#include "xil_cache.h"
#include "sleep.h"
#include "xil_mmu.h"
#include <stdlib.h>
#include <math.h>


extern s32 tm_high[32];
extern s32 tm_low[32];
extern SemaphoreHandle_t xHPP_Data_Sem;
extern SemaphoreHandle_t xPulse_Train_Sem;

SemaphoreHandle_t xHPP_Burst_Analysis_Sem = NULL;
SemaphoreHandle_t xHPP_Neural_Ensemble_Sem = NULL;
SemaphoreHandle_t xHPP_Spike_Detect_Sem = NULL;

struct Burst_Analysis_Params *parameters;

struct Pulse_Train_Params pulse_params;

s8 DEBUG_DEMO_APPS = 0;


static TaskHandle_t xEnsemble_Detect_Handle = NULL;
static TaskHandle_t xSpike_Detect_Handle = NULL;
static TaskHandle_t xBurst_Analysis_Handle = NULL;

static u8 xSpike_Detect_Running = 0;
static u8 xEnsemble_Detect_Running = 0;
static u8 xBurst_Analysis_Running = 0;

s32 high_templates[32][32];
s32 low_templates[32][32];


//It would be a good idea to add protections (such as mutex semaphores) for things like the TTL lines in case of parallel processes trying
//to access the gpio lines simultaneously...


void vAnalogVerification(void *params)
{
	s8 status;
	s8 update_mask = 15;
	u32 i = 0;
	u8 j = 0;
	u16 analog[4][512];
	u32 sample_freq = 50000;
	u32 freq[4];
	freq[0] = 1000;
	freq[1] = 2000;
	freq[2] = 3000;
	freq[3] = 4000;
	s16 temp_sin = 0;

	(void) params;

	vSetupAnalogVerification();

	double sin_output = 0;

	for(i = 0; i < 512; i++)
	{
		for(j = 0; j < 4; j++)
		{
			//sin() accepts a double in radians and returns a double
			sin_output = sin(2*M_PI*i*freq[j]/sample_freq); //Get frequency point
			sin_output = sin_output * 65536; // = 0xffff...make 16 MSBs integer
			temp_sin = (s16)(sin_output); //Convert to signed, 16 bit integer
			analog[j][i] = (u16)(temp_sin + 32768); //Convert to unsigned int with 0 = lowest point in waveform
		}
	}

	i = 0;
	for(;;)
	{
		i = i % 512;
		status = UpdateAnalog(update_mask, analog[0][i], analog[1][i], analog[2][i], analog[3][i]);
		if(status != 0)
		{
			xil_printf("\n\rUnknown error with UpdateAnalog...should always return 0...\n\r");
		}
		//KBE!!! nanosleep(20000);
		usleep(20);
		i++;
	}
}


void vSetupAnalogVerification(void)
{
	u8 status = 0;

	//Assert PS control (as opposed to fabric control) over the commands sent to the motherboard
	status = SetPSControl(PS_Ctrl);
	if(status != 0)
	{
		xil_printf("\n\rProblem with SetPSControl(PS_Ctrl)");
	}
	else
	{
		xil_printf("\n\rSetPSControl(PS_Ctrl) successfully called.\n\r");
	}


	//Assert HPP control over MB TTL lines
	status = SetAnalogOutputCtrl(HPP_Ctrl);
	if(status != 0)
	{
		xil_printf("\n\rProblem with SetAnalogOutputCtrl(HPP_Ctrl), Status = %d", status);
	}


	SetPSReady(PS_Ctrl);
}


void vBurstAnalysis(void *params)
{
	u32 i = 0;
	u32 j = 0;
	u8 overflow_flag = 0;
	u32 cur_index = 0;
	u8 match_just_found[512];
	u32 num_matches[512]; //Used to keep track of how many times the threshold has been surpassed, per channel
	u64 thresholds_met[512][32]; //Used to keep track of the timestamps for each time the threshold was surpassed, per channel
								 //The first index is for the channel number and the second is to maintain the history of max 32 matches
	u32 threshold_values[10][32]; //Used to keep track of the amplitudes for each time the threshold was surpassed, per channel
								 //The first index is for the channel number and the second is to maintain the history of max 32 matches
	u32 sample_number[10][32]; //Used to keep track of the sample_number for each time the threshold was surpassed, per channel
								 //The first index is for the channel number and the second is to maintain the history of max 32 matches

	u32 current_num_matched = 0;
	u32 current_ch_num = 0;
	u8 current_match_just_found = 0;
	u8 num_bursts = 0;

	u8 old_index = 0;
	u8 new_index = 0;
	u64 old_timestamp = 0;
	u64 new_timestamp = 0;
	u64 temp_timestamp = 0;
	u64 temp_timestamp_high = 0;

	parameters = (struct Burst_Analysis_Params *) params;

	vSetupBurstAnalysis(&parameters[0]);

	/*
	u16		num_channels; //Number of channels in the ch_to_match array (max 512)
	u16		ch_to_match[512]; //A list of the channel numbers to test against the templates
	u8		ttl_output_bitnum; //The Digital I/O signal to toggle upon a match
	u8		portnum; //Set inside of SW (it can be, but doesn't need to be assigned by user)
	u8		bitnum; //Set inside of SW (it can be, but doesn't need to be assigned by user)
	u8		pulse_type; //Input by user, type = enum TTLPulseType (0 = DIO_PULSE_LOW, 1 = DIO_PULSE_HIGH)
	u8		number_of_bursts; //Input by user to specify how many threshold ascents need to occur within the duration time for a successful burst.
	u64		duration; //Input by user.  It is the allowed amount of time to occur from first match/timestamp to last match/timestamp to determine a successful burst.
	s32		threshold; //Threshold
	 */

	for(i = 0; i < 512; i++)
	{
		match_just_found[i] = 0;
		num_matches[i] = 0;
		for(j = 0; j < 32; j++)
		{
			thresholds_met[i][j] = 0; //Ensure match count is cleared ahead of time...
		}
	}

	for(i = 0; i < 10; i++)
	{
		for(j = 0; j < 32; j++)
		{
			threshold_values[i][j] = 0;
			sample_number[i][j] = 0;
		}
	}

	//we need to loop through the number of active channels, compare them to the threshold, store the timestamp in a circular buffer (of length number_of_bursts),
	//and make sure that a spike isn't treated as a burst for each sample that occurs above the threshold...
	for(;;)
	{
		if(xHPP_Burst_Analysis_Sem != NULL)
		{
			if(xSemaphoreTake(xHPP_Burst_Analysis_Sem, portMAX_DELAY ) == pdTRUE)
			{
				cur_index = num_data_buffers_loaded & AVAILABLE_BUFFERS_MASK; // & is faster than %, so the mask is setup for &

				for(i = 0; i < parameters[0].num_channels; i++) //For loop to go through each channel to be compared to the threshold
				{
					current_ch_num = parameters[0].ch_to_match[i];
					current_num_matched = num_matches[current_ch_num];
					current_match_just_found = match_just_found[current_ch_num];
					num_bursts = parameters[0].number_of_bursts;

					if(HPP_Data[cur_index].AD[current_ch_num] > parameters[0].threshold)
					{
						if(current_match_just_found == 0)
						{
							current_match_just_found = 1;
							temp_timestamp_high = ((u64)HPP_Data[cur_index].TimeStamp_High << 32) & 0xFFFFFFFF00000000;
							temp_timestamp = temp_timestamp_high | ((u64)HPP_Data[cur_index].TimeStamp_Low & 0x00000000FFFFFFFF);
							thresholds_met[current_ch_num][current_num_matched % num_bursts] = temp_timestamp;
							threshold_values[current_ch_num][current_num_matched % num_bursts] = HPP_Data[cur_index].AD[current_ch_num];
							sample_number[current_ch_num][current_num_matched % num_bursts] = cur_index;
							//xil_printf("\n\r HPP_Data[%d].AD[%d] = %d\n\r", cur_index, current_ch_num, HPP_Data[cur_index].AD[current_ch_num]);
							//xil_printf("HPP_Data[%d].TimeStamp_High = %8x, HPP_Data[%d].TimeStamp_Low = %8x\n\r", cur_index, HPP_Data[cur_index].TimeStamp_High, cur_index, HPP_Data[cur_index].TimeStamp_Low);
							current_num_matched++;
							//xil_printf("current_num_matched = %d, current_ch_num = %d\n\r", current_num_matched, current_ch_num);
						}
					}
					else
					{
						current_match_just_found = 0;
					}
					//If we saw the threshold surpassed and the channel has surpassed it at least as many times as the minimum required
					if((current_match_just_found == 1) && (current_num_matched >= num_bursts))
					{
						//Take the timestamp of the "match" that just occurred and subtract the timestamp of the "match" occurring as many back as the number of bursts required to find out if
						//all of the matches for the number of matches required occurred within the amount of time necessary for the burst to be identified correctly.  If so, send pulse...
						old_index = (current_num_matched - num_bursts) % num_bursts;
						old_timestamp = thresholds_met[current_ch_num][old_index];
						new_index = (current_num_matched - 1) % num_bursts;
						new_timestamp = thresholds_met[current_ch_num][new_index];
						if((new_timestamp - old_timestamp) < parameters[0].duration)
						{
							ToggleDigIO(parameters[0].ttl_output_bitnum);
							switch(current_ch_num)
							{
								case 0:	xil_printf(".");
										break;
								case 1:	xil_printf(",");
										break;
								case 2:	xil_printf(";");
										break;
								case 3:	xil_printf(":");
										break;
								case 4:	xil_printf("'");
										break;
								case 5:	xil_printf("]");
										break;
								case 6:	xil_printf("/");
										break;
								case 7:	xil_printf("?");
										break;
							}
						}
					}
					num_matches[current_ch_num] = current_num_matched;
					match_just_found[current_ch_num] = current_match_just_found;
				}
				if(cur_index != (num_data_buffers_loaded & AVAILABLE_BUFFERS_MASK)) // & is faster than %, so the mask is setup for &
				{
					overflow_flag = 1;
					xil_printf("\n\rAlgorithm is too slow for real time...overflow_flag = %d\n\r", overflow_flag);
				}
			}
		}
	}
	vTaskDelete(NULL);
}


void vSetupBurstAnalysis(struct Burst_Analysis_Params *params)
{
	u8 portnum = 0;
	u8 bitnum = 0;
	u8 status = 0;

	//Initialize HPP with required control messages
	IdentifyDIOParams(params[0].ttl_output_bitnum, &portnum, &bitnum);

	params[0].portnum = portnum;
	params[0].bitnum = bitnum;

	//Assert PS control (as opposed to fabric control) over the commands sent to the motherboard
	status = SetPSControl(PS_Ctrl);
	if(status != 0)
	{
		xil_printf("\n\rProblem with SetPSControl(PS_Ctrl)");
	}
	else
	{
		xil_printf("\n\rSetPSControl(PS_Ctrl) successfully called.\n\r");
	}


	//Assert HPP control over MB TTL lines
	status = SetDIOCtrl(portnum, HPP_Ctrl);
	if(status != 0)
	{
		xil_printf("\n\rProblem with SetDigitalIOCtrl(%d, HPP_Ctrl), Status = %d", portnum, status);
	}

	//Set TTLs to outputs
	status = SetDIOPortDir(portnum, DIO_PORT_DIR_OUTPUT);
	if(status != 0)
	{
		xil_printf("\n\rProblem with SetDigitalIOPortDir(%d, DIO_PORT_DIR_OUTPUT), Status = %d", portnum, status);
	}


	SetPSReady(PS_Ctrl);
}


void vNeuralEnsembles(void *params)
{
	u32 i = 0;
	u32 j = 0;
	u32 k = 0;
	u32 m = 0;
	u32 bit_num = 0;
	u32 led_num = 0;
	u32 cur_index = 0;
	u32 oldest_allowable_timestamp = 0;
	u16 ensemble_found = 0;
	u32 most_recent_matches[512]; //Used to keep track of only the most recent matches per channel
	u8 templates_matched[512][32]; //Used to keep track of how many matches occur per template, per channel
	u32 most_matches = 0; //Testing
	//BaseType_t xReturn;
	u32 num_ensembles = 0;

	struct Neural_Ensemble_Params *parameters;

	parameters = (struct Neural_Ensemble_Params *) params;

	vSetupNeuralEnsembles(&parameters[0]);

	bit_num = 0x000000FF & parameters[0].ttl_output_bitnum;

	for(;;)
	{
		if(xHPP_Neural_Ensemble_Sem != NULL)
		{
			if(xSemaphoreTake(xHPP_Neural_Ensemble_Sem, portMAX_DELAY ) == pdTRUE)
			{
				if(num_data_buffers_loaded > 31)
				{
					cur_index = (num_data_buffers_loaded - 32) & AVAILABLE_BUFFERS_MASK; // & is faster than %, so the mask is setup for &

					for(i = 0; i < 32; i++)  //For loop to go through each template point
					{
						for(j = 0; j < parameters[0].num_templates; j++) //For loop to go through each template
						{
							for(k = 0; k < parameters[0].num_channels; k++) //For loop to go through each channel to be compared to the template
							{
								if(i == 0)
								{
									templates_matched[k][j] = 0; //Ensure match count is cleared ahead of time...
								}
								if(HPP_Data[cur_index].AD[parameters[0].ch_to_match[k]] > low_templates[j][i])
								{
									templates_matched[k][j] = templates_matched[k][j] + 1; //Increase our count if the data is higher than the low boundary
								}
								if(HPP_Data[cur_index].AD[parameters[0].ch_to_match[k]] < high_templates[j][i])
								{
									templates_matched[k][j] = templates_matched[k][j] + 1; //Increase our count if the data is lower than the high boundary
								}
								if(templates_matched[k][j] > 62)
								{
									most_recent_matches[parameters[0].ch_to_match[k]] = HPP_Data[cur_index].TimeStamp_Low;
									oldest_allowable_timestamp = HPP_Data[cur_index].TimeStamp_Low - parameters[0].ensemble_duration;
									ensemble_found = 0;

									for(m = 0; m < parameters[0].num_channels; m++)
									{
										if(most_recent_matches[parameters[0].ch_to_match[m]] > oldest_allowable_timestamp)
										{
											ensemble_found++;
										}
									}
									if(ensemble_found >= parameters[0].num_channels)
									{
										//xReturn = xTaskCreate(vToggleDigIO, "vToggleDigIO", 5000, &bit_num, 5, NULL);
										//xReturn = xTaskCreate(vToggleLED, "vToggleLED", 5000, &led_num, 5, NULL);
										ToggleDigIO(bit_num);
										ToggleLED(led_num);
										num_ensembles++;
									}
									if(ensemble_found > most_matches)
									{
										most_matches = ensemble_found;
									}
								}
								//For testing
								if(HPP_Data[cur_index].AD[parameters[0].ch_to_match[k]] > 10000)
								{
							//		xil_printf("Amplitude of HPP_Data[cur_index].AD[parameters[0].ch_to_match[k]] is %d", HPP_Data[cur_index].AD[parameters[0].ch_to_match[k]]);
								}
							}
						}
						cur_index = (cur_index + 1) & AVAILABLE_BUFFERS_MASK; // & is faster than %, so the mask is setup for &
					}
				}
			}
		}
	}
	vTaskDelete(NULL);
}


void vSetupNeuralEnsembles(struct Neural_Ensemble_Params *params)
{
	u8	i = 0;
	u8 j = 0;

	u8 portnum = 0;
	u8 bitnum = 0;
	u8 status = 0;

	//Initialize Templates
	for(i = 0; i < 32; i++) //Templates are fixed at 32 points in length
	{
		for(j = 0; j < params[0].num_templates; j++) //Only loop through the number of templates to be used
		{
			low_templates[j][i] = params[0].templates[(j<<5) + i];
			high_templates[j][i] = params[0].templates[(j<<5) + i + 1024];
		}
	}

	//Initialize HPP with required control messages
	IdentifyDIOParams(params[0].ttl_output_bitnum, &portnum, &bitnum);

	params[0].portnum = portnum;
	params[0].bitnum = bitnum;

	//Assert PS control (as opposed to fabric control) over the commands sent to the motherboard
	status = SetPSControl(PS_Ctrl);
	if(status != 0)
	{
		xil_printf("\n\rProblem with SetPSControl(PS_Ctrl)");
	}
	else
	{
		xil_printf("\n\rSetPSControl(PS_Ctrl) successfully called.\n\r");
	}


	//Assert HPP control over MB TTL lines
	status = SetDIOCtrl(portnum, HPP_Ctrl);
	if(status != 0)
	{
		xil_printf("\n\rProblem with SetDigitalIOCtrl(%d, HPP_Ctrl), Status = %d", portnum, status);
	}

	//Set TTLs to outputs
	status = SetDIOPortDir(portnum, DIO_PORT_DIR_OUTPUT);
	if(status != 0)
	{
		xil_printf("\n\rProblem with SetDigitalIOPortDir(%d, DIO_PORT_DIR_OUTPUT), Status = %d", portnum, status);
	}

	//Assert HPP control over LEDs
	status = SetFPLEDCtrl(HPP_Ctrl);
	if(status != 0)
	{
		xil_printf("\n\rProblem with SetFPLEDCtrl(PS_Ctrl), Status = %d", status);
	}


	SetPSReady(PS_Ctrl);
}


void vSpikeDetect(void *params)
{
	u32 i = 0;
	u32 j = 0;
	u32 k = 0;
	u32 cur_index = 0;
	u8 templates_matched[512][32]; //Used to keep track of how many matches occur per template, per channel

	XTime start_time = 0;
	XTime stop_time = 0;
	u32 start_high = 0;
	u32 start_low = 0;
	u32 stop_high = 0;
	u32 stop_low = 0;
	u32 time_taken_high = 0;
	u32 time_taken_low = 0;
	u32 num_buffers_loaded_at_start = 0;
	u32 num_buffers_loaded_at_match = 0;

	struct Spike_Detect_Params *parameters;

	parameters = (struct Spike_Detect_Params *) params;

	vSetupSpikeDetect(&parameters[0]);

	for(;;)
	{
		stop_time = 0;
		XTime_GetTime(&start_time);

		if(xHPP_Spike_Detect_Sem != NULL)
		{
			if(xSemaphoreTake(xHPP_Spike_Detect_Sem, portMAX_DELAY) == pdTRUE)
			{
				if(num_data_buffers_loaded > 31)
				{
					cur_index = (num_data_buffers_loaded - 32) & AVAILABLE_BUFFERS_MASK; // & is faster than %, so the mask is setup for &
					num_buffers_loaded_at_start = num_data_buffers_loaded;

					for(i = 0; i < 32; i++)  //For loop to go through each template point
					{
						for(j = 0; j < parameters[0].num_templates; j++) //For loop to go through each template
						{
							for(k = 0; k < parameters[0].num_channels; k++) //For loop to go through each channel to be compared to the template
							{
								if(i == 0)
								{
									templates_matched[k][j] = 0;
								}
								if(HPP_Data[cur_index].AD[parameters[0].ch_to_match[k]] > low_templates[j][i])
								{
									templates_matched[k][j] = templates_matched[k][j] + 1;
								}
								if(HPP_Data[cur_index].AD[parameters[0].ch_to_match[k]] < high_templates[j][i])
								{
									templates_matched[k][j] = templates_matched[k][j] + 1;
								}
								if(templates_matched[k][j] > 62)
								{
									//xTaskCreate(vToggleDigIO, "vToggleDigIO", 5000, (void *)&parameters[0].ttl_output_bitnum, configMAX_PRIORITIES-1, NULL);
									ToggleDigIO(parameters[0].ttl_output_bitnum);
									XTime_GetTime(&stop_time);
									num_buffers_loaded_at_match = num_data_buffers_loaded;
									xil_printf("\n\rTimestamp_high at match = %d", HPP_Data[cur_index].TimeStamp_High);
									xil_printf("\n\rTimestamp_low at match = %d", HPP_Data[cur_index].TimeStamp_Low);
									xil_printf("\n\rcur_index at match = %d", cur_index);
									xil_printf("\n\rnum_buffers_loaded_at_start = %d", num_buffers_loaded_at_start);
									xil_printf("\n\rnum_buffers_loaded_at_match = %d", num_buffers_loaded_at_match);
								}
							}
						}
						cur_index = (cur_index + 1) & AVAILABLE_BUFFERS_MASK;
					}
				}
			}
		}
		if(stop_time != 0)
		{
			start_high = (u32)(start_time >> 32);
			start_low = (u32)start_time;
			stop_high = (u32)(stop_time >> 32);
			stop_low = (u32)stop_time;
			time_taken_high = stop_high - start_high;
			time_taken_low = stop_low - start_low;
			xil_printf("\n\rTime taken for Spike_Detect = %d (high) %d (low)", time_taken_high, time_taken_low);
			time_taken_high = 0;
			time_taken_low = 0;
		}
	}
	vTaskDelete(NULL);
}


void vSetupSpikeDetect(struct Spike_Detect_Params *params)
{
	u8	i = 0;
	u8 j = 0;

	u8 portnum = 0;
	u8 bitnum = 0;
	u8 status = 0;

	//Initialize Templates
	for(i = 0; i < 32; i++) //Templates are fixed at 32 points in length
	{
		for(j = 0; j < params[0].num_templates; j++) //Only loop through the number of templates to be used
		{
			low_templates[j][i] = params[0].templates[(j<<5) + i];
			high_templates[j][i] = params[0].templates[(j<<5) + i + 1024];
		}
	}

	//Initialize HPP with required control messages
	IdentifyDIOParams(params[0].ttl_output_bitnum, &portnum, &bitnum);

	params[0].portnum = portnum;
	params[0].bitnum = bitnum;

	//Assert PS control (as opposed to fabric control) over the commands sent to the motherboard
	status = SetPSControl(PS_Ctrl);
	if(status != 0)
	{
		xil_printf("\n\rProblem with SetPSControl(PS_Ctrl)");
	}
	else
	{
		xil_printf("\n\rSetPSControl(PS_Ctrl) successfully called.\n\r");
	}


	//Assert HPP control over MB TTL lines
	status = SetDIOCtrl(portnum, HPP_Ctrl);
	if(status != 0)
	{
		xil_printf("\n\rProblem with SetDigitalIOCtrl(%d, HPP_Ctrl), Status = %d", portnum, status);
	}

	//Set TTLs to outputs
	status = SetDIOPortDir(portnum, DIO_PORT_DIR_OUTPUT);
	if(status != 0)
	{
		xil_printf("\n\rProblem with SetDigitalIOPortDir(%d, DIO_PORT_DIR_OUTPUT), Status = %d", portnum, status);
	}



	//Added for Cheetah Debug
	//Assert HPP control over MB TTL lines
	status = SetDIOCtrl(1, HPP_Ctrl);
	if(status != 0)
	{
		xil_printf("\n\rProblem with SetDigitalIOCtrl(%d, HPP_Ctrl), Status = %d", portnum, status);
	}
	//Set TTLs to outputs
	status = SetDIOPortDir(1, DIO_PORT_DIR_OUTPUT);
	if(status != 0)
	{
		xil_printf("\n\rProblem with SetDigitalIOPortDir(%d, DIO_PORT_DIR_OUTPUT), Status = %d", portnum, status);
	}



	//For SfN Demos
	for(i = 0; i < 4; i++)
	{
		//Assert HPP control over MB TTL lines
		status = SetDIOCtrl(i, HPP_Ctrl);
		if(status != 0)
		{
			xil_printf("\n\rProblem with SetDigitalIOCtrl(%d, HPP_Ctrl), Status = %d", i, status);
		}

		//Set TTLs to outputs
		status = SetDIOPortDir(i, DIO_PORT_DIR_OUTPUT);
		if(status != 0)
		{
			xil_printf("\n\rProblem with SetDigitalIOPortDir(%d, DIO_PORT_DIR_OUTPUT), Status = %d", i, status);
		}
	}
	xil_printf("\n\rAll digital I/O ports setup with HPP control and as outputs\n\r");

	SetPSReady(PS_Ctrl);
}


void vPrintTimeStamp()
{
	u32 cur_index = 0;
	cur_index = (num_data_buffers_loaded - 1) & AVAILABLE_BUFFERS_MASK;
	xil_printf("Num data buffers loaded %d\n\r", num_data_buffers_loaded);
	xil_printf("\n\rCurrent TimeStamp = 0x%x%x", HPP_Data[cur_index].TimeStamp_High, HPP_Data[cur_index].TimeStamp_Low);
}

/* Definitions for interface TIMESTAMP */
//KBE??? removed in VHDL
#define XPAR_TIMESTAMP_BASEADDR 0x83C10000
#define XPAR_TIMESTAMP_HIGHADDR 0x83C1FFFF

void vTimeStampCheck()
{
	u32 last_index = 0;
	u32 cur_index = 0;
	int difference = 0;
	u64 cur_timestamp = 0;
	u64 temp_timestamp_high = 0;
	u32 num_ints = 0;
	int ints_diff = 0;
	//u32 dir = 0;

	vSetupTimeStampCheck();

	for(;;)
	{
		if(xHPP_Data_Sem != NULL)
		{
			if(xSemaphoreTake(xHPP_Data_Sem, portMAX_DELAY) == pdTRUE)
			{
				// Added for debug only STM 1Oct14
				//Xil_Out32(XPAR_TIMESTAMP_BASEADDR, HPP_Data[num_data_buffers_loaded - 1].TimeStamp_High);
				//Xil_Out32(XPAR_TIMESTAMP_BASEADDR, HPP_Data[num_data_buffers_loaded - 1].TimeStamp_Low);
				cur_index = (num_data_buffers_loaded - 1) & AVAILABLE_BUFFERS_MASK;
				temp_timestamp_high = ((u64)HPP_Data[cur_index].TimeStamp_High << 32) & 0xFFFFFFFF00000000;
				cur_timestamp = temp_timestamp_high | ((u64)HPP_Data[cur_index].TimeStamp_Low & 0x00000000FFFFFFFF);
				//memcpy((int *)XPAR_TIMESTAMP_BASEADDR, &cur_timestamp, sizeof(cur_timestamp));
				//KBE!!!
				memcpy((int *)XPAR_TIMESTAMP_BASEADDR, &cur_timestamp, 8);



				if(num_data_buffers_loaded > 50)
				{
					last_index = (num_data_buffers_loaded - 2) & AVAILABLE_BUFFERS_MASK; // & is faster than %, so the mask is setup for &
					cur_index = (num_data_buffers_loaded - 1) & AVAILABLE_BUFFERS_MASK; // & is faster than %, so the mask is setup for &
					difference = HPP_Data[cur_index].TimeStamp_Low - HPP_Data[last_index].TimeStamp_Low;
					if(abs(difference) > 34)
					{
						//ToggleDigIO(12);
						//dir = XGpioPs_GetDirection(&xGpiops, GPIO_BANK_3);
						//XGpioPs_SetDirection(&xGpiops, GPIO_BANK_3, 0);
						//num_ints = XGpioPs_Read(&xGpiops, GPIO_BANK_3);
						//XGpioPs_SetDirection(&xGpiops, GPIO_BANK_3, dir);
						//XGpioPs_Write(&xGpiops, GPIO_BANK_3, gpio_bank3);
						num_ints = num_data_interrupts[0];
						ints_diff = num_ints - num_data_buffers_loaded;
						xil_printf("\n\rERROR: MISMATCHED TIMESTAMPS -> cur timestamp = %d, last timestamp = %d, diff = %d, num_buffers_loaded = %d",
									HPP_Data[cur_index].TimeStamp_Low, HPP_Data[last_index].TimeStamp_Low, abs(difference), num_data_buffers_loaded);
						xil_printf("\n\rNumber of PL interrupts = %d, num_data_buffers_loaded = %d, difference = %d\n\r",
									num_ints, num_data_buffers_loaded, abs(ints_diff));
					}
				}
			}
		}
	}
	vTaskDelete(NULL);
}


void vSetupTimeStampCheck()
{
	u8 portnum = 0;
	u8 status = 0;

	//Assert PS control (as opposed to fabric control) over the commands sent to the motherboard
	status = SetPSControl(PS_Ctrl);
	if(status != 0)
	{
		xil_printf("\n\rProblem with SetPSControl(PS_Ctrl)");
	}
	else
	{
		xil_printf("\n\rSetPSControl(PS_Ctrl) successfully called.\n\r");
	}


	//Assert HPP control over MB TTL lines
	status = SetDIOCtrl(portnum, HPP_Ctrl);
	if(status != 0)
	{
		xil_printf("\n\rProblem with SetDigitalIOCtrl(%d, HPP_Ctrl), Status = %d", portnum, status);
	}

	//Set TTLs to outputs
	status = SetDIOPortDir(portnum, DIO_PORT_DIR_OUTPUT);
	if(status != 0)
	{
		xil_printf("\n\rProblem with SetDigitalIOPortDir(%d, DIO_PORT_DIR_OUTPUT), Status = %d", portnum, status);
	}

	portnum = 1;
	//Assert HPP control over MB TTL lines
	status = SetDIOCtrl(portnum, HPP_Ctrl);
	if(status != 0)
	{
		xil_printf("\n\rProblem with SetDigitalIOCtrl(%d, HPP_Ctrl), Status = %d", portnum, status);
	}

	//Set TTLs to outputs
	status = SetDIOPortDir(portnum, DIO_PORT_DIR_OUTPUT);
	if(status != 0)
	{
		xil_printf("\n\rProblem with SetDigitalIOPortDir(%d, DIO_PORT_DIR_OUTPUT), Status = %d", portnum, status);
	}


	SetPSReady(PS_Ctrl);
}

//Initializes all necessary components needed to successfully run vRealTimeVerification
void vSetup_Template_Matching_Demo(void *pvParameters)
{
	s8 status = 0;
	u16 ind = 0;
	(void) pvParameters;


	//Assert PS control (as opposed to fabric control) over the commands sent to the motherboard
	status = SetPSControl(PS_Ctrl);
	if(status != 0)
	{
		xil_printf("\n\rProblem with SetPSControl(PS_Ctrl)");
	}
	else
	{
		xil_printf("\n\rSetPSControl(PS_Ctrl) successfully called.\n\r");
	}


	for(ind = 0; ind < 4; ind++)
	{
		//Assert HPP control over MB TTL lines
		status = SetDIOCtrl(ind, HPP_Ctrl);
		if(status != 0)
		{
			xil_printf("\n\rProblem with SetDigitalIOCtrl(%d, HPP_Ctrl), Status = %d", ind, status);
		}

		//Set TTLs to outputs
		status = SetDIOPortDir(ind, DIO_PORT_DIR_OUTPUT);
		if(status != 0)
		{
			xil_printf("\n\rProblem with SetDigitalIOPortDir(%d, DIO_PORT_DIR_OUTPUT), Status = %d", ind, status);
		}
	}

	//Assume LED Control
	status = SetFPLEDCtrl(HPP_Ctrl);
	if(status != 0)
	{
		xil_printf("\n\rProblem with SetFPLEDCtrl(LED_S1), Status = %d", status);
	}

	//Toggle each LED once for verification of functionality before template matching
	status = SetFPLED(LED_S1, LED_On);
	if(status != 0)
	{
		xil_printf("\n\rProblem with SetFPLED(LED_S1, LED_On), Status = %d", status);
	}


	status = SetFPLED(LED_S1, LED_Off);
	if(status != 0)
	{
		xil_printf("\n\rProblem with SetFPLED(LED_S1, LED_Off), Status = %d", status);
	}


	status = SetFPLED(LED_S2, LED_On);
	if(status != 0)
	{
		xil_printf("\n\rProblem with SetFPLED(LED_S2, LED_On), Status = %d", status);
	}


	status = SetFPLED(LED_S2, LED_Off);
	if(status != 0)
	{
		xil_printf("\n\rProblem with SetFPLED(LED_S2, LED_Off), Status = %d", status);
	}


	SetPSReady(PS_Ctrl);

	vTaskDelete(NULL);
}


void vRealTimeVerification(void *pvParameters)
{
	// Remove compile warnings if configASSERT() is not defined.
	(void) pvParameters;
	//u32 dio_values = 0;
	//u64 temp_timestamp = 0;
	//u32 timestamp_lower = 0;
	//u32 timestamp_upper = 0;
	u32 index = 0;
	//s8 status = 0;
	//u32 success = 0;
	//u32 address = 0;
	//BaseType_t xReturn;
	//u32 cur_index = 0;
	u8 first_time_through = 0;

	for(;;)
	{
		if(xHPP_Data_Sem != NULL)
		{
			if(xSemaphoreTake( xHPP_Data_Sem, portMAX_DELAY ) == pdTRUE)
			{
				/*
				if(num_data_buffers_loaded > 31)
				{
					success = 0;

					cur_index = (num_data_buffers_loaded - 32) & AVAILABLE_BUFFERS_MASK; // & is faster than %, so the mask is setup for &

					for(index = 0; index < 32; index++)
					{
						if(HPP_Data[cur_index].AD[0] >= tm_low[index])
						{
							success++;
						}
						if(HPP_Data[cur_index].AD[0] <= tm_high[index])
						{
							success++;
						}

						cur_index = (cur_index + 1) & AVAILABLE_BUFFERS_MASK;

					}

					if(success > most_matches)
					{
						most_matches = success;
					}

					if(success >= 62)
					{
						led_s1_value ^= 1;

						status = SetFPLED(LED_S1, led_s1_value);
						if(status != 0)
						{
							xil_printf("\n\rProblem with SetFPLED(LED_S1, %d), Status = %d", led_s1_value, status);
						}

						xil_printf("\n\rTemplate Matched, num_buffers_loaded = %d!!!\n\r", num_data_buffers_loaded);
						address = (u32)&HPP_Data[cur_index];
						xil_printf("Cur_index = %d and the address of HPP_Data[%d] = %h\n\r", cur_index, cur_index, address);
					}
				}*/

				/*
				if(num_data_buffers_loaded == 1)
				{
					verify_hpp_data(0);
					xil_printf("\n\rAddress of HPP_Data = %p", HPP_Data);
					xil_printf("\n\rAddress of HPP_Data[0] = %p", HPP_Data[0]);
				}*/

				if(num_data_buffers_loaded > 100)
				{
					if(first_time_through == 0)
					{
						Xil_DCacheFlush();
						xil_printf("\n\r\n\r\n\r");
						for(index = 0; index < 100; index++)
						{
							verify_hpp_data(index);
							//temp_timestamp = HPP_Data[index].TimeStamp;
							//timestamp_lower = (u32)(temp_timestamp & 0x00000000FFFFFFFF);
							//temp_timestamp = temp_timestamp >> 32;
							//timestamp_upper = (u32)(temp_timestamp & 0x00000000FFFFFFFF);
							//xil_printf("HPP_Data[%d] - Timestamp: %d%d AD[0]: %d\n\r", index, timestamp_upper, timestamp_lower, HPP_Data[index].AD[0]);
						}
						first_time_through = 1;
					}
				}
			}
		}
	}
	vTaskDelete(NULL);
}




void vExperimentControl(void *parameters)
{
	u8 i = 0;
	u8 train_started = 0;
	u32 records_loaded = 0;
	struct Pulse_Train_Params *params;
	params = (struct Pulse_Train_Params *) parameters;
	u32 bitmask = 0x00000000;
	u8 portnum = 0;
	u8 bitnum = 0;
	s8 status = 0;
	u32 pulse_high = 0;
	u32 pulse_low = 0;
	u32 masked_bit = 0;

    vSemaphoreCreateBinary(xPulse_Train_Sem);
	if(xSemaphoreTake(xPulse_Train_Sem, portMAX_DELAY) != pdTRUE)
	{
		xil_printf("\n\rProblem with initial xSemaphoreTake of xPulse_Train_Sem\n\r");
	}

	IdentifyDIOParams(params[0].ttl_output_bitnum, &portnum, &bitnum);

	params[0].portnum = portnum;
	params[0].bitnum = bitnum;

	IdentifyDIOParams(params[0].ttl_input_bitnum, &portnum, &bitnum);

	bitmask = (1 << portnum) << bitnum;

	status = SetDIOCtrl(params[0].portnum, HPP_Ctrl); //Assert HPP control over DIO port "portnum"
	if(status != 0)
	{
		xil_printf("\n\rProbem with SetDIOCtrl(%d, HPP_Ctrl)", params[0].portnum);
	}

	status = SetDIOPortDir(params[0].portnum, DIO_PORT_DIR_OUTPUT); //Assert DIO port "portnum" as output
	if(status != 0)
	{
		xil_printf("\n\rProbem with SetDIOPortDir(%d, DIO_PORT_DIR_OUTPUT)", params[0].portnum);
	}

	status = SetDIOCtrl(0, HPP_Ctrl); // For verification, set port 0 to the PS having control over the bit
	if(status != 0)
	{
		xil_printf("\n\rProbem with SetDIOCtrl(0, HPP_Ctrl)");
	}

	status = SetDIOPortDir(0, DIO_PORT_DIR_OUTPUT); //For verification, set port 0 to output so the PS can set the value through the command line
	if(status != 0)
	{
		xil_printf("\n\rProbem with SetDIOPortDir(0, DIO_PORT_DIR_OUTPUT)");
	}


	//For verification, clear all DIO bits for these two ports
	status = SetDIOPortValue(params[0].portnum, 0);
	if(status != 0)
	{
		xil_printf("\n\rProbem with SetDIOPortValue(%d, 0)", params[0].portnum);
	}

	status = SetDIOPortValue(0, 0);
	if(status != 0)
	{
		xil_printf("\n\rProbem with SetDIOPortValue(0, 0)");
	}


	SetPSReady(PS_Ctrl);


	for(i = 0; i < 100; i++)
	{
		pulse_params.bitnum = bitnum;
		pulse_params.portnum = portnum;
		pulse_params.high_time_ms = pulse_high;
		pulse_params.low_time_ms = pulse_low;
		xTaskCreate(vPulseTrain, "PulseTrain", 5000, NULL, TASK_PRIORITY, NULL); // KBE???
	}

	pulse_high = params[0].high_time_ms;
	pulse_low = params[0].low_time_ms;
	bitnum = params[0].bitnum;
	portnum = params[0].portnum;

	for( ; ; )
	{
		if(xHPP_Data_Sem != NULL)
		{
			if(xSemaphoreTake( xHPP_Data_Sem, portMAX_DELAY ) == pdTRUE)
			{
				records_loaded = num_data_buffers_loaded & AVAILABLE_BUFFERS_MASK; // & is faster than %, so the mask is setup for &
				masked_bit = HPP_Data[records_loaded].TTL_Port_Values & bitmask;
				if(masked_bit == bitmask)
				{
					if(train_started == 0)
					{
						train_started = 1;

						xSemaphoreGive(xPulse_Train_Sem);
					}
				}
				else
				{
					train_started = 0;
				}
			}
		}
	}
}


//Commented out for SfN
/*
portBASE_TYPE Start_Spike_Detect(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
	(void)pcCommandString;
	(void)xWriteBufferLen;
	//(void)pcWriteBuffer;
	configASSERT(pcWriteBuffer);
	BaseType_t xReturn;

	if(xSpike_Detect_Running == 1)
	{
		sprintf(pcWriteBuffer,"\n\rSpike Detection is already running...\n\r");
	}
	else
	{
		xSpike_Detect_Running = 1;

		struct Spike_Detect_Params params;
		//	params.num_channels = 5;
		//	params.num_templates = 1;
		//	params.ch_to_match[0] = 1;
		//	params.ch_to_match[1] = 12;
		//	params.ch_to_match[2] = 29;
		//	params.ch_to_match[3] = 34;
		//	params.ch_to_match[4] = 42;
		params.num_channels = 1;
		params.num_templates = 1;
		params.ch_to_match[0] = 1;
		params.ttl_output_bitnum = 16;
		params.pulse_type = DIO_PULSE_HIGH;
		params.templates[0] = -40000;
		params.templates[1] = -40000;
		params.templates[2] = -40000;
		params.templates[3] = -40000;
		params.templates[4] = -40000;
		params.templates[5] = -40000;
		params.templates[6] = -40000;
		params.templates[7] = -40000;
		params.templates[8] = -40000;
		params.templates[9] = -40000;
		params.templates[10] = -40000;
		params.templates[11] = 0;
		params.templates[12] = 0;
		params.templates[13] = 0;
		params.templates[14] = -40000;
		params.templates[15] = -40000;
		params.templates[16] = -40000;
		params.templates[17] = -40000;
		params.templates[18] = -40000;
		params.templates[19] = -40000;
		params.templates[20] = -40000;
		params.templates[21] = -40000;
		params.templates[22] = -40000;
		params.templates[23] = -40000;
		params.templates[24] = -40000;
		params.templates[25] = -40000;
		params.templates[26] = -40000;
		params.templates[27] = -40000;
		params.templates[28] = -40000;
		params.templates[29] = -40000;
		params.templates[30] = -40000;
		params.templates[31] = -40000;

		u16 i = 0;

		for(i = 32; i < 1024; i++)
		{
			params.templates[i] = 0;
		}

		params.templates[1024] = 10000;
		params.templates[1025] = 10000;
		params.templates[1026] = 10000;
		params.templates[1027] = 10000;
		params.templates[1028] = 10000;
		params.templates[1029] = 10000;
		params.templates[1030] = 10000;
		params.templates[1031] = 10000;
		params.templates[1032] = 10000;
		params.templates[1033] = 0x7FFFFFFF;
		params.templates[1034] = 0x7FFFFFFF;
		params.templates[1035] = 0x7FFFFFFF;
		params.templates[1036] = 0x7FFFFFFF;
		params.templates[1037] = 0x7FFFFFFF;
		params.templates[1038] = 0x7FFFFFFF;
		params.templates[1039] = 0x7FFFFFFF;
		params.templates[1040] = 10000;
		params.templates[1041] = 10000;
		params.templates[1042] = 10000;
		params.templates[1043] = 10000;
		params.templates[1044] = 10000;
		params.templates[1045] = 10000;
		params.templates[1046] = 10000;
		params.templates[1047] = 10000;
		params.templates[1048] = 10000;
		params.templates[1049] = 10000;
		params.templates[1050] = 10000;
		params.templates[1051] = 10000;
		params.templates[1052] = 10000;
		params.templates[1053] = 10000;
		params.templates[1054] = 10000;
		params.templates[1055] = 10000;


		for(i = 1056; i < 2047; i++)
		{
			params.templates[i] = 0;
		}

		xReturn = xTaskCreate(vSpikeDetect, "vSpikeDetect", (uint16_t)50000, (void *)&params, 5, &xSpike_Detect_Handle);
		sprintf(pcWriteBuffer,"\n\rvSpikeDetect created with return value of %d\n\r", xReturn);
	}
	return pdFALSE; //No more lines for the command line to be generated
}*/


//For SfN
portBASE_TYPE Start_Spike_Detect(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
	(void)pcCommandString;
	(void)xWriteBufferLen;
	//(void)pcWriteBuffer;
	configASSERT(pcWriteBuffer);
	BaseType_t xReturn;

	if(xSpike_Detect_Running == 1)
	{
		sprintf(pcWriteBuffer,"\n\rSpike Detection is already running...\n\r");
	}
	else
	{
		xSpike_Detect_Running = 1;

		struct Spike_Detect_Params params;
		//	params.num_channels = 5;
		//	params.num_templates = 1;
		//	params.ch_to_match[0] = 1;
		//	params.ch_to_match[1] = 12;
		//	params.ch_to_match[2] = 29;
		//	params.ch_to_match[3] = 34;
		//	params.ch_to_match[4] = 42;
		params.num_channels = 1;
		params.num_templates = 1;
		params.ch_to_match[0] = 21;
		params.ttl_output_bitnum = 16;
		params.pulse_type = DIO_PULSE_HIGH;
		params.templates[0] = -586046;
		params.templates[1] = -669648;
		params.templates[2] = -631865;
		params.templates[3] = -428721;
		params.templates[4] = -256107;
		params.templates[5] = -175647;
		params.templates[6] = -106025;
		params.templates[7] = -37383;
		params.templates[8] = -87572;
		params.templates[9] = 101633;
		params.templates[10] = 294626;
		params.templates[11] = 744999;
		params.templates[12] = 1192454;
		params.templates[13] = 1792886;
		params.templates[14] = 2405768;
		params.templates[15] = 2314482;
		params.templates[16] = 1746162;
		params.templates[17] = 1039501;
		params.templates[18] = 431067;
		params.templates[19] = -320898;
		params.templates[20] = -872965;
		params.templates[21] = -1304172;
		params.templates[22] = -1819557;
		params.templates[23] = -2151848;
		params.templates[24] = -2319994;
		params.templates[25] = -2413141;
		params.templates[26] = -2461976;
		params.templates[27] = -2443916;
		params.templates[28] = -2372748;
		params.templates[29] = -2254268;
		params.templates[30] = -2062515;
		params.templates[31] = -1809681;

		u16 i = 0;

		for(i = 32; i < 1024; i++)
		{
			params.templates[i] = 0;
		}

		params.templates[1024] =33670;
		params.templates[1025] = 30538;
		params.templates[1026] = 107949;
		params.templates[1027] = 235262;
		params.templates[1028] = 339361;
		params.templates[1029] = 415452;
		params.templates[1030] = 595308;
		params.templates[1031] = 793833;
		params.templates[1032] = 977958;
		params.templates[1033] = 1175596;
		params.templates[1034] = 1472405;
		params.templates[1035] = 1926422;
		params.templates[1036] = 2440464;
		params.templates[1037] = 2784308;
		params.templates[1038] = 2886502;
		params.templates[1039] = 2888979;
		params.templates[1040] = 2798219;
		params.templates[1041] = 2365098;
		params.templates[1042] = 1431863;
		params.templates[1043] = 568481;
		params.templates[1044] = -218852;
		params.templates[1045] = -624324;
		params.templates[1046] = -860763;
		params.templates[1047] = -992077;
		params.templates[1048] = -1079144;
		params.templates[1049] = -1197416;
		params.templates[1050] = -1321813;
		params.templates[1051] = -1239608;
		params.templates[1052] = -1033883;
		params.templates[1053] = -884824;
		params.templates[1054] = -787455;
		params.templates[1055] = -630733;


		for(i = 1056; i < 2047; i++)
		{
			params.templates[i] = 0;
		}

		xReturn = xTaskCreate(vSpikeDetect, "vSpikeDetect", (uint16_t)50000, (void *)&params, TASK_PRIORITY, &xSpike_Detect_Handle);
		sprintf(pcWriteBuffer,"\n\rvSpikeDetect created with return value of %d\n\r", (int)xReturn);
	}
	return pdFALSE; //No more lines for the command line to be generated
}

portBASE_TYPE Stop_Spike_Detect(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
	(void)pcCommandString;
	(void)xWriteBufferLen;
	configASSERT(pcWriteBuffer);
	if(xSpike_Detect_Running == 1)
	{
		vTaskDelete(xSpike_Detect_Handle);
		sprintf(pcWriteBuffer,"\n\rvSpikeDetect deleted\n\r");
	}
	else
	{
		sprintf(pcWriteBuffer,"\n\rvSpikeDetect was not running\n\r");
	}

	xSpike_Detect_Running = 0;

	return pdFALSE; //No more lines for the command line to be generated
}


portBASE_TYPE Start_Ensemble_Detect(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
	(void)pcCommandString;
	(void)xWriteBufferLen;
	configASSERT(pcWriteBuffer);
	BaseType_t xReturn;


	if(xEnsemble_Detect_Running == 1)
	{
		sprintf(pcWriteBuffer,"\n\rEnsemble Detection is already running...\n\r");
	}
	else
	{
		u16 i = 0;
		xEnsemble_Detect_Running = 1;

		struct Neural_Ensemble_Params params;
		params.num_channels = 5;
		params.num_templates = 2;
		params.ch_to_match[0] = 1;
		params.ch_to_match[1] = 12;
		params.ch_to_match[2] = 29;
		params.ch_to_match[3] = 34;
		params.ch_to_match[4] = 42;
		params.ttl_output_bitnum = 7;
		params.pulse_type = DIO_PULSE_HIGH;
		params.ensemble_duration = 40000;
		params.templates[0] = -40000;
		params.templates[1] = -40000;
		params.templates[2] = -40000;
		params.templates[3] = -40000;
		params.templates[4] = -40000;
		params.templates[5] = -40000;
		params.templates[6] = -40000;
		params.templates[7] = -40000;
		params.templates[8] = -40000;
		params.templates[9] = -40000;
		params.templates[10] = -40000;
		params.templates[11] = 0;
		params.templates[12] = 0;
		params.templates[13] = 0;
		params.templates[14] = -40000;
		params.templates[15] = -40000;
		params.templates[16] = -40000;
		params.templates[17] = -40000;
		params.templates[18] = -40000;
		params.templates[19] = -40000;
		params.templates[20] = -40000;
		params.templates[21] = -40000;
		params.templates[22] = -40000;
		params.templates[23] = -40000;
		params.templates[24] = -40000;
		params.templates[25] = -40000;
		params.templates[26] = -40000;
		params.templates[27] = -40000;
		params.templates[28] = -40000;
		params.templates[29] = -40000;
		params.templates[30] = -40000;
		params.templates[31] = -40000;

		for(i = 32; i < 64; i++)
		{
			params.templates[i] = 0;
		}

		for(i = 64; i < 1024; i++)
		{
			params.templates[i] = 0;
		}

		params.templates[1024] = 10000;
		params.templates[1025] = 10000;
		params.templates[1026] = 10000;
		params.templates[1027] = 10000;
		params.templates[1028] = 10000;
		params.templates[1029] = 10000;
		params.templates[1030] = 10000;
		params.templates[1031] = 10000;
		params.templates[1032] = 10000;
		params.templates[1033] = 0x7FFFFFFF;
		params.templates[1034] = 0x7FFFFFFF;
		params.templates[1035] = 0x7FFFFFFF;
		params.templates[1036] = 0x7FFFFFFF;
		params.templates[1037] = 0x7FFFFFFF;
		params.templates[1038] = 0x7FFFFFFF;
		params.templates[1039] = 0x7FFFFFFF;
		params.templates[1040] = 10000;
		params.templates[1041] = 10000;
		params.templates[1042] = 10000;
		params.templates[1043] = 10000;
		params.templates[1044] = 10000;
		params.templates[1045] = 10000;
		params.templates[1046] = 10000;
		params.templates[1047] = 10000;
		params.templates[1048] = 10000;
		params.templates[1049] = 10000;
		params.templates[1050] = 10000;
		params.templates[1051] = 10000;
		params.templates[1052] = 10000;
		params.templates[1053] = 10000;
		params.templates[1054] = 10000;
		params.templates[1055] = 10000;


		for(i = 1056; i < 1088; i++)
		{
			params.templates[i] = 0x7FFFFFFF;
		}



		for(i = 1088; i < 2047; i++)
		{
			params.templates[i] = 0;
		}

		xReturn = xTaskCreate(vNeuralEnsembles, "vNeuralEnsembles", (uint16_t)65535, (void *)&params, TASK_PRIORITY, &xEnsemble_Detect_Handle);
		sprintf(pcWriteBuffer,"vNeuralEnsembles created with return value of %d\n\r", (int)xReturn);
	}
	return pdFALSE; //No more lines for the command line to be generated
}


portBASE_TYPE Stop_Ensemble_Detect(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
	(void)pcCommandString;
	(void)xWriteBufferLen;
	configASSERT(pcWriteBuffer);
	if(xEnsemble_Detect_Running == 1)
	{
		vTaskDelete(xEnsemble_Detect_Handle);
		sprintf(pcWriteBuffer,"\n\rvNeuralEnsembles deleted\n\r");
	}
	else
	{
		sprintf(pcWriteBuffer,"\n\rvNeuralEnsembles was not running\n\r");
	}

	xEnsemble_Detect_Running = 0;

	return pdFALSE; //No more lines for the command line to be generated
}

portBASE_TYPE Start_Burst_Analysis(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
	(void)pcCommandString;
	(void)xWriteBufferLen;
	configASSERT(pcWriteBuffer);
	u32 i = 0;
	BaseType_t xReturn;


	if(xBurst_Analysis_Running == 1)
	{
		sprintf(pcWriteBuffer,"\n\rBurst Analysis is already running...\n\r");
	}
	else
	{
		xBurst_Analysis_Running = 1;
		struct Burst_Analysis_Params params;
		params.num_channels = 8;
		//Timestamp units are microseconds and we need the burst to occur within 0.02 seconds (20000us)
		params.duration = 20000;
		params.number_of_bursts = 4;
		params.ttl_output_bitnum = 10;
		params.threshold = 10000;
		params.pulse_type = 1;

		for(i = 0; i < params.num_channels; i++)
		{
			params.ch_to_match[i] = i;
		}

		xReturn = xTaskCreate(vBurstAnalysis, "vBurstAnalysis", (uint16_t)65535, (void *)&params, TASK_PRIORITY, (void**)xBurst_Analysis_Handle);
		sprintf(pcWriteBuffer,"vBurstAnalysis created with return value of %d\n\r", (int)xReturn);
	}
	return pdFALSE; //No more lines for the command line to be generated
}


portBASE_TYPE Stop_Burst_Analysis(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
	(void)pcCommandString;
	(void)xWriteBufferLen;
	configASSERT(pcWriteBuffer);

	if(xBurst_Analysis_Running == 1)
	{
		vTaskDelete(xBurst_Analysis_Handle);
		sprintf(pcWriteBuffer,"\n\rvBurstAnalysis deleted\n\r");
	}
	else
	{
		sprintf(pcWriteBuffer,"\n\rvBurstAnalysis was not running\n\r");
	}

	xBurst_Analysis_Running = 0;

	return pdFALSE; //No more lines for the command line to be generated
}


portBASE_TYPE Start_PL_Demos(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
	(void)pcCommandString;
	(void)xWriteBufferLen;
	configASSERT(pcWriteBuffer);
	u32 temp = 0;
	u8 i = 0;
	s8 status = 0;



	//For SfN Demos
	for(i = 0; i < 4; i++)
	{
		//Assert HPP control over MB TTL lines
		status = SetDIOCtrl(i, HPP_Ctrl);
		if(status != 0)
		{
			xil_printf("\n\rProblem with SetDigitalIOCtrl(%d, HPP_Ctrl), Status = %d", i, status);
		}

		//Set TTLs to outputs
		status = SetDIOPortDir(i, DIO_PORT_DIR_OUTPUT);
		if(status != 0)
		{
			xil_printf("\n\rProblem with SetDigitalIOPortDir(%d, DIO_PORT_DIR_OUTPUT), Status = %d", i, status);
		}
	}
	xil_printf("\n\rAll digital I/O ports setup with HPP control and as outputs\n\r");

	//SetPSControl(Logic_Ctrl);

	temp = 1 <<	PL_DEMO_CMD_START;

	gpio_bank2 |= temp;
	XGpioPs_Write(&xGpiops, GPIO_BANK_2, gpio_bank2);

	usleep(250000);

	temp = 1 <<	PL_DEMO_START;

	gpio_bank2 |= temp;
	XGpioPs_Write(&xGpiops, GPIO_BANK_2, gpio_bank2);

	sprintf(pcWriteBuffer,"\n\rPL demos have been started\n\r");

	return pdFALSE; //No more lines for the command line to be generated
}

portBASE_TYPE Stop_PL_Demos(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
	(void)pcCommandString;
	(void)xWriteBufferLen;
	configASSERT(pcWriteBuffer);
	u32 temp = 0;

	SetPSControl(PS_Ctrl);

	temp = 1 <<	PL_DEMO_CMD_START;
	temp |= 1 << PL_DEMO_START;

	temp ^= temp;

	gpio_bank2 &= temp;
	XGpioPs_Write(&xGpiops, GPIO_BANK_2, gpio_bank2);

	sprintf(pcWriteBuffer,"\n\rPL demos have been stopped\n\r");

	return pdFALSE; //No more lines for the command line to be generated
}


portBASE_TYPE exp_ctrl_test(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
	(void)pcCommandString;
	(void)xWriteBufferLen;
	configASSERT(pcWriteBuffer);
	s8 status = 0;

	status = SetDIOPortBit(0, 0, Bit_High);
	if(status != 0)
	{
		sprintf(pcWriteBuffer,"\n\rProblem setting output bit 0 high.");
	}
	vTaskDelete(NULL);

	sprintf(pcWriteBuffer,"\n\rExperiment control verification in progress\n\r");

	return pdFALSE; //No more lines for the command line to be generated
}



portBASE_TYPE print_num_ints(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
	(void)pcCommandString;
	(void)xWriteBufferLen;
	configASSERT(pcWriteBuffer);

	sprintf(pcWriteBuffer, "\n\rNum_Data_Buffers_Loaded = %d", (int)num_data_buffers_loaded);

	//vTaskDelete(NULL);

	return pdFALSE; //No more lines for the command line to be generated
}



void verify_hpp_data(int channel_buf_num)
{
	//u64 temp_timestamp = 0;
	u32 timestamp_lower = 0;
	u32 timestamp_upper = 0;


	if((channel_buf_num < 0) || (channel_buf_num > 511))
	{
		xil_printf("\n\rChannel_buf_num %d is outside of the bounds of 0 through 511.\n\r", channel_buf_num);
	}
	else
	{
		int anindex = 0;
		int adindex = 0;
		/*temp_timestamp = HPP_Data[channel_buf_num].TimeStamp;
		timestamp_lower = (u32)(temp_timestamp & 0x00000000FFFFFFFF);
		temp_timestamp = temp_timestamp >> 32;
		timestamp_upper = (u32)(temp_timestamp & 0x00000000FFFFFFFF);*/

		timestamp_lower = HPP_Data[channel_buf_num].TimeStamp_Low;
		timestamp_upper = HPP_Data[channel_buf_num].TimeStamp_High;

		xil_printf("Verifying Channel Data for Buffer Number %d\n\r", channel_buf_num);
		xil_printf(".....................................\n\r");
		xil_printf("HPP_Data[%d].TimeStamp (Upper) = 0x%08X\n\r",channel_buf_num, timestamp_upper);
		xil_printf("HPP_Data[%d].TimeStamp (Lower) = 0x%08X\n\r",channel_buf_num, timestamp_lower);
		xil_printf("HPP_Data[%d].TTL_Port_Values = 0x%08X\n\r",channel_buf_num, HPP_Data[channel_buf_num].TTL_Port_Values);
		for(anindex = 0; anindex < 4; anindex++)
		{
			xil_printf("HPP_Data[%d].Analog_Output_Data[%d] = 0x%08X\n\r", channel_buf_num, anindex, HPP_Data[channel_buf_num].Analog_Output_Data[anindex]);
		}
		for(adindex = 0; adindex < 32; adindex++)
		{
			xil_printf("HPP_Data[%d].AD[%d] = 0x%08X\n\r",channel_buf_num, adindex, HPP_Data[channel_buf_num].AD[adindex]);
		}
		xil_printf(".....................................\n\r");
		xil_printf("End Verifying Channel Data for Buffer Number %d\n\n\n\r", channel_buf_num);
	}
}



void vinit_templates(void)
{
	tm_low[0] = -40000;
	tm_low[1] = -40000;
	tm_low[2] = -40000;
	tm_low[3] = -40000;
	tm_low[4] = -40000;
	tm_low[5] = -40000;
	tm_low[6] = -40000;
	tm_low[7] = -40000;
	tm_low[8] = -40000;
	tm_low[9] = -40000;
	tm_low[10] = -40000;
	tm_low[11] = 0;
	tm_low[12] = 0;
	tm_low[13] = 0;
	tm_low[14] = -40000;
	tm_low[15] = -40000;
	tm_low[16] = -40000;
	tm_low[17] = -40000;
	tm_low[18] = -40000;
	tm_low[19] = -40000;
	tm_low[20] = -40000;
	tm_low[21] = -40000;
	tm_low[22] = -40000;
	tm_low[23] = -40000;
	tm_low[24] = -40000;
	tm_low[25] = -40000;
	tm_low[26] = -40000;
	tm_low[27] = -40000;
	tm_low[28] = -40000;
	tm_low[29] = -40000;
	tm_low[30] = -40000;
	tm_low[31] = -40000;

	tm_high[0] = 0;
	tm_high[1] = 0;
	tm_high[2] = 0;
	tm_high[3] = 0;
	tm_high[4] = 0;
	tm_high[5] = 0;
	tm_high[6] = 0;
	tm_high[7] = 0;
	tm_high[8] = 0;
	tm_high[9] = 0;
	tm_high[10] = 750000;
	tm_high[11] = 0x7FFFFFFF;
	tm_high[12] = 0x7FFFFFFF;
	tm_high[13] = 0x7FFFFFFF;
	tm_high[14] = 750000;
	tm_high[15] = 0;
	tm_high[16] = 0;
	tm_high[17] = 0;
	tm_high[18] = 0;
	tm_high[19] = 0;
	tm_high[20] = 0;
	tm_high[21] = 0;
	tm_high[22] = 0;
	tm_high[23] = 0;
	tm_high[24] = 0;
	tm_high[25] = 0;
	tm_high[26] = 0;
	tm_high[27] = 0;
	tm_high[28] = 0;
	tm_high[29] = 0;
	tm_high[30] = 0;
	tm_high[31] = 0;
}
