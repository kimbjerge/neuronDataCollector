/*
 * Led.h
 *
 *  Created on: 9. aug. 2017
 *      Author: Kim Bjerge
 */

#ifndef SRC_HAL_LEDS_H_
#define SRC_HAL_LEDS_H_

class Leds : Gpio
{
public:

	enum LedTypes {
		LED0 = 0,
		LED1 = 1,
		LED2 = 2,
		LED3 = 3,
		LED4 = 4,
		LED5 = 5,
		LED6 = 6,
		LED7 = 7
	};

	Leds() : Gpio(XPAR_LEDS_DEVICE_ID) {};

	void setOn(LedTypes led, bool on)
	{
		int bits = read();

        if (on)
        	bits |= 1 << led;
        else {
        	bits &= ~(1 << led);
        }

		write(bits);
	}

};


#endif /* SRC_HAL_LEDS_H_ */
