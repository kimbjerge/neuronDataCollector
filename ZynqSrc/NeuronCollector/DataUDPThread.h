/*
 * DataThread.h
 *
 *  Created on: 28. nov. 2018
 *      Author: au288681
 */

#ifndef SRC_DATAUDPTHREAD_H_
#define SRC_DATAUDPTHREAD_H_

#include "Thread.h"
using namespace AbstractOS;
#include "LynxRecord.h"
#include "TestDataGenerator.h"
#include "Gpio.h"
#include "Leds.h"

#define DATA_UDP_PORT 			7

class DataUDPThread : public Thread
{
public:

	DataUDPThread()
	{
		 counter = 0;
	}

	virtual void run();

private:
	int create_bind_socket(unsigned port);
	void print_app_header();

	Leds leds;
	int counter;
	LynxRecord lynxRecord;
	// Global Variables to store results and handle data flow
	char HelloStr[256];
};


#endif /* SRC_DATAUDPTHREAD_H_ */
