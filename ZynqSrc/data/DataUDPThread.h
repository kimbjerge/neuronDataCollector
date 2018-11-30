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
#include "Switch.h"

#define DATA_UDP_PORT 			26090
#define REMOTE_IP_CFG           IP4_ADDR(&RemoteAddr,  192, 168, 1, 20)

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
	Switch sw;
	int counter;
	LRECORD lxRecord;
	TestDataGenerator dataGenerator;
	// Global Variables to store results and handle data flow
	char HelloStr[256];
};


#endif /* SRC_DATAUDPTHREAD_H_ */
