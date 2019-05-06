/*
 * main.cpp
 *
 *  Created on: 20. July 2018
 *      Author: Kim Bjerge
 */
#include <defsNet.h>

#include "CliCmdTemplates.h"
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

TestDataSDCard testDataSDCard;
HPPDataSDGenerator HPPSDGenerator(&testDataSDCard);
#if 0 // For fast time measure reading data from SD card as fast as possible, (Leds and TestIO must be disabled)
TemplateMatch mTemplateMatch(&testDataSDCard); // Read next sample data from SD Card
#else
TemplateMatch mTemplateMatch(&HPPSDGenerator); // Wait for new HPP samples before read next sample data from SD Card
#endif

//TestDataGenerator testDataGenerator;
//DataUDPThread dataThread(&testDataGenerator);
HPPDataGenerator hppDataGenerator;
DataUDPThread dataThread(&hppDataGenerator);
Config 		  config;
CliTCPThread  cliThread;
CliCommand    cliCommand(&mTemplateMatch, &dataThread, &testDataSDCard);

static int16_t *GenSamples(void)
{
	return HPPSDGenerator.GenerateSamples();
}

int main()
{
	//init_net_server(echo_tcp_thread, 0);
	//init_net_server(demo_udp_thread, 0);
	//init_net_server(dataThread.threadMapper, &dataThread);
#if 1
	printf("--------------------------------------------------------------------\r\n");
	printf("Real Time Neuron Template Matching Algorithm Version %d.%d\r\n", VERSION_HI, VERSION_LO);
	printf("--------------------------------------------------------------------\r\n");
	printf("Loads test data from SD card of 32 channels and max. 6 templates\r\n");
	printf("Template max. size of width 9 channels and 17 in length\r\n");
	printf("Performs 6xIIR SOS filtering and NXCOR template matching\r\n");
	printf("Loads samples DATA.bin, configuration CONFIG.txt, filter IIR.txt\r\n");
	printf("Maximum 60 seconds of samples and NXCOR will be logged for debugging\r\n");
	printf("--------------------------------------------------------------------\r\n");
	printf("Control operation remotely on Ethernet 198.168.1.10 port 7 using telnet\r\n");
	printf("SX TTL port 2 bit 0-5 goes high when a template 1-6 match is found\r\n");
	printf("SX TTL port 2 bit 7 toggles when template 1/2 is seen within window\r\n");
	printf("SX TTL port 3 bit 0 is set high during processing and analyzing data\r\n");
	printf("--------------------------------------------------------------------\r\n");

	//HPPSDGenerator.setFromSDCard(true); // Data from SD Card or Digital Lynx SX
	HPPSDGenerator.addCliCommand(&cliCommand); // Use CLI interface to control processing mode
	testDataSDCard.setFuncToGenSamples(&GenSamples); // Add function to get sample data when running process mode 3 (Storing HPPDATA.BIN file)

	// Initialize HPP and FreeRTOS CLI using UART with neural spike processing demos
	main_hpp();

	printf("Read template configuration from CONFIG.txt\r\n");
	config.loadConfig("CONFIG.txt");
	config.loadTemplateConfig();

	printf("Read IIR coefficients from IIR.txt or IIR.bin\r\n");
	config.loadCoeff("IIR.txt");
	config.loadCoeffBin("IIR.bin");

	printf("Reading test data samples from DATA.bin\r\n");
	testDataSDCard.readFile((char *)"DATA.bin");

	mTemplateMatch.Init(&config, testDataSDCard.getNumSamples()); // Use number of data samples read from file

	#if 0
		mTemplateMatch.runThread(Thread::PRIORITY_NORMAL, "TemplateMatch");
	#else
		// Initialize network and command CLI socket TCP interface
		cliThread.addCommand(&cliCommand);
		init_net_server(cliThread.threadMapper, &cliThread);
	#endif

#else
	// Testing user thread controlling TTL ouputs and LEDS
	UserThread mUserThread(Thread::PRIORITY_NORMAL, "UserControlThread");

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
