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
#include "HPPDataGenerator.h"
#include "HPPDataSDGenerator.h"
#include "TemplateMatch.h"
#include "Config.h"

// main_hpp.c in directory hpp calls main_full in directory Full_Demo (A UART Cli interface to FreeRTOS)
int main_hpp( void );
void TestFileSDCard(void);

NeuronChannels neuronChannels;
//TestDataGenerator testDataGenerator;
//DataUDPThread dataThread(&testDataGenerator);
HPPDataGenerator hppDataGenerator;
DataUDPThread dataThread(&hppDataGenerator);
CliCommand    cliCommand(&neuronChannels, &dataThread);
CliTCPThread  cliThread;

TestDataSDCard testDataSDCard;
HPPDataSDGenerator HPPSDGenerator(&testDataSDCard);
//TemplateMatch mTemplateMatch(&testDataSDCard); // Read next sample data from SD Card
TemplateMatch mTemplateMatch(&HPPSDGenerator); // Wait for new HPP samples before read next sample data from SD Card
Config 		  config;

int main()
{
	//init_net_server(echo_tcp_thread, 0);
	//init_net_server(demo_udp_thread, 0);
	//init_net_server(dataThread.threadMapper, &dataThread);
#if 1
	printf("-------------------------------------------------------------\r\n");
	printf("Neuron Real Time Template Matching Algorithm Version 1.6\r\n");
	printf("Loads data from SD card of 32 channels and max. 6 templates\r\n");
	printf("Template max. size of width 9 channels and 17 in length \r\n");
	printf("Performs 60 taps FIR filtering and NXCOR template matching\r\n");
	printf("Maximum 60 seconds of samples will be used from DATA.bin\r\n");
	printf("-------------------------------------------------------------\r\n");

	// Initialize HPP and FreeRTOS CLI using UART with neural spike processing demos
	main_hpp();

	//TestFileSDCard();

	printf("Read template configuration from CONFIG.txt\r\n");
	config.loadConfig("CONFIG.txt");
	config.loadTemplateConfig();
	printf("Read FIR coefficients from FIR.txt\r\n");
	config.loadCoeff("FIR.txt");
	config.loadCoeffBin("FIR.bin");

	printf("Reading test samples from DATA.bin\r\n");
	if (testDataSDCard.readFile((char *)"DATA.bin") == XST_SUCCESS)
	{
		mTemplateMatch.Init(&config, testDataSDCard.getNumSamples()); // Use number of data samples read from file
		mTemplateMatch.runThread(Thread::PRIORITY_NORMAL, "TemplateMatch");
	}

#else
	cliThread.addCommand(&cliCommand);
	init_net_server(cliThread.threadMapper, &cliThread);

	//UserThread mUserThread(Thread::PRIORITY_NORMAL, "UserControlThread");

	// Initialize HPP and FreeRTOS CLI using UART with neural spike processing demos
	main_hpp();
#endif

	/* Start FreeRTOS, the tasks running. */
	vTaskStartScheduler();

	/* If all is well, the scheduler will now be running, and the following line
	will never be reached.  If the following line does execute, then there was
	insufficient FreeRTOS heap memory available for the idle and/or timer tasks
	to be created.  See the memory management section on the FreeRTOS web site
	for more details. */
	for( ;; );

}
