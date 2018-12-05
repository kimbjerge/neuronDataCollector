/*
 * hpp_testing.c
 *
 *  Created on: 10July14, 2014
 *      Author: stevem
 */

#include "hpp.h"
#include "Nlx_Demos.h"
#include <stdio.h>
#include "platform.h"
#include "freertos.h"
#include "task.h"
#include "portmacro.h"
#include "sleep.h"
#include "hpp_testing.h"
#include "hpp_iic_master.h"

struct Pulse_Train_Params pt_params;


float lowcutcoeffs[16] = {0, -0.000886, -0.002053, -0.003557, -0.005468, -0.007886, -0.010948, -0.014857,
						  -0.01992, -0.026633, -0.035854, -0.049199, -0.070129, -0.107645, -0.194749, -0.628459};
float highcutcoeffs[16] = {0, -0.000863, -0.003001, 0, 0.007046, 0.005962, -0.007908, -0.016684,
						   0, 0.02805, 0.022765, -0.030606, -0.069458, 0, 0.189081, 0.375616};

void vTestCheetahEmulatedCommands(void* pvNotUsed)
{
	u8 status = 0;
	u16 i = 0;
	u16 j = 0;
	u16 k = 0;
	u32 score = 0;
	u32 sample_freq = 0;
	enum HPPMessageControl msg_controller = PS_Ctrl;
	char* board_info = NULL;
	char *cmdstring = (char*)"b0 hsp 0    \r";

	s32 low_range = 0;
	s32 high_range = 0;

	(void) pvNotUsed;



	//We need to ensure that the PS has control over the ascii/cheetah emulated commands
	status = (u8)SetPSControl(msg_controller);
	switch(status)
	{
		case 0:
			xil_printf("\n\rSetPSControl(%d) called successfully.", msg_controller);
			break;
		case 255:
			xil_printf("\n\rProblem with SetPSControl(%d), Status = General Failure", msg_controller);
			score++;
			break;
		case 254:
			xil_printf("\n\rProblem with SetPSControl(%d), Status = Invalid Command", msg_controller);
			score++;
			break;
		case 253:
			xil_printf("\n\rProblem with SetPSControl(%d), Status = Invalid Parameter", msg_controller);
			score++;
			break;
		default:
			xil_printf("\n\rProblem with SetPSControl(%d), Status = %d", msg_controller, status);
			score++;
			break;
	}


	status = StartAcquisition();
	switch(status)
	{
		case 0:
			xil_printf("\n\rStartAcquisition() called successfully.");
			break;
		case 255:
			xil_printf("\n\rProblem with StartAcquisition(), Status = General Failure");
			score++;
			break;
		case 254:
			xil_printf("\n\rProblem with StartAcquisition(), Status = Invalid Command");
			score++;
			break;
		case 253:
			xil_printf("\n\rProblem with StartAcquisition(), Status = Invalid Parameter");
			score++;
			break;
		default:
			xil_printf("\n\rProblem with StartAcquisition(), Status = %d", status);
			score++;
			break;
	}


	usleep(1000);


	status = StopAcquisition();
	switch(status)
	{
		case 0:
			xil_printf("\n\rStopAcquisition() called successfully.");
			break;
		case 255:
			xil_printf("\n\rProblem with StopAcquisition(), Status = General Failure");
			score++;
			break;
		case 254:
			xil_printf("\n\rProblem with StopAcquisition(), Status = Invalid Command");
			score++;
			break;
		case 253:
			xil_printf("\n\rProblem with StopAcquisition(), Status = Invalid Parameter");
			score++;
			break;
		default:
			xil_printf("\n\rProblem with StopAcquisition(), Status = %d", status);
			score++;
			break;
	}


	usleep(1000);


	status = GetSampleFrequency(&sample_freq);
	switch(status)
	{
		case 0:
			xil_printf("\n\rGetSampleFrequency(&sample_freq) called successfully, Sample Frequency = %d", sample_freq);
			break;
		case 255:
			xil_printf("\n\rProblem with GetSampleFrequency(&sample_freq), Status = General Failure");
			score++;
			break;
		case 254:
			xil_printf("\n\rProblem with GetSampleFrequency(&sample_freq), Status = Invalid Command");
			score++;
			break;
		case 253:
			xil_printf("\n\rProblem with GetSampleFrequency(&sample_freq), Status = Invalid Parameter");
			score++;
			break;
		default:
			xil_printf("\n\rProblem with GetSampleFrequency(&sample_freq), Status = %d", status);
			score++;
			break;
	}


	usleep(1000);


	for(i = 0; i < 16; i++)
	{
		board_info = NULL;
		status = GetBoardInformation(i, board_info);
		switch(status)
		{
			case 0:
				xil_printf("\n\rGetBoardInformation(%d, board_info) called successfully, board_info = %s", i, board_info);
				break;
			case 255:
				xil_printf("\n\rProblem with GetBoardInformation(%d, board_info), Status = General Failure", i);
				score++;
				break;
			case 254:
				xil_printf("\n\rProblem with GetBoardInformation(%d, board_info), Status = Invalid Command", i);
				score++;
				break;
			case 253:
				xil_printf("\n\rProblem with GetBoardInformation(%d, board_info), Status = Invalid Parameter", i);
				score++;
				break;
			default:
				xil_printf("\n\rProblem with GetBoardInformation(%d, board_info), Status = %d", i, status);
				score++;
				break;
		}

		usleep(1000);
	}


	status = DRSCommand(cmdstring);
	switch(status)
	{
		case 0:
			xil_printf("\n\rDRSCommand() called successfully with parameter:\n\r%s\n\r", cmdstring);
			break;
		case 255:
			xil_printf("\n\rProblem with DRSCommand(%s), Status = General Failure", cmdstring);
			score++;
			break;
		case 254:
			xil_printf("\n\rProblem with DRSCommand(%s), Status = Invalid Command", cmdstring);
			score++;
			break;
		case 253:
			xil_printf("\n\rProblem with DRSCommand(%s), Status = Invalid Parameter", cmdstring);
			score++;
			break;
		default:
			xil_printf("\n\rProblem with DRSCommand(%s), Status = %d", cmdstring, status);
			score++;
			break;
	}


	usleep(1000);


	for(i = 0; i < 4; i++)
	{
		for(j = 0; j < 2; j++)
		{
			status = SetDigitalIOPortDirection(i, (DigitalIOPortDirection)j);
			switch(status)
			{
				case 0:
					xil_printf("\n\rSetDigitalIOPortDirection(%d, %d) called successfully.", i, j);
					break;
				case 255:
					xil_printf("\n\rProblem with SetDigitalIOPortDirection(%d, %d), Status = General Failure", i, j);
					score++;
					break;
				case 254:
					xil_printf("\n\rProblem with SetDigitalIOPortDirection(%d, %d), Status = Invalid Command", i, j);
					score++;
					break;
				case 253:
					xil_printf("\n\rProblem with SetDigitalIOPortDirection(%d, %d), Status = Invalid Parameter", i, j);
					score++;
					break;
				default:
					xil_printf("\n\rProblem with SetDigitalIOPortDirection(%d, %d), Status = %d", i, j, status);
					score++;
					break;
			}

			usleep(1000);
		}
	}


	for(i = 0; i < 4; i++)
	{
		status = SetTTLPulseDuration(i, 1000);
		switch(status)
		{
			case 0:
				xil_printf("\n\rSetTTLPulseDuration(%d, 1000) called successfully.", i);
				break;
			case 255:
				xil_printf("\n\rProblem with SetTTLPulseDuration(%d, 1000), Status = General Failure", i);
				score++;
				break;
			case 254:
				xil_printf("\n\rProblem with SetTTLPulseDuration(%d, 1000), Status = Invalid Command", i);
				score++;
				break;
			case 253:
				xil_printf("\n\rProblem with SetTTLPulseDuration(%d, 1000), Status = Invalid Parameter", i);
				score++;
				break;
			default:
				xil_printf("\n\rProblem with SetTTLPulseDuration(%d, 1000), Status = %d", i, status);
				score++;
				break;
		}

		usleep(1000);

		for(j = 0; j < 8; j++)
		{
			for(k = 0; k < 2; k++)
			{
				status = ExecuteTTLPulse(i, j, (TTLPulseType)k);
				switch(status)
				{
					case 0:
						xil_printf("\n\rExecuteTTLPulse(%d, %d, %d) called successfully.", i, j, k);
						break;
					case 255:
						xil_printf("\n\rProblem with ExecuteTTLPulse(%d, %d, %d), Status = General Failure", i, j, k);
						score++;
						break;
					case 254:
						xil_printf("\n\rProblem with ExecuteTTLPulse(%d, %d, %d), Status = Invalid Command", i, j, k);
						score++;
						break;
					case 253:
						xil_printf("\n\rProblem with ExecuteTTLPulse(%d, %d, %d), Status = Invalid Parameter", i, j, k);
						score++;
						break;
					default:
						xil_printf("\n\rProblem with ExecuteTTLPulse(%d, %d, %d), Status = %d", i, j, k, status);
						score++;
						break;
				}

				usleep(1000);
			}
		}
	}


	for(i = 0; i < 4; i++)
	{
		for(j = 0; j < 8; j++)
		{
			for(k = 0; k < 2; k++)
			{
				status = SetDigitalIOBit(i, j, (BitValue)k);
				switch(status)
				{
					case 0:
						xil_printf("\n\rSetDigitalIOBit(%d, %d, %d) called successfully.", i, j, k);
						break;
					case 255:
						xil_printf("\n\rProblem with SetDigitalIOBit(%d, %d, %d), Status = General Failure", i, j, k);
						score++;
						break;
					case 254:
						xil_printf("\n\rProblem with SetDigitalIOBit(%d, %d, %d), Status = Invalid Command", i, j, k);
						score++;
						break;
					case 253:
						xil_printf("\n\rProblem with SetDigitalIOBit(%d, %d, %d), Status = Invalid Parameter", i, j, k);
						score++;
						break;
					default:
						xil_printf("\n\rProblem with SetDigitalIOBit(%d, %d, %d), Status = %d", i, j, k, status);
						score++;
						break;
				}

				usleep(1000);
			}
		}
	}


	for(i = 0; i < 4; i++)
	{
		for(j = 0; j < 256; j++)
		{
			status = SetDigitalIOPortValue(i, j);
			switch(status)
			{
				case 0:
					xil_printf("\n\rSetDigitalIOPortValue(%d, %d) called successfully.", i, j);
					break;
				case 255:
					xil_printf("\n\rProblem with SetDigitalIOPortValue(%d, %d), Status = General Failure", i, j);
					score++;
					break;
				case 254:
					xil_printf("\n\rProblem with SetDigitalIOPortValue(%d, %d), Status = Invalid Command", i, j);
					score++;
					break;
				case 253:
					xil_printf("\n\rProblem with SetDigitalIOPortValue(%d, %d), Status = Invalid Parameter", i, j);
					score++;
					break;
				default:
					xil_printf("\n\rProblem with SetDigitalIOPortValue(%d, %d), Status = %d", i, j, status);
					score++;
					break;
			}

			usleep(1000);
		}
	}


	for(i = 0; i < 4; i++)
	{
		for(j = 0; j < 2; j++)
		{
			status = SetAudioSource(i, (AudioSource)j);
			switch(status)
			{
				case 0:
					xil_printf("\n\rSetAudioSource(%d, %d) called successfully.", i, j);
					break;
				case 255:
					xil_printf("\n\rProblem with SetAudioSource(%d, %d), Status = General Failure", i, j);
					score++;
					break;
				case 254:
					xil_printf("\n\rProblem with SetAudioSource(%d, %d), Status = Invalid Command", i, j);
					score++;
					break;
				case 253:
					xil_printf("\n\rProblem with SetAudioSource(%d, %d), Status = Invalid Parameter", i, j);
					score++;
					break;
				default:
					xil_printf("\n\rProblem with SetAudioSource(%d, %d), Status = %d", i, j, status);
					score++;
					break;
			}

			usleep(1000);
		}
	}

	//Not yet supported by SX
	//But test this one to verify error;
	status = SetADSignalOutput(0, 0, 0);
					switch(status)
					{
						case 0:
							xil_printf("\n\rSetADSignalOutput(0, 0, 0) called successfully.", i, j, k);
							break;
						case 255:
							xil_printf("\n\rProblem with SetADSignalOutput(0, 0, 0), Status = General Failure", i, j, k);
							score++;
							break;
						case 254:
							xil_printf("\n\rExpected problem with SetADSignalOutput(0, 0, 0), Status = Invalid Command", i, j, k);
							score++;
							break;
						case 253:
							xil_printf("\n\rProblem with SetADSignalOutput(0, 0, 0), Status = Invalid Parameter", i, j, k);
							score++;
							break;
						default:
							xil_printf("\n\rProblem with SetADSignalOutput(0, 0, 0), Status = %d", i, j, k, status);
							score++;
							break;
					}

	usleep(1000);


	//Not yet supported by SX
	/*
	for(i = 0; i < 4; i++)
	{
		for(j = 0; j < 4; j++)
		{
			for(k = 0; k < 32; k++)
			{
				status = SetADSignalOutput(i, j, k);
				switch(status)
				{
					case 0:
						xil_printf("\n\rSetADSignalOutput(%d, %d, %d) called successfully.", i, j, k);
						break;
					case 255:
						xil_printf("\n\rProblem with SetADSignalOutput(%d, %d, %d), Status = General Failure", i, j, k);
						score++;
						break;
					case 254:
						xil_printf("\n\rProblem with SetADSignalOutput(%d, %d, %d), Status = Invalid Command", i, j, k);
						score++;
						break;
					case 253:
						xil_printf("\n\rProblem with SetADSignalOutput(%d, %d, %d), Status = Invalid Parameter", i, j, k);
						score++;
						break;
					default:
						xil_printf("\n\rProblem with SetADSignalOutput(%d, %d, %d), Status = %d", i, j, k, status);
						score++;
						break;
				}

				usleep(1000);
			}
		}
	}
	 */


	//Not yet supported by SX
	//But tested anyway to verify error condition
	status = SetWaveformSignalOutput(1, 1, (char*)"Buffered");
	switch(status)
	{
		case 0:
			xil_printf("\n\rSetWaveformSignalOutput(1, 1, \"Buffered\") called successfully.");
			break;
		case 255:
			xil_printf("\n\rProblem with SetWaveformSignalOutput(1, 1, \"Buffered\"), Status = General Failure");
			score++;
			break;
		case 254:
			xil_printf("\n\rExpected problem with SetWaveformSignalOutput(1, 1, \"Buffered\"), Status = Invalid Command");
			score++;
			break;
		case 253:
			xil_printf("\n\rProblem with SetWaveformSignalOutput(1, 1, \"Buffered\"), Status = Invalid Parameter");
			score++;
			break;
		default:
			xil_printf("\n\rProblem with SetWaveformSignalOutput(1, 1, \"Buffered\"), Status = %d", status);
			score++;
			break;
	}

	usleep(1000);

	//Not yet supported by SX
	/*
	for(i = 0; i < 4; i++)
	{
		for(j = 0; j < 32; j++)
		{
			status = SetWaveformSignalOutput(i, j, "Buffered");
			switch(status)
			{
				case 0:
					xil_printf("\n\rSetWaveformSignalOutput(%d, %d, \"Buffered\") called successfully.", i, j);
					break;
				case 255:
					xil_printf("\n\rProblem with SetWaveformSignalOutput(%d, %d, \"Buffered\"), Status = General Failure", i, j);
					score++;
					break;
				case 254:
					xil_printf("\n\rProblem with SetWaveformSignalOutput(%d, %d, \"Buffered\"), Status = Invalid Command", i, j);
					score++;
					break;
				case 253:
					xil_printf("\n\rProblem with SetWaveformSignalOutput(%d, %d, \"Buffered\"), Status = Invalid Parameter", i, j);
					score++;
					break;
				default:
					xil_printf("\n\rProblem with SetWaveformSignalOutput(%d, %d, \"Buffered\"), Status = %d", i, j, status);
					score++;
					break;
			}

			usleep(1000);
		}
	}
	*/


	//Not yet supported by SX
	//But tested anyway to verify error condition
	status = SetWaveformSignalOutput(2, 2, (char*)"AcqSyncStart");
	switch(status)
	{
		case 0:
			xil_printf("\n\rSetWaveformSignalOutput(2, 2, \"AcqSyncStart\") called successfully.", i, j);
			break;
		case 255:
			xil_printf("\n\rProblem with SetWaveformSignalOutput(2, 2, \"AcqSyncStart\"), Status = General Failure", i, j);
			score++;
			break;
		case 254:
			xil_printf("\n\rExpected problem with SetWaveformSignalOutput(2, 2, \"AcqSyncStart\"), Status = Invalid Command", i, j);
			score++;
			break;
		case 253:
			xil_printf("\n\rProblem with SetWaveformSignalOutput(2, 2, \"AcqSyncStart\"), Status = Invalid Parameter", i, j);
			score++;
			break;
		default:
			xil_printf("\n\rProblem with SetWaveformSignalOutput(2, 2, \"AcqSyncStart\"), Status = %d", i, j, status);
			score++;
			break;
	}

	usleep(1000);

	//Not yet supported by SX
	/*
	for(i = 0; i < 4; i++)
	{
		for(j = 0; j < 32; j++)
		{
			status = SetWaveformSignalOutput(i, j, "AcqSyncStart");
			switch(status)
			{
				case 0:
					xil_printf("\n\rSetWaveformSignalOutput(%d, %d, \"AcqSyncStart\") called successfully.", i, j);
					break;
				case 255:
					xil_printf("\n\rProblem with SetWaveformSignalOutput(%d, %d, \"AcqSyncStart\"), Status = General Failure", i, j);
					score++;
					break;
				case 254:
					xil_printf("\n\rProblem with SetWaveformSignalOutput(%d, %d, \"AcqSyncStart\"), Status = Invalid Command", i, j);
					score++;
					break;
				case 253:
					xil_printf("\n\rProblem with SetWaveformSignalOutput(%d, %d, \"AcqSyncStart\"), Status = Invalid Parameter", i, j);
					score++;
					break;
				default:
					xil_printf("\n\rProblem with SetWaveformSignalOutput(%d, %d, \"AcqSyncStart\"), Status = %d", i, j, status);
					score++;
					break;
			}

			usleep(1000);
		}
	}
	*/


	//Not yet supported by SX
	//But tested anyway to verify error condition
	status = SetWaveformSignalOutput(3, 3, (char*)"Continuous");
	switch(status)
	{
		case 0:
			xil_printf("\n\rSetWaveformSignalOutput(3, 3, \"Continuous\") called successfully.", i, j);
			break;
		case 255:
			xil_printf("\n\rProblem with SetWaveformSignalOutput(3, 3, \"Continuous\"), Status = General Failure", i, j);
			score++;
			break;
		case 254:
			xil_printf("\n\rExpected problem with SetWaveformSignalOutput(3, 3, \"Continuous\"), Status = Invalid Command", i, j);
			score++;
			break;
		case 253:
			xil_printf("\n\rProblem with SetWaveformSignalOutput(3, 3, \"Continuous\"), Status = Invalid Parameter", i, j);
			score++;
			break;
		default:
			xil_printf("\n\rProblem with SetWaveformSignalOutput(3, 3, \"Continuous\"), Status = %d", i, j, status);
			score++;
			break;
	}

	usleep(1000);


	//Not yet supported by SX
	/*
	for(i = 0; i < 4; i++)
	{
		for(j = 0; j < 32; j++)
		{
			status = SetWaveformSignalOutput(i, j, "Continuous");
			switch(status)
			{
				case 0:
					xil_printf("\n\rSetWaveformSignalOutput(%d, %d, \"Continuous\") called successfully.", i, j);
					break;
				case 255:
					xil_printf("\n\rProblem with SetWaveformSignalOutput(%d, %d, \"Continuous\"), Status = General Failure", i, j);
					score++;
					break;
				case 254:
					xil_printf("\n\rProblem with SetWaveformSignalOutput(%d, %d, \"Continuous\"), Status = Invalid Command", i, j);
					score++;
					break;
				case 253:
					xil_printf("\n\rProblem with SetWaveformSignalOutput(%d, %d, \"Continuous\"), Status = Invalid Parameter", i, j);
					score++;
					break;
				default:
					xil_printf("\n\rProblem with SetWaveformSignalOutput(%d, %d, \"Continuous\"), Status = %d", i, j, status);
					score++;
					break;
			}

			usleep(1000);
		}
	}
	*/



	//Not yet supported by SX
	//But tested anyway to verify error condition
	status = PlayOutputWaveform(0, (ContinuousType)0);
	switch(status)
	{
		case 0:
			xil_printf("\n\rPlayOutputWaveform(0, 0) called successfully.", i, j);
			break;
		case 255:
			xil_printf("\n\rProblem with PlayOutputWaveform(0, 0), Status = General Failure", i, j);
			score++;
			break;
		case 254:
			xil_printf("\n\rExpected problem with PlayOutputWaveform(0, 0), Status = Invalid Command", i, j);
			score++;
			break;
		case 253:
			xil_printf("\n\rProblem with PlayOutputWaveform(0, 0), Status = Invalid Parameter", i, j);
			score++;
			break;
		default:
			xil_printf("\n\rProblem with PlayOutputWaveform(0, 0), Status = %d", i, j, status);
			score++;
			break;
	}

	usleep(1000);

	//Not yet supported by SX
	/*
	for(i = 0; i < 4; i++)
	{
		for(j = 0; j < 2; j++)
		{
			status = PlayOutputWaveform(i, j);
			switch(status)
			{
				case 0:
					xil_printf("\n\rPlayOutputWaveform(%d, %d) called successfully.", i, j);
					break;
				case 255:
					xil_printf("\n\rProblem with PlayOutputWaveform(%d, %d), Status = General Failure", i, j);
					score++;
					break;
				case 254:
					xil_printf("\n\rProblem with PlayOutputWaveform(%d, %d), Status = Invalid Command", i, j);
					score++;
					break;
				case 253:
					xil_printf("\n\rProblem with PlayOutputWaveform(%d, %d), Status = Invalid Parameter", i, j);
					score++;
					break;
				default:
					xil_printf("\n\rProblem with PlayOutputWaveform(%d, %d), Status = %d", i, j, status);
					score++;
					break;
			}

			usleep(1000);
		}
	}
	*/


	//Not yet supported by SX
	//But tested anyway to verify error condition
	status = SetDAOutputFrequency(1, 10000);
	switch(status)
	{
		case 0:
			xil_printf("\n\rSetDAOutputFrequency(1, 10000) called successfully.");
			break;
		case 255:
			xil_printf("\n\rProblem with SetDAOutputFrequency(1, 10000), Status = General Failure");
			score++;
			break;
		case 254:
			xil_printf("\n\rExpected problem with SetDAOutputFrequency(1, 10000), Status = Invalid Command");
			score++;
			break;
		case 253:
			xil_printf("\n\rProblem with SetDAOutputFrequency(1, 10000), Status = Invalid Parameter");
			score++;
			break;
		default:
			xil_printf("\n\rProblem with SetDAOutputFrequency(1, 10000), Status = %d", status);
			score++;
			break;
	}

	usleep(1000);


	//Not yet supported by SX
	/*
	for(i = 0; i < 4; i++)
	{
		j = 1 << i;
		status = SetDAOutputFrequency(i, j);
		switch(status)
		{
			case 0:
				xil_printf("\n\rSetDAOutputFrequency(%d, %d) called successfully.", i, j);
				break;
			case 255:
				xil_printf("\n\rProblem with SetDAOutputFrequency(%d, %d), Status = General Failure", i, j);
				score++;
				break;
			case 254:
				xil_printf("\n\rProblem with SetDAOutputFrequency(%d, %d), Status = Invalid Command", i, j);
				score++;
				break;
			case 253:
				xil_printf("\n\rProblem with SetDAOutputFrequency(%d, %d), Status = Invalid Parameter", i, j);
				score++;
				break;
			default:
				xil_printf("\n\rProblem with SetDAOutputFrequency(%d, %d), Status = %d", i, j, status);
				score++;
				break;
		}

		usleep(1000);
	}
	*/



	//Not yet supported by SX
	//But tested anyway to verify error condition
	status = InitiateWaveformDownload((char*)"192.168.1.15", 69);  //Random IP address, but port 69 is correct for TFTP
	switch(status)
	{
		case 0:
			xil_printf("\n\rInitiateWaveformDownload(\"192.168.1.15\", 69) called successfully.");
			break;
		case 255:
			xil_printf("\n\rProblem with InitiateWaveformDownload(\"192.168.1.15\", 69), Status = General Failure");
			score++;
			break;
		case 254:
			xil_printf("\n\rExpected problem with InitiateWaveformDownload(\"192.168.1.15\", 69), Status = Invalid Command");
			score++;
			break;
		case 253:
			xil_printf("\n\rProblem with InitiateWaveformDownload(\"192.168.1.15\", 69), Status = Invalid Parameter");
			score++;
			break;
		default:
			xil_printf("\n\rProblem with InitiateWaveformDownload(\"192.168.1.15\", 69), Status = %d", status);
			score++;
			break;
	}


	usleep(1000);



	for(i = 0; i < 4; i++)
	{
		for(j = 0; j < 4; j++)
		{
			status = ClearAudioOutput(i, j);
			switch(status)
			{
				case 0:
					xil_printf("\n\rClearAudioOutput(%d, %d) called successfully.", i, j);
					break;
				case 255:
					xil_printf("\n\rProblem with ClearAudioOutput(%d, %d), Status = General Failure", i, j);
					score++;
					break;
				case 254:
					xil_printf("\n\rProblem with ClearAudioOutput(%d, %d), Status = Invalid Command", i, j);
					score++;
					break;
				case 253:
					xil_printf("\n\rProblem with ClearAudioOutput(%d, %d), Status = Invalid Parameter", i, j);
					score++;
					break;
				default:
					xil_printf("\n\rProblem with ClearAudioOutput(%d, %d), Status = %d", i, j, status);
					score++;
					break;
			}

			usleep(1000);
		}
	}


	for(i = 0; i < 4; i++)
	{
		for(j = 0; j < 101; j++)
		{
			status = SetAudioVolume(i, j);
			switch(status)
			{
				case 0:
					xil_printf("\n\rSetAudioVolume(%d, %d) called successfully.", i, j);
					break;
				case 255:
					xil_printf("\n\rProblem with SetAudioVolume(%d, %d), Status = General Failure", i, j);
					score++;
					break;
				case 254:
					xil_printf("\n\rProblem with SetAudioVolume(%d, %d), Status = Invalid Command", i, j);
					score++;
					break;
				case 253:
					xil_printf("\n\rProblem with SetAudioVolume(%d, %d), Status = Invalid Parameter", i, j);
					score++;
					break;
				default:
					xil_printf("\n\rProblem with SetAudioVolume(%d, %d), Status = %d", i, j, status);
					score++;
					break;
			}

			usleep(1000);
		}
	}


	for(i = 0; i < 4; i++)
	{
		for(j = 0; j < 4; j++)
		{
			for(k = 0; k < 2; k++)
			{
				status = SetAudioInputInverted(i, j, (InputInversion)k);
				switch(status)
				{
					case 0:
						xil_printf("\n\rSetAudioInputInverted(%d, %d, %d) called successfully.", i, j, k);
						break;
					case 255:
						xil_printf("\n\rProblem with SetAudioInputInverted(%d, %d, %d), Status = General Failure", i, j, k);
						score++;
						break;
					case 254:
						xil_printf("\n\rProblem with SetAudioInputInverted(%d, %d, %d), Status = Invalid Command", i, j, k);
						score++;
						break;
					case 253:
						xil_printf("\n\rProblem with SetAudioInputInverted(%d, %d, %d), Status = Invalid Parameter", i, j, k);
						score++;
						break;
					default:
						xil_printf("\n\rProblem with SetAudioInputInverted(%d, %d, %d), Status = %d", i, j, k, status);
						score++;
						break;
				}

				usleep(1000);
			}
		}
	}

	low_range = -1000;
	high_range = 1000;
	for(i = 0; i < 32; i++)
	{
		for(j = 0; j < 4; j++)
		{
			status = SetAudioInputRange(i, j, low_range, high_range);
			switch(status)
			{
				case 0:
					xil_printf("\n\rSetAudioInputRange(%d, %d, %d, %d) called successfully.", i, j, low_range, high_range);
					break;
				case 255:
					xil_printf("\n\rProblem with SetAudioInputRange(%d, %d, %d, %d), Status = General Failure", i, j, low_range, high_range);
					score++;
					break;
				case 254:
					xil_printf("\n\rProblem with SetAudioInputRange(%d, %d, %d, %d), Status = Invalid Command", i, j, low_range, high_range);
					score++;
					break;
				case 253:
					xil_printf("\n\rProblem with SetAudioInputRange(%d, %d, %d, %d), Status = Invalid Parameter", i, j, low_range, high_range);
					score++;
					break;
				default:
					xil_printf("\n\rProblem with SetAudioInputRange(%d, %d, %d, %d), Status = %d", i, j, low_range, high_range, status);
					score++;
					break;
			}

			usleep(1000);
		}
	}


	for(i = 0; i < 32; i++)
	{
		for(j = 0; j < 4; j++)
		{
			for(k = 0; k < 4; k++)
			{
				status = SetAudioFilter(i, j, k, 300, 32, lowcutcoeffs, DSPDisabled, 6000, 32, highcutcoeffs, DSPDisabled);
				switch(status)
				{
					case 0:
						xil_printf("\n\rSetAudioFilter(%d, %d, %d, 300.00, 32, lowcutcoeffs, 0, 6000.00, 32, highcutcoeffs, 0) called successfully.", i, j, k);
						break;
					case 255:
						xil_printf("\n\rProblem with SetAudioFilter(%d, %d, %d, 300.00, 32, lowcutcoeffs, 0, 6000.00, 32, highcutcoeffs, 0), Status = General Failure", i, j, k);
						score++;
						break;
					case 254:
						xil_printf("\n\rProblem with SetAudioFilter(%d, %d, %d, 300.00, 32, lowcutcoeffs, 0, 6000.00, 32, highcutcoeffs, 0), Status = Invalid Command", i, j, k);
						score++;
						break;
					case 253:
						xil_printf("\n\rProblem with SetAudioFilter(%d, %d, %d, 300.00, 32, lowcutcoeffs, 0, 6000.00, 32, highcutcoeffs, 0), Status = Invalid Parameter", i, j, k);
						score++;
						break;
					default:
						xil_printf("\n\rProblem with SetAudioFilter(%d, %d, %d, 300.00, 32, lowcutcoeffs, 0, 6000.00, 32, highcutcoeffs, 0), Status = %d", i, j, k, status);
						score++;
						break;
				}

				usleep(1000);
			}
		}
	}


	status = HeadstageStimCommand((char*)"025C00000000000000000000");
	switch(status)
	{
		case 0:
			xil_printf("\n\rHeadstageStimCommand(\"025C00000000000000000000\") called successfully.");
			break;
		case 255:
			xil_printf("\n\rProblem with HeadstageStimCommand(\"025C00000000000000000000\"), Status = General Failure");
			score++;
			break;
		case 254:
			xil_printf("\n\rProblem with HeadstageStimCommand(\"025C00000000000000000000\"), Status = Invalid Command");
			score++;
			break;
		case 253:
			xil_printf("\n\rProblem with HeadstageStimCommand(\"025C00000000000000000000\"), Status = Invalid Parameter");
			score++;
			break;
		default:
			xil_printf("\n\rProblem with HeadstageStimCommand(\"025C00000000000000000000\"), Status = %d", status);
			score++;
			break;
	}


	//Not yet supported by SX
	//But tested anyway to verify error condition
	status = SetEth2IP((char*)"192.168.1.91");
	switch(status)
	{
		case 0:
			xil_printf("\n\rSetEth2IP(\"192.168.1.91\") called successfully.");
			break;
		case 255:
			xil_printf("\n\rProblem with SetEth2IP(\"192.168.1.91\"), Status = General Failure");
			score++;
			break;
		case 254:
			xil_printf("\n\rExpected problem with SetEth2IP(\"192.168.1.91\"), Status = Invalid Command");
			score++;
			break;
		case 253:
			xil_printf("\n\rProblem with SetEth2IP(\"192.168.1.91\"), Status = Invalid Parameter");
			score++;
			break;
		default:
			xil_printf("\n\rProblem with SetEth2IP(\"192.168.1.91\"), Status = %d", status);
			score++;
			break;
	}


	if(score == 0)
	{
		xil_printf("\n\rvTestCheetahEmulatedCommands completed successfully...!\n\r");
	}
	else
	{
		xil_printf("\n\rvTestCheetahEmulatedCommands completed with failures.  Total score = %d.\n\r", score);
	}

	vTaskDelete(NULL);
}


