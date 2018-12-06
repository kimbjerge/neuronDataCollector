/*
 * hpp.h
 *
 *  Created on: Feb 18, 2014
 *      Author: stevem
 */

#ifndef HPP_H_
#define HPP_H_

#include <stdio.h>
#include "platform.h"
#include "xscugic.h"
//#include "xtmrctr.h"
#include "xil_types.h"
#include "xparameters.h"
#include "xgpiops.h"
#include "hpp_iic_master.h"
#include "xtime_l.h"
#include "xqspips.h"

/***Definitions for Core_nIRQ/nFIQ interrupts ****/
/* Definitions for Fabric interrupts connected to ps7_scugic_0 */
#define XPAR_FABRIC_SYSTEM_FILE_INT_INTR 91
#define XPAR_FABRIC_SYSTEM_HPP_INT_INTR 90
#define XPAR_FABRIC_SYSTEM_GPIO_INT_INTR 89

#define GPIO_BANK_2 			2
#define GPIO_BANK_3 			3
#define DIR_INPUT 				0x00000000
#define DIR_OUTPUT 				0xffffffff
#define ALL_LOW 				0x00000000
#define ALL_HIGH 				0xFFFFFFFF


#ifdef ZEDBOARD_DEBUG

// Define virtual HPP address space in RAM 1 memory
#define HPP_DATA_ADDR 			0xFFFF0000
#define HPP_DATA_ADDR_HIGH		0xFFFFFDF0

#define CHEETAH_CMD_ADDR 			0xFFFF0000
#define XPAR_ANALOG_M_AXI_BASEADDR 	0xFFFF0000

#define FILE_EXE_ADDR			0xFFFF0400
#define FILE_BASE_ADDR			0xFFFF0000
#define FILE_TRANSFER_ADDR		0xFFFF0400
#define FILE_STORAGE_BASE_ADDR	0xFFFF0000
#define FILE_HIGH_ADDR			0xFFFF0000
#define BIT_EXE_ADDR			0xFFFF0000

#define CHEETAH_ACK_ADDR 		0xFFFFF000
#define CHEETAH_ACK_ADDR_HIGH	0xFFFFFBFF

#define PL_INTERRUPTS_ADDR		0xFFFFE000

#else

#define HPP_DATA_ADDR 			0x30000000
#define HPP_DATA_ADDR_HIGH		0x3FFFFFFF

//May need to update ASCII command interface depending on size of transfers... (STM 23June14)
//Address 0x43C00000
#define CHEETAH_CMD_ADDR 		XPAR_ASCII_M_AXI_BASEADDR

#define FILE_EXE_ADDR			0x20000400
#define FILE_BASE_ADDR			0x20000000
#define FILE_TRANSFER_ADDR		0x20000400
#define FILE_STORAGE_BASE_ADDR	0x20800000
#define FILE_HIGH_ADDR			0x2F800000
#define BIT_EXE_ADDR			0x2F000000

#define CHEETAH_ACK_ADDR 		0x2FFFF000
#define CHEETAH_ACK_ADDR_HIGH	0x2FFFFBFF

#define PL_INTERRUPTS_ADDR		0x2FFFE000

#endif

#define HPPSTX 					0x00000800
#define HPPPID 					0x00000001

#define FPGA_RST_CTRL			0xF8000240


#define PURE_ASCII_MSG 			0
#define FILTER_MSG 				1

//Acknowledgment Success or Failure values for the Status byte
#define HPP_SUCCESS				0
#define HPP_GENERAL_FAILURE 	-1
#define HPP_COMMAND_NOT_FOUND 	-2
#define HPP_INVALID_PARAMETER 	-3

