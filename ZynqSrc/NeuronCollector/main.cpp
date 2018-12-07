/*
 * Empty C++ Application
 */

/*
 * main.cpp
 *
 *  Created on: 20. July 2018
 *      Author: Kim Bjerge
 */
#include <defsNet.h>

#include "NeuronChannels.h"
#include "CliCommand.h"
#include "DataUDPThread.h"
#include "CliTCPThread.h"
#include "UserThread.h"
#include "TestDataGenerator.h"
#include "TestDataSDCard.h"

NeuronChannels neuronChannels;
TestDataSDCard testDataSDCard;
TestDataGenerator testDataGenerator;
//DataUDPThread dataThread(&testDataGenerator);
DataUDPThread dataThread(&testDataSDCard);
CliCommand    cliCommand(&neuronChannels, &dataThread);
CliTCPThread  cliThread;

int main()
{
	//init_net_server(echo_tcp_thread, 0);
	//init_net_server(demo_udp_thread, 0);
	//init_net_server(dataThread.threadMapper, &dataThread);

	printf("Reading test samples from file: DATA.bin\r\n");
	if (testDataSDCard.readFile((char *)"DATA.bin") != XST_SUCCESS) return 0;
	cliThread.addCommand(&cliCommand);
	init_net_server(cliThread.threadMapper, &cliThread);

	UserThread mUserThread(Thread::PRIORITY_NORMAL, "UserControlThread");

	/* Start FreeRTOS, the tasks running. */
	vTaskStartScheduler();

	/* If all is well, the scheduler will now be running, and the following line
	will never be reached.  If the following line does execute, then there was
	insufficient FreeRTOS heap memory available for the idle and/or timer tasks
	to be created.  See the memory management section on the FreeRTOS web site
	for more details. */
	for( ;; );
}