void vTestLogicCommands(void* pvNotUsed)
{
	s8 status = 0;
	enum HPPMessageControl msg_controller = PS_Ctrl;
	s8 score = 0;
	/*
	enum ControlSource source = HPP_Ctrl;
	enum DigitalIOPortDirection dir = DIO_PORT_DIR_OUTPUT;
	u8 i = 0;
	u16 j = 0;
	u8 k = 0;
	u32 ttl_value = 0x2C2C2C2C;

	u16 an0 = 1;
	u16 an1 = 100;
	u16 an2 = 153;
	u16 an3 = 1001;
	*/

	u32 iterations = 0;


	(void) pvNotUsed;

	//Control and Setup Commands
	status = SetPSControl(msg_controller);
	if(status != 0)
	{
		xil_printf("\n\rProblem with SetPSControl(%d), Status = %d", msg_controller, status);
	}
	else
	{
		xil_printf("\n\rSetPSControl(%d) called successfully, Status = %d\n\r", msg_controller, status);
	}
	score = score + status;


	char recvbuffer[256];
	char sendbuffer[256];
	strcpy(sendbuffer, "025C00000000000000000000");

	for(iterations = 0; iterations < 50; iterations++)
	{
		status = SendHeadstageStimCommand(sendbuffer, recvbuffer);
		if(status != 0)
		{
			xil_printf("\n\rSendHeadstageStimCommand(\"025C00000000000000000000\", recvbuffer), Status = %d",  status);
		}
		else
		{
			xil_printf("\n\rSendHeadstageStimCommand(\"025C00000000000000000000\", recvbuffer) called successfully, Status = %d\n\r", status);
		}
		score = score + status;
	}

/*

	for(iterations = 0; iterations < 1000; iterations++)
	{
		for(i = 0; i < 4; i++)
		{
			status = SetDIOCtrl(i, source);
			if(status != 0)
			{
				xil_printf("\n\rProblem with SetDIOCtrl(%d, %d), Status = %d", i, source, status);
			}
			else
			{
				xil_printf("\n\rSetDIOCtrl(%d, %d) called successfully, Status = %d\n\r", i, source, status);
			}
			score = score + status;
		}

		status = SetFPLEDCtrl(source);
		if(status != 0)
		{
			xil_printf("\n\rProblem with SetFPLEDCtrl(%d), Status = %d", source, status);
		}
		else
		{
			xil_printf("\n\rSetFPLEDCtrl(%d) called successfully, Status = %d\n\r", source, status);
		}
		score = score + status;

		status = SetAnalogOutputCtrl(source);
		if(status != 0)
		{
			xil_printf("\n\rProblem with SetAnalogOutputCtrl(%d), Status = %d", source, status);
		}
		else
		{
			xil_printf("\n\rSetAnalogOutputCtrl(%d) called successfully, Status = %d\n\r", source, status);
		}
		score = score + status;

		status = UpdateAnalog(0xF, an0, an1, an2, an3);
		if(status != 0)
		{
			xil_printf("\n\rProblem with UpdateAnalog(0xF, %d, %d, %d, %d), Status = %d", an0, an1, an2, an3, status);
		}
		else
		{
			xil_printf("\n\rUpdateAnalog(0xF, %d, %d, %d, %d) called successfully, Status = %d\n\r", an0, an1, an2, an3, status);
		}
		score = score + status;
		an0 += 1;
		an1 += 10;
		an2 += 100;
		an3 += 1000;


		status = SetHeadstageStimCtrl(source);
		if(status != 0)
		{
			xil_printf("\n\rProblem with SetHeadstageStimCtrl(%d), Status = %d", source, status);
		}
		else
		{
			xil_printf("\n\rSetHeadstageStimCtrl(%d) called successfully, Status = %d\n\r", source, status);
		}
		score = score + status;

		for(i = 0; i < 4; i++)
		{
			status = SetDIOPortDir(i, dir);
			if(status != 0)
			{
				xil_printf("\n\rProblem with SetDIOAllValues(%d, %d), Status = %d", i, dir, status);
			}
			else
			{
				xil_printf("\n\rSetDIOAllValues(%d, %d) called successfully, Status = %d\n\r", i, dir, status);
			}
			score = score + status;
		}

		for(i = 0; i < 4; i++)
		{
			for(j = 0; j < 8; j++)
			{
				for(k = 0; k < 2; k++)
				{
					status = SetDIOPortBit(i, j, k);
					if(status != 0)
					{
						xil_printf("\n\rProblem with SetDIOPortBit(%d, %d, %d), Status = %d", i, j, k, status);
					}
					else
					{
						xil_printf("\n\rSetDIOPortBit(%d, %d, %d) called successfully, Status = %d\n\r", i, j, k, status);
					}
					score = score + status;
				}
			}
		}

		for(i = 0; i < 4; i++)
		{
			for(j = 0; j < 256; j++)
			{
				status = SetDIOPortValue(i, j);
				if(status != 0)
				{
					xil_printf("\n\rProblem with SetDIOPortValue(%d, %d), Status = %d", i, j, status);
				}
				else
				{
					xil_printf("\n\rSetDIOPortValue(%d, %d) called successfully, Status = %d\n\r", i, j, status);
				}
				score = score + status;
			}
		}

		status = SetDIOAllValues(ttl_value);
		if(status != 0)
		{
			xil_printf("\n\rProblem with SetDIOAllValues(%d), Status = %d", ttl_value, status);
		}
		else
		{
			xil_printf("\n\rSetDIOAllValues(%d) called successfully, Status = %d\n\r", ttl_value, status);
		}
		score = score + status;

		status = SetDIOAllValues(0x75757575);
		if(status != 0)
		{
			xil_printf("\n\rProblem with SetDIOAllValues(%d), Status = %d", 0x75757575, status);
		}
		else
		{
			xil_printf("\n\rSetDIOAllValues(%d) called successfully, Status = %d\n\r", 0x75757575, status);
		}
		score = score + status;

		for(i = 0; i < 2; i++)
		{
			for(j = 0; j < 2; j++)
			{
				status = SetFPLED(i, j);
				if(status != 0)
				{
					xil_printf("\n\rProblem with SetFPLED(%d, %d), Status = %d", i, j, status);
				}
				else
				{
					xil_printf("\n\rSetFPLED(%d, %d) called successfully, Status = %d\n\r", i, j, status);
				}
				score = score + status;
			}
		}
	}

	if(score == 0)
	{
		xil_printf("\n\rvTestLogicCommands completed successfully...!\n\r");
	}
	else
	{
		xil_printf("\n\rvTestLogicCommands completed with failures.  Total score = %d.\n\r", score);
	}
*/

	vTaskDelete(NULL);
}


