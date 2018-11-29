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

#include "DataUDPThread.h"
#include "UserThread.h"

DataUDPThread dataThread;

int main()
{
	//init_net_server(echo_tcp_thread, 0);
	//init_net_server(demo_udp_thread, 0);
	init_net_server(dataThread.threadMapper, &dataThread);

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
