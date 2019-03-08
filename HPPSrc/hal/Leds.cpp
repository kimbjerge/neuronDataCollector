/*
 * Gpio.cpp
 *
 *  Created on: 9. aug. 2017
 *      Author: Kim Bjerge
 */

#include "Leds.h"
#include "hpp.h"

void Leds::setOn(LedTypes led, bool on)
{
	LEDNum ledNum;
	u8 status = 0;

	// Only control LED0 = LED_S1 (template 0)
	// LED7 = LED_S2 (template within window
	if (led == LED0)
		ledNum = LED_S1;
	else if (led == LED7)
		ledNum = LED_S2;
	else
		return;

	if (on) {
		//Toggle each LED once for verification of functionality before template matching
		status = SetFPLED(ledNum, LED_On);
		if(status != 0)
		{
			xil_printf("\n\rProblem with SetFPLED(LED_S1, LED_On), Status = %d", status);
		}
	} else {
		status = SetFPLED(ledNum, LED_Off);
		if(status != 0)
		{
			xil_printf("\n\rProblem with SetFPLED(LED_S1, LED_Off), Status = %d", status);
		}
	}

}