void vTestDemoApps(void)
{
	//STM 10July14 - Should start acquisition from cheetah emulated command within each demo app and stop it when done with testing...



	BaseType_t xReturn;
	//Template Matching Demo
	//xReturn = xTaskCreate(vSetup_Template_Matching_Demo, "TM_Setup", 10000, NULL, configMAX_PRIORITIES-1, NULL);
	//xil_printf("vSetup_Template_Matching_Demo created with return value of %d\n\r", xReturn);

	/*
	//Real-Time Verification that samples are coming in correctly
	xReturn = xTaskCreate(vRealTimeVerification, "RTVerify", 10000, NULL, 5, NULL);
	xil_printf("RTVerify created with return value of %d\n\r", xReturn);*/


	//Verification of command and control signals
	//xReturn = xTaskCreate(vVerifyCommandControl, "vVerifyCommandControl", 10000, NULL, 5, NULL);
	//xil_printf("vVerifyCommandControl created with return value of %d\n\r", xReturn);

	/*
	//Burst Analysis
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

	xReturn = xTaskCreate(vBurstAnalysis, "vBurstAnalysis", (uint16_t)65535, (void *)&params, 5, NULL);
	xil_printf("vBurstAnalysis created with return value of %d\n\r", xReturn);
	*/

	/*
	//Analog Verification
	xReturn = xTaskCreate(vAnalogVerification, "vAnalogVerification", 50000, NULL, 5, NULL);
	xil_printf("vAnalogVerification created with return value of %d\n\r", xReturn);*/



	//xReturn = xTaskCreate(vVerifyCommandControl, "vVerifyCommandControl", 10000, NULL, 5, NULL);
/*
	u16 i = 0;

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

	xReturn = xTaskCreate(vNeuralEnsembles, "vNeuralEnsembles", (uint16_t)65535, (void *)&params, 5, NULL);
	xil_printf("vNeuralEnsembles created with return value of %d\n\r", xReturn); */



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
	params.ttl_output_bitnum = 4;
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

	xReturn = xTaskCreate(vSpikeDetect, "vSpikeDetect", (uint16_t)50000, (void *)&params, 5, NULL);
	xil_printf("vSpikeDetect created with return value of %d\n\r", xReturn);

	//xReturn = xTaskCreate(vTimeStampCheck, "vTimeStampCheck", (uint16_t)50000, NULL, 3, NULL);
	//xil_printf("vTimeStampCheck created with return value of %d\n\r", xReturn);


//	struct Pulse_Train_Params
//	{
//		u8		ttl_input_bitnum;
//		u8		ttl_output_bitnum;
//		u8		portnum;
//		u8		bitnum;
//		u32		high_time_ms;
//		u32		low_time_ms;
//	};

/*
	pt_params.ttl_input_bitnum = 7;
	pt_params.ttl_output_bitnum = 8;
	pt_params.portnum = 3;
	pt_params.bitnum = 3;
	pt_params.high_time_ms = 1;
	pt_params.low_time_ms = 1;


	xReturn = xTaskCreate(vExperimentControl, "vExperimentControl", (uint16_t)50000, (void *)&pt_params, 5, NULL);
	xil_printf("vExperimentControl created with return value of %d\n\r", xReturn);*/
}


