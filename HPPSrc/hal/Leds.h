/*
 * Led.h
 *
 *  Created on: 9. aug. 2017
 *      Author: Kim Bjerge
 */

#ifndef SRC_HAL_LEDS_H_
#define SRC_HAL_LEDS_H_

class Leds
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

	Leds(){};

	void setOn(LedTypes led, bool on);

};

#endif /* SRC_HAL_LEDS_H_ */
