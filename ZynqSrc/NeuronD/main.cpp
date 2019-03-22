/*
 * main.cpp
 *
 *  Created on: 20. July 2018
 *      Author: Kim Bjerge
 */
#include <defsNet.h>

//#include "NeuronChannels.h"
#include "CliCmdTemplates.h"
#include "DataUDPThread.h"
#include "CliTCPThread.h"
#include "UserThread.h"
#include "TestDataGenerator.h"
#include "TestDataSDCard.h"
#include "TemplateMatch.h"
#include "Config.h"

// main_hpp.c in directory hpp calls main_full in directory Full_Demo (A UART Cli interface to FreeRTOS)
int main_hpp( void );

//TestDataGenerator testDataGenerator;
//DataUDPThread dataThread(&testDataGenerator);

TestDataSDCard testDataSDCard;
DataUDPThread dataThread(&testDataSDCard); // Task to upload data using UDP
Config 		  config;
TemplateMatch mTemplateMatch(&testDataSDCard); // Task to perform neural template matching
CliCommand    cliCommand(&mTemplateMatch, &dataThread, &testDataSDCard); // Task to handle user command line interface (CLI) over sockets
CliTCPThread  cliThread; // Task to create and wait for socket connection

int main()
{
	//init_net_server(echo_tcp_thread, 0);
	//init_net_server(demo_udp_thread, 0);
	//init_net_server(dataThread.threadMapper, &dataThread);
	printf("-------------------------------------------------------------\r\n");
	printf("Neuron Real Time Template Matching Algorithm Version %d.%d\r\n", VERSION_HI, VERSION_LO);
	printf("Loads data from SD card of 32 channels and max. 6 templates\r\n");
	printf("Template max. size of width 9 channels and 17 in length \r\n");
	printf("Performs 60 taps FIR filtering and NXCOR template matching\r\n");
	printf("Maximum 60 seconds of samples will be used from DATA.bin\r\n");
	printf("-------------------------------------------------------------\r\n");

	printf("Read template configuration from CONFIG.txt\r\n");
	config.loadConfig("CONFIG.txt");
	config.loadTemplateConfig();
	printf("Read FIR coefficients from FIR.txt\r\n");
	config.loadCoeff("FIR.txt");
	config.loadCoeffBin("FIR.bin");

	printf("Reading test samples from DATA.bin\r\n");
	if (testDataSDCard.readFile((char *)"DATA.bin") != XST_SUCCESS) return 0;

	mTemplateMatch.Init(&config, testDataSDCard.getNumSamples()); // Use number of data samples read from file

#if 0
	mTemplateMatch.runThread(Thread::PRIORITY_NORMAL, "TemplateMatch");
#else
	// Initialize network and command CLI telnet interface
	cliThread.addCommand(&cliCommand);
	init_net_server(cliThread.threadMapper, &cliThread);
#endif
	//UserThread mUserThread(Thread::PRIORITY_NORMAL, "UserControlThread");

	// Initialize HPP and FreeRTOS CLI using UART with neural spike processing demos
	//main_hpp();

	/* Start FreeRTOS, the tasks running. */
	vTaskStartScheduler();

	/* If all is well, the scheduler will now be running, and the following line
	will never be reached.  If the following line does execute, then there was
	insufficient FreeRTOS heap memory available for the idle and/or timer tasks
	to be created.  See the memory management section on the FreeRTOS web site
	for more details. */
	for( ;; );

}
