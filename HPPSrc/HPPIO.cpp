/*
 * HPPIO.cpp
 *
 *  Created on: 2. mar. 2019
 *      Author: Kim Bjerge
 */
#include "hpp.h"

// To be called just after calling vSetupHPP()
// but before vTaskScheduler()
// to get control from PS over the DLSX Motherboard
void initIO(void)
{
	u8 portnum = 0;
	u8 status = 0;

	// Assert PS control (as opposed to fabric control) over the commands sent to the DLSX motherboard
	status = SetPSControl(PS_Ctrl);
	if(status != 0)
	{
		xil_printf("\n\rProblem with SetPSControl(PS_Ctrl)");
	}
	else
	{
		xil_printf("\n\rSetPSControl(PS_Ctrl) successfully called.\n\r");
	}

	// Assert output control over ports 0-3
	for (portnum = 0; portnum < 4; portnum++)
	{
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
	}

	//Set PS control over front panel LEDs
	status = SetFPLEDCtrl(HPP_Ctrl);
	if(status != 0)
	{
		xil_printf("\n\rProblem with SetFPLEDCtrl(LED_S1, LED_S2), Status = %d", status);
	}

	//Set PS control over analog outputs
	status = SetAnalogOutputCtrl(HPP_Ctrl);
	if(status != 0)
	{
		xil_printf("\n\rProblem with SetAnalogeOutputCtrl, Status = %d", status);
	}

	// Set PS ready - why this call? - done in Nlx_Demos.c
	SetPSReady(PS_Ctrl);

	xil_printf("Control of the DLSX motherboard initialized (LEDS, TTL and Analog outputs).\n\r");
}

// To be called from a task to toggle LEDS S1 and S2
void setLEDSOn(bool on)
{
	u8 status = 0;

	if (on) {

		//Toggle each LED once for verification of functionality before template matching
		status = SetFPLED(LED_S1, LED_On);
		if(status != 0)
		{
			xil_printf("\n\rProblem with SetFPLED(LED_S1, LED_On), Status = %d", status);
		}
		status = SetFPLED(LED_S2, LED_On);
		if(status != 0)
		{
			xil_printf("\n\rProblem with SetFPLED(LED_S2, LED_On), Status = %d", status);
		}

	} else {

		status = SetFPLED(LED_S1, LED_Off);
		if(status != 0)
		{
			xil_printf("\n\rProblem with SetFPLED(LED_S1, LED_Off), Status = %d", status);
		}



		status = SetFPLED(LED_S2, LED_Off);
		if(status != 0)
		{
			xil_printf("\n\rProblem with SetFPLED(LED_S2, LED_On), Status = %d", status);
		}
	}

}

// To be called from a task to set port 1 bit 0 high or low on the DLSX Motherboard
void setPort2Bit0(bool on)
{
	u8 status;
    if (on) {
    	status = SetDIOPortBit(2, 0, Bit_High); // Locks in function
    	if(status != 0)
    	{
    		xil_printf("\n\rProblem with SetFPLED(LED_S2, LED_Off), Status = %d", status);
    	}
    } else {
		status = SetDIOPortBit(2, 0, Bit_Low);
		if(status != 0)
		{
			xil_printf("\n\rProblem with SetFPLED(LED_S2, LED_Off), Status = %d", status);
		}
    }
}


// To be called from a task to set all ports to high or low on the DLSX Motherboard
void setAllPortsOn(bool on)
{
	u8 status;
    if (on) {
    	//status = SetDIOAllValues(0x00000004); // Locks?? port 0 and 1 locks
    	//status = SetDIOAllValues(0x80000004); // Don't locks???
    	//status = SetDIOAllValues(0x7fffffff); // Works???, port 2 bit 3, (nr. 4 fra neden)
    	//status = SetDIOAllValues(0xffff0000); // Works???, port 2 and 3 only!!
    	status = SetDIOAllValuesNoAck(0xffff0000); // Fast version
    	if(status != 0)
    	{
    		xil_printf("\n\rProblem with SetDIOAllValues, Status = %d", status);
    	}
    } else {
		//status = SetDIOAllValues(0x00000000);
		status = SetDIOAllValuesNoAck(0x00000000);
		if(status != 0)
		{
			xil_printf("\n\rProblem with SetDIOAllValues, Status = %d", status);
		}
    }
}

// To be called from a task to toggle any port and bitnum to high or low on the DLSX Motherboard
void toggleTTLOuput(short ttl_output_bitnum)
{
	u8 portnum, bitnum;

	IdentifyDIOParams(ttl_output_bitnum, &portnum, &bitnum);
	xil_printf("Toggle output port %d, bitnum %d", portnum, bitnum);
	ToggleDigIO(ttl_output_bitnum); // Locks in function SetDIOAllValues ???
}

