/*
 * Switch.h
 *
 *  Created on: 9. aug. 2017
 *      Author: Kim Bjerge
 */

#ifndef SRC_HAL_SWITCH_H_
#define SRC_HAL_SWITCH_H_

#include "Gpio.h"

class Switch : Gpio
{
public:

	enum SwitchTypes {
		SW0 = 1,
		SW1 = 2,
		SW2 = 4,
		SW3 = 16,
		SW4 = 32,
		SW5 = 64,
		SW6 = 128,
		SW7 = 256
	};

	Switch() : Gpio(XPAR_SWITCH_DEVICE_ID)
	{
		mSwitchValues = read();
	};

	bool isOn(SwitchTypes sw)
	{
		mSwitchValues = read();
		if (mSwitchValues & sw)
			return true;
		else
			return false;
	}

private:
	int mSwitchValues;

};


#endif /* SRC_HAL_SWITCH_H_ */