//PL <-> PS GPIO pin definitions
#define DATA_INT_ACK_BIT		0
#define CMD_MSG_ACK_BIT			1
#define FILE_INT_ACK_BIT		2
#define FILE_CMD_ID_LSB			3
#define FILE_CMD_ID_MSB			5
#define PL_DEMO_CMD_START		6 //The demo needs the logic commands sent first for ensuring outputs are setup properly
#define PL_DEMO_START			7 //Starts the demo after ample time has passed for above.
#define FILE_TRANSFER_READY		15
#define ANALOG_CTL_BIT			16
#define STIM_CTL_BIT			17
#define LED_CTL_BIT				18
#define S1_DATA_BIT				19
#define S2_DATA_BIT				20
#define TTL_IO_CTL_BASE			21
#define TTL_IO_CTL_HIGH			24
#define TTL_PORT_DIR_BASE		25
#define TTL_PORT_DIR_HIGH		28
#define CONTROL_BIT				29
#define ASCII_TRANSFER			30
#define READY_BIT				31
#define TTL_VALUE_BASE			32
#define TTL_VALUE_HIGH			63


//Logic Command IDs
#define CMD_DIG_IO_CTRL			1
#define CMD_FP_LED_CTRL			2
#define CMD_AN_OUT_CTRL			3
#define CMD_HS_STIM_CTRL		4
#define CMD_PORT_DIR_CTRL		5
#define CMD_DIG_IO_BIT			6
#define CMD_DIG_IO_PORT			7
#define CMD_ALL_DIG_IO			8
#define CMD_SET_FP_LED			9


#define AVAILABLE_BUFFERS		0x10000//(HPP_DATA_ADDR_HIGH + 1 - HPP_DATA_ADDR) / (0x1000)
#define AVAILABLE_BUFFERS_MASK	0x0000FFFF


struct Pulse_Train_Params
{
	u8		ttl_input_bitnum;
	u8		ttl_output_bitnum;
	u8		portnum;
	u8		bitnum;
	u32		high_time_ms;
	u32		low_time_ms;
};


struct HPP_Data_Type
{
	u32		TimeStamp_High;
	u32		TimeStamp_Low;
	u32		TTL_Port_Values;
	u32		Analog_Output_Data[4];
	s32		AD[512];
	u32		Reserved[505]; //Fill in the rest of the address space until the next struct location
};

struct HPP_Logic_Ack_Type
{
	/* Changed STM 27May14 due to processor byte-swapping
	u8		Command;
	s8		Status;
	*/
	u8		Command;
	s8		Status;
};

struct HPP_Ascii_Ack_Type
{
	u32		Header;
	char	Command[508];  //Also includes status...forcing 8-bit and 32-bit CRCs within this string for simplicity
};

struct HPP_Cheetah_Command
{
	u32		SOP;
	u32		PacketID;
	u32		Size;
	u8 		Stx; //Start Internal Cheetah Packet
	u8 		RecordID;
	u16 	DataSize;
	char 	Data[64];
	u8 		CRC; //End Internal Cheetah Packet
	u32		OverallCRC;
};


struct HPP_Cheetah_Response
{
	u8 		Stx; //Start Internal Cheetah Packet
	u8 		RecordID;
	u16 	DataSize;
	u8		ResponseParameter;
	char 	Data[64];
	u8 		CRC; //End Internal Cheetah Packet
};


struct HPP_File_Command
{
	u8		CommandID;
	char 	Filename[264]; //The data is not 32 bit word aligned, so the code will take care of the interpretation of the type and size...
};


struct HPP_File_DataSet
{
	u32		Data[128]; //8 MB maximum of file space
};


struct HPP_File_End
{
	u8		CommandID;
	u32		CRC;
};


struct HPP_File_Exe
{
	u8		CommandID;
	char	Filename[256];
	char	Extension[4];
};



struct HPP_Audio_Filter
{
	u32		SOP;
	u32		PacketID;
	u32		Size;
	u8 		Stx; //Start Internal Cheetah Packet
	u8 		RecordID;
	u16 	DataSize;
	char 	Data[128];
	float	lowcutcoeffs[128];
	char	Data2[64];
	float	highcutcoeffs[128];
	char	Data3[4];
	u8 		CRC; //End Internal Cheetah Packet
	u32		OverallCRC;
};


