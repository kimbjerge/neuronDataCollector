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
#include "NeuronData.h"
#include "Gpio.h"
#include "Leds.h"
#include "Switch.h"

#define DATA_UDP_PORT 			26090
#define REMOTE_IP_CFG           IP4_ADDR(&RemoteAddr,  192, 168, 1, 20)
#define NUM_TX_RECORDS			322 // Number of records in each UPD package  (46*7)*200 = 64400 (MTU 7*200=1400)

class DataUDPThread : public Thread
{
public:

	DataUDPThread(NeuronData *neuronData)
	{
		 counter = 0;
		 running = true;
		 pNeuronData = neuronData;
	}

	virtual void run();
	void setStreaming(bool run) { running = run; }

private:
	int create_bind_socket(unsigned port);
	void print_app_header();
	void PrintRecords(int cnt);

	Leds leds;
	Switch sw;
	int counter;
	bool running;
	LRECORD lxRecord[NUM_TX_RECORDS];
	NeuronData *pNeuronData;
};


#endif /* SRC_DATAUDPTHREAD_H_ */
