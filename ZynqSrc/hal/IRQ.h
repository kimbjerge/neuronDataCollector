/*
 * IRQ.h
 *
 *  Created on: 16. aug. 2017
 *      Author: Kim Bjerge
 */

#ifndef SRC_HAL_IRQ_H_
#define SRC_HAL_IRQ_H_

#include "xscugic.h"

class IRQ
{
public:
	IRQ(int deviceId): mDeviceId(deviceId) {};

	int Init(void);

	XScuGic *getScuGic(void) { return &mScuGic; }

private:
	int mDeviceId;
	//Interrupt Controller Instance
	XScuGic mScuGic;

};


#endif /* SRC_HAL_IRQ_H_ */