struct HPP_Analog_Data
{
	u16 	Analog_Out_0_Data;
	u16 	Analog_Out_1_Data;
	u16 	Analog_Out_2_Data;
	u16 	Analog_Out_3_Data;
	//Ordered this way for simplicity of transfers...may need to change if problems found through integration
	u8 		Update_Mask; //Bit 0: Update Analog Out 0 when high
						 //Bit 1: Update Analog Out 1 when high
	 	 	 	 	 	 //Bit 2: Update Analog Out 2 when high
						 //Bit 3: Update Analog Out 3 when high
};


void ConvertMsg(u8 msgType);


enum DigitalIOPortDirection
{
	DIO_PORT_DIR_INPUT,
	DIO_PORT_DIR_OUTPUT
};


enum ControlSource
{
	Motherboard_Ctrl,
	HPP_Ctrl
};

enum TTLPulseType
{
	DIO_PULSE_LOW,
	DIO_PULSE_HIGH
};

enum AudioSource
{
	ADSignal,
	ArbitraryWaveform,
	HPP
};

enum ContinuousType
{
	NotContinuous,
	Continuous
};

enum FilterEnabled
{
	DSPDisabled,
	DSPEnabled
};

enum InputInversion
{
	NotInverted,
	Inverted
};

enum BitValue
{
	Bit_Low,
	Bit_High
};


enum HPPMessageControl
{
	Logic_Ctrl,
	PS_Ctrl
};


enum LEDNum
{
	LED_S1,
	LED_S2
};


enum LED_Status
{
	LED_Off,
	LED_On
};


extern const struct HPP_Data_Type* HPP_Data;
extern struct HPP_Ascii_Ack_Type* HPP_Ascii_Ack;
extern const struct HPP_Logic_Ack_Type* HPP_Logic_Ack;


extern const struct HPP_File_Exe* HPP_File_Execute;

extern const unsigned int* num_data_interrupts;

extern struct HPP_Cheetah_Command HPP_Cmd;
extern struct HPP_Audio_Filter HPP_AF;
extern char temp_msg_str[1256];
extern u32 temp_msg_str_len;
extern char cheetah_msg_str[1256];
extern u32 cheetah_msg_str_len;

extern struct File_Params* File_Parameters;
extern u32 current_file_address;

extern u32 num_data_buffers_loaded;
extern u32 data_channels_loaded;
extern u32 most_matches;
extern u32 active_buffer_index;
extern enum LED_Status led_s1_value;
extern enum LED_Status led_s2_value;

extern u32 gpio_bank2;
extern u32 gpio_bank3;
extern u32 logic_status;
extern XGpioPs xGpiops;

extern XQspiPs QspiInstance;

extern u32 num_file_interrupts;


extern u32 hpp_debug;
void vSetupHPP(void);
void vVerifyCommandControl(void* pvNotUsed);

void vGPIOInit(void);
void vInit_HPPData(void);
void vSetup_HPP_GPIO(void);
void vHPP_DataInterruptHandler(void *pvNotUsed);
void vHPP_AckInterruptHandler(void *pvNotUsed);
void vHPP_FileInterruptHandler(void *pvNotUsed);
void vRealTimeVerification(void *pvParameters);
void vinit_templates(void);
void vToggleLED(void* params);
void ToggleLED(u8 led_num);
void vToggleDigIO(void* params);
void ToggleDigIO(u8 bitnum);
void IdentifyDIOParams(u16 bit_number, u8 *portnum, u8 *bitnum);

void vSetup_Template_Matching_Demo(void *pvParameters);
void verify_hpp_data(int channel_buf_num);
void vConfigureHPPDataInterrupt(void *pvNotUsed);
void vConfigureHPPAckInterrupt(void *pvNotUsed);
void vConfigureHPPFileInterrupt(void *pvNotUsed);
void vpVerify_Task(void);

void ClearPSGPIO(void);

void New_HPPCmd(void);
void New_AudioFilterCmd(void);
void Calc8CRC(u8);
void Calc32CRC(u8);
s8 ProcessAck(void *result);
s8 ProcessAsciiAck(void *result);

