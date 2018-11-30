/*
 * Buttons.h
 *
 *  Created on: 9. aug. 2017
 *      Author: Kim Bjerge
 */

#ifndef SRC_HAL_BUTTONS_H_
#define SRC_HAL_BUTTONS_H_


#include "Gpio.h"

class Buttons : public Gpio
{
public:

	enum ButtonTypes {
		BTN0 = 1,
		BTN1 = 2,
		BTN2 = 4,
		BTN3 = 8,
		BTN4 = 16
	};

	Buttons() : Gpio(XPAR_BTNS_DEVICE_ID) {};

	bool isOn(ButtonTypes btn)
	{
		int val = readio();
		if (val & btn)
			return true;
		else
			return false;
	}

};


#endif /* SRC_HAL_BUTTONS_H_ */
