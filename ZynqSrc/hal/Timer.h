/*
 * Timer.h
 *
 *  Created on: 16. aug. 2017
 *      Author: Kim Bjerge
 */

#ifndef SRC_HAL_TIMER_H_
#define SRC_HAL_TIMER_H_

#include "xscutimer.h"

class Timer
{
public:
	Timer(int deviceId){ mDeviceId = deviceId; };

	int Init(void);

	void Delay(int uSDelay);

private:
	int mDeviceId;
	//Timer Instance
	XScuTimer mTimerInstance;

};



#endif /* SRC_HAL_TIMER_H_ */