s8 SetPSControl(enum HPPMessageControl msg_controller);
s8 SetPSReady(enum HPPMessageControl msg_controller);
s8 SetFTReady(enum HPPMessageControl msg_controller);
s8 SetDIOCtrl(u8, enum ControlSource source);
s8 SetFPLEDCtrl(enum ControlSource source);
s8 SetAnalogOutputCtrl(enum ControlSource source);
s8 SetHeadstageStimCtrl(enum ControlSource source);
s8 SetDIOPortDir(u8 portnum, enum DigitalIOPortDirection dir);
s8 SetDIOPortBit(u8 portnum, u8 bitnum, enum BitValue);
s8 SetDIOPortValue(u8 portnum, u8 value);
s8 SetDIOAllValues(u32 values);

s8 SetFPLED(enum LEDNum led_num, enum LED_Status value);


//STM 24Sept14
//for debug purposes only
void FastToggleDigIO(u8 bitnum);
s8 SetDIOAllValuesNoAck(u32 values);



s8 UpdateAnalog(u8 update_mask, u16 analog0, u16 analog1, u16 analog2, u16 analog3);
s8 HeadstageStimCommand(char *command);


s8 StartAcquisition(void);
s8 StopAcquisition(void);
s8 GetSampleFrequency(u32 *sample_freq);
s8 GetBoardInformation(u32 boardnum, char *board_type);
s8 DRSCommand(char *cmdstring);
s8 SetDigitalIOPortDirection(u8 portnum, enum DigitalIOPortDirection direction);
s8 ExecuteTTLPulse(u8 portnum, u8 bitnum, enum TTLPulseType pulsetype);
s8 SetTTLPulseDuration(u8 portnum, u16 duration);
s8 SetDigitalIOBit(u8 portnum, u8 bitnum, enum BitValue value);
s8 SetDigitalIOPortValue(u8 portnum, u8 value);
s8 SetAudioSource(u8 channel, enum AudioSource source);
s8 SetADSignalOutput(u8 daoutchannel, u8 dasubchannel, u16 adchannel);
s8 SetWaveformSignalOutput(u8 daoutchannel, u16 buffer, char *mode);
s8 PlayOutputWaveform(u8 daoutchannel, enum ContinuousType continuous);
s8 SetDAOutputFrequency(u8 daoutchannel, u16 rate);
s8 InitiateWaveformDownload(char *ipaddress, u8 portnum);
s8 ClearAudioOutput(u8 daoutchannel, u8 dasubchannel);
s8 SetAudioFilter(u16 adchannel, u8 daoutchannel, u8 dasubchannel, double lowcutfreq,
					u16 lowcuttapcount, float *lowcutcoeffs, enum FilterEnabled lowcutenabled,
					double highcutfreq, u16 highcuttapcount, float *highcutcoeffs, enum
					FilterEnabled highcutenabled);
s8 SetAudioVolume(u8 daoutchannel, u8 volume);
s8 SetAudioInputInverted(u8 daoutchannel, u8 dasubchannel, enum InputInversion invert);
s8 SetAudioInputRange(u16 adchannel, u8 daoutchannel, s32 lowrange, s32 highrange);
s8 HeadstageStimCommand(char *command);
s8 InitAppDownload(char *filename);
s8 ExeApplication(char *filename);
s8 InitHPPBitDownload(char *filename);
s8 SetEth2IP(char *ipaddress);


//Abstracted User Functions
s8 GetNumSamples(u32 *numsamples);  //Puts number of samples loaded into *numsamples
s8 GetSample(u32 samplenum, struct HPP_Data_Type* Data_Sample); //Copies Data sample of number samplenum into *Data_Sample parameter
s8 GetSampleRange(u32 startsamplenum, u32 endsamplenum, struct HPP_Data_Type* Data_Sample); //Copies data samples from startsamplenum to endsamplenum into *Data_Sample parameter

void vPulseTrain(void *pvparams);


//Demonstration functions
void vExperimentControl(void *parameters);

#endif /* HPP_H_ */
