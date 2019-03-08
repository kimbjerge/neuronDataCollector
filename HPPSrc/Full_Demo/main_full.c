/*
    FreeRTOS V8.0.0 - Copyright (C) 2014 Real Time Engineers Ltd.
    All rights reserved

    VISIT http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS provides completely free yet professionally developed,    *
     *    robust, strictly quality controlled, supported, and cross          *
     *    platform software that has become a de facto standard.             *
     *                                                                       *
     *    Help yourself get started quickly and support the FreeRTOS         *
     *    project by purchasing a FreeRTOS tutorial book, reference          *
     *    manual, or both from: http://www.FreeRTOS.org/Documentation        *
     *                                                                       *
     *    Thank you!                                                         *
     *                                                                       *
    ***************************************************************************

    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation >>!AND MODIFIED BY!<< the FreeRTOS exception.

    >>! NOTE: The modification to the GPL is included to allow you to distribute
    >>! a combined work that includes FreeRTOS without being obliged to provide
    >>! the source code for proprietary components outside of the FreeRTOS
    >>! kernel.

    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  Full license text is available from the following
    link: http://www.freertos.org/a00114.html

    1 tab == 4 spaces!

    ***************************************************************************
     *                                                                       *
     *    Having a problem?  Start by reading the FAQ "My application does   *
     *    not run, what could be wrong?"                                     *
     *                                                                       *
     *    http://www.FreeRTOS.org/FAQHelp.html                               *
     *                                                                       *
    ***************************************************************************

    http://www.FreeRTOS.org - Documentation, books, training, latest versions,
    license and Real Time Engineers Ltd. contact details.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, a DOS
    compatible FAT file system, and our tiny thread aware UDP/IP stack.

    http://www.OpenRTOS.com - Real Time Engineers ltd license FreeRTOS to High
    Integrity Systems to sell under the OpenRTOS brand.  Low cost OpenRTOS
    licenses offer ticketed support, indemnification and middleware.

    http://www.SafeRTOS.com - High Integrity Systems also provide a safety
    engineered and independently SIL3 certified version for use in safety and
    mission critical applications that require provable dependability.

    1 tab == 4 spaces!
*/

/******************************************************************************
 * NOTE 1:  This project provides two demo applications.  A simple blinky style
 * project, and a more comprehensive test and demo application.  The
 * mainCREATE_SIMPLE_BLINKY_DEMO_ONLY setting in main.c is used to select
 * between the two.  See the notes on using mainCREATE_SIMPLE_BLINKY_DEMO_ONLY
 * in main.c.  This file implements the comprehensive test and demo version.
 *
 * NOTE 2:  This file only contains the source code that is specific to the
 * full demo.  Generic functions, such FreeRTOS hook functions, and functions
 * required to configure the hardware, are defined in main.c.
 *
 * NOTE 3:  The full demo includes a test that checks the floating point context
 * is maintained correctly across task switches.  The standard GCC libraries can
 * use floating point registers and made this test fail (unless the tasks that
 * use the library are given a floating point context as described on the
 * documentation page for this demo).  printf-stdarg.c is included in this
 * project to prevent the standard GCC libraries being linked into the project.
 *
 ******************************************************************************
 *
 * main_full() creates all the demo application tasks and software timers, then
 * starts the scheduler.  The web documentation provides more details of the
 * standard demo application tasks, which provide no particular functionality,
 * but do provide a good example of how to use the FreeRTOS API.
 *
 * In addition to the standard demo tasks, the following tasks and tests are
 * defined and/or created within this file:
 *
 * FreeRTOS+CLI command console.  The command console is access through the
 * UART to USB connector on the ZC702 Zynq development board (marked J2).  For
 * reasons of robustness testing the UART driver is deliberately written to be
 * inefficient and should not be used as a template for a production driver.
 * Type "help" to see a list of registered commands.  The FreeRTOS+CLI license
 * is different to the FreeRTOS license, see http://www.FreeRTOS.org/cli for
 * license and usage details.  The default baud rate is 115200.
 *
 * "Reg test" tasks - These fill both the core and floating point registers with
 * known values, then check that each register maintains its expected value for
 * the lifetime of the task.  Each task uses a different set of values.  The reg
 * test tasks execute with a very low priority, so get preempted very
 * frequently.  A register containing an unexpected value is indicative of an
 * error in the context switching mechanism.
 *
 * "Check" task - The check task period is initially set to three seconds.  The
 * task checks that all the standard demo tasks, and the register check tasks,
 * are not only still executing, but are executing without reporting any errors.
 * If the check task discovers that a task has either stalled, or reported an
 * error, then it changes its own execution period from the initial three
 * seconds, to just 200ms.  The check task also toggles an LED each time it is
 * called.  This provides a visual indication of the system status:  If the LED
 * toggles every three seconds, then no issues have been discovered.  If the LED
 * toggles every 200ms, then an issue has been discovered with at least one
 * task.
 */

/* Standard includes. */
#include <stdio.h>

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"

/* Standard demo application includes. */
#include "flop.h"
#include "semtest.h"
#include "dynamic.h"
#include "BlockQ.h"
#include "blocktim.h"
#include "countsem.h"
#include "GenQTest.h"
#include "recmutex.h"
#include "death.h"
#include "partest.h"
#include "comtest2.h"
#include "serial.h"
#include "TimerDemo.h"
#include "QueueOverwrite.h"
#include "IntQueue.h"
#include "EventGroupsDemo.h"
#include "hpp.h"
#include "Nlx_Demos.h"
#include "hpp_testing.h"
#include "file_execution_test.h"

// The stack size and priority used by the UART command console task.
#define mainUART_COMMAND_CONSOLE_STACK_SIZE	(uint16_t)50000
// KBE??? changed
#define mainUART_COMMAND_CONSOLE_TASK_PRIORITY	( 2 )

/* KBE???
// Priorities for the demo application tasks.
#define mainSEM_TEST_PRIORITY				( tskIDLE_PRIORITY + 1UL )
#define mainBLOCK_Q_PRIORITY				( tskIDLE_PRIORITY + 2UL )
#define mainCREATOR_TASK_PRIORITY			( tskIDLE_PRIORITY + 3UL )
#define mainFLOP_TASK_PRIORITY				( tskIDLE_PRIORITY )
//Changed STM 10Oct14
//#define mainUART_COMMAND_CONSOLE_STACK_SIZE	( configMINIMAL_STACK_SIZE * 3UL )
#define mainCOM_TEST_TASK_PRIORITY			( tskIDLE_PRIORITY + 2 )
#define mainCHECK_TASK_PRIORITY				( configMAX_PRIORITIES - 1 )
#define mainQUEUE_OVERWRITE_PRIORITY		( tskIDLE_PRIORITY )


// The LED used by the check timer.
#define mainCHECK_LED						( 0 )

// A block time of zero simply means "don't block".
#define mainDONT_BLOCK						( 0UL )

// The period after which the check timer will expire, in ms, provided no errors
// have been reported by any of the standard demo tasks.  ms are converted to the
// equivalent in ticks using the portTICK_PERIOD_MS constant.
#define mainNO_ERROR_CHECK_TASK_PERIOD		( 3000UL / portTICK_PERIOD_MS )

// The period at which the check timer will expire, in ms, if an error has been
// reported in one of the standard demo tasks.  ms are converted to the equivalent
// in ticks using the portTICK_PERIOD_MS constant.
#define mainERROR_CHECK_TASK_PERIOD 		( 200UL / portTICK_PERIOD_MS )

// Parameters that are passed into the register check tasks solely for the
// purpose of ensuring parameters are passed into tasks correctly.
#define mainREG_TEST_TASK_1_PARAMETER		( ( void * ) 0x12345678 )
#define mainREG_TEST_TASK_2_PARAMETER		( ( void * ) 0x87654321 )

// The base period used by the timer test tasks.
#define mainTIMER_TEST_PERIOD				( 50 )

-----------------------------------------------------------*/

/*
 * Called by main() to run the full demo (as opposed to the blinky demo) when
 * mainCREATE_SIMPLE_BLINKY_DEMO_ONLY is set to 0.
 */
void main_full( void );

/*
 * The check task, as described at the top of this file.
 */
//static void prvCheckTask( void *pvParameters );

/*
 * Register check tasks, and the tasks used to write over and check the contents
 * of the FPU registers, as described at the top of this file.  The nature of
 * these files necessitates that they are written in an assembly file, but the
 * entry points are kept in the C file for the convenience of checking the task
 * parameter.
 */
//static void prvRegTestTaskEntry1( void *pvParameters );
extern void vRegTest1Implementation( void );
//static void prvRegTestTaskEntry2( void *pvParameters );
extern void vRegTest2Implementation( void );

/*
 * Register commands that can be used with FreeRTOS+CLI.  The commands are
 * defined in CLI-Commands.c and File-Related-CLI-Command.c respectively.
 */
extern void vRegisterSampleCLICommands( void );

/*
 * The task that manages the FreeRTOS+CLI input and output.
 */
extern void vUARTCommandConsoleStart( uint16_t usStackSize, UBaseType_t uxPriority );

/*
 * A high priority task that does nothing other than execute at a pseudo random
 * time to ensure the other test tasks don't just execute in a repeating
 * pattern.
 */
//static void prvPseudoRandomiser( void *pvParameters );

/*-----------------------------------------------------------*/

/* The following two variables are used to communicate the status of the
register check tasks to the check task.  If the variables keep incrementing,
then the register check tasks has not discovered any errors.  If a variable
stops incrementing, then an error has been found. */
volatile unsigned long ulRegTest1LoopCounter = 0UL, ulRegTest2LoopCounter = 0UL;

/*-----------------------------------------------------------*/


extern SemaphoreHandle_t xHPP_Data_Int_Sem;
extern SemaphoreHandle_t xHPP_Ack_Int_Sem;
extern SemaphoreHandle_t xHPP_Data_Sem;
extern SemaphoreHandle_t xHPP_Response_Sem;


void main_full( void )
{
	//BaseType_t xReturn;

	/* Start the tasks that implements the command console on the UART*/
#ifdef STOP_HPP_TASKS
	xil_printf("UART command console not started\n\r");
#else
	vUARTCommandConsoleStart( mainUART_COMMAND_CONSOLE_STACK_SIZE, mainUART_COMMAND_CONSOLE_TASK_PRIORITY );
	xil_printf("UART command console started\n\r");
#endif

	/* Register the standard CLI commands. */
	vRegisterSampleCLICommands();

	vSetupHPP();

	//xReturn = xTaskCreate(vTestLogicCommands, "vTestLogicCommands", 10000, NULL, 5, NULL);
	//xil_printf("vTestLogicCommands created with return value of %d\n\r", xReturn);


	//xReturn = xTaskCreate(vTestCheetahEmulatedCommands, "vTestCheetahEmulatedCommands", 10000, NULL, 5, NULL);
	//xil_printf("vTestCheetahEmulatedCommands created with return value of %d\n\r", xReturn);

	//vTestDemoApps();

	//SetPSControl(Logic_Ctrl);

	//The PCAP will not work when running from a JTAG connection, so the following commented code is not usable for debug
	//xReturn = xTaskCreate(bit_execution_test, "bit_execution_test", 10000, NULL, 5, NULL);
	//xil_printf("bit_execution_test created with return value of %d\n\r", xReturn);

	//xReturn = xTaskCreate(elf_test, "elf_test", 50000, NULL, 5, NULL);
	//xil_printf("elf_test created with return value of %d\n\r", xReturn);


//    Xil_Out32(FPGA_RST_CTRL, 0x0);
//    Xil_Out32(FPGA_RST_CTRL, 0xf);
//    Xil_Out32(FPGA_RST_CTRL, 0x0);

	/* Start the scheduler. */
	//vTaskStartScheduler(); //KBE???


	/* If all is well, the scheduler will now be running, and the following
	line will never be reached.  If the following line does execute, then
	there was either insufficient FreeRTOS heap memory available for the idle
	and/or timer tasks to be created, or vTaskStartScheduler() was called from
	User mode.  See the memory management section on the FreeRTOS web site for
	more details on the FreeRTOS heap http://www.freertos.org/a00111.html.  The
	mode from which main() is called is set in the C start up code and must be
	a privileged mode (not user mode). */
	//for( ;; );	//KBE???
}



//FreeRTOS-Specific Demo Functions
/*-----------------------------------------------------------*/
/*
static void prvCheckTask( void *pvParameters )
{
TickType_t xDelayPeriod = mainNO_ERROR_CHECK_TASK_PERIOD;
TickType_t xLastExecutionTime;
static unsigned long ulLastRegTest1Value = 0, ulLastRegTest2Value = 0;
unsigned long ulErrorFound = pdFALSE;

	// Just to stop compiler warnings.
	( void ) pvParameters;

	// Initialise xLastExecutionTime so the first call to vTaskDelayUntil()	works correctly.
	xLastExecutionTime = xTaskGetTickCount();

	// Cycle for ever, delaying then checking all the other tasks are still
	// operating without error.  The onboard LED is toggled on each iteration.
	// If an error is detected then the delay period is decreased from
	// mainNO_ERROR_CHECK_TASK_PERIOD to mainERROR_CHECK_TASK_PERIOD.  This has the
	// effect of increasing the rate at which the onboard LED toggles, and in so
	// doing gives visual feedback of the system status.
	for( ;; )
	{
		// Delay until it is time to execute again.
		vTaskDelayUntil( &xLastExecutionTime, xDelayPeriod );

		// Check all the demo tasks (other than the flash tasks) to ensure
		// that they are all still running, and that none have detected an error.
		if( xAreIntQueueTasksStillRunning() != pdTRUE )
		{
			ulErrorFound = pdTRUE;
		}

		if( xAreMathsTaskStillRunning() != pdTRUE )
		{
			ulErrorFound = pdTRUE;
		}

		if( xAreDynamicPriorityTasksStillRunning() != pdTRUE )
		{
			ulErrorFound = pdTRUE;
		}

		if( xAreBlockingQueuesStillRunning() != pdTRUE )
		{
			ulErrorFound = pdTRUE;
		}

		if ( xAreBlockTimeTestTasksStillRunning() != pdTRUE )
		{
			ulErrorFound = pdTRUE;
		}

		if ( xAreGenericQueueTasksStillRunning() != pdTRUE )
		{
			ulErrorFound = pdTRUE;
		}

		if ( xAreRecursiveMutexTasksStillRunning() != pdTRUE )
		{
			ulErrorFound = pdTRUE;
		}

		if( xIsCreateTaskStillRunning() != pdTRUE )
		{
			ulErrorFound = pdTRUE;
		}

		if( xAreSemaphoreTasksStillRunning() != pdTRUE )
		{
			ulErrorFound = pdTRUE;
		}

		if( xAreTimerDemoTasksStillRunning( ( TickType_t ) mainNO_ERROR_CHECK_TASK_PERIOD ) != pdPASS )
		{
			ulErrorFound = pdTRUE;
		}

		if( xAreCountingSemaphoreTasksStillRunning() != pdTRUE )
		{
			ulErrorFound = pdTRUE;
		}

		if( xIsQueueOverwriteTaskStillRunning() != pdPASS )
		{
			ulErrorFound = pdTRUE;
		}

		if( xAreEventGroupTasksStillRunning() != pdPASS )
		{
			ulErrorFound = pdTRUE;
		}

		// Check that the register test 1 task is still running.
		if( ulLastRegTest1Value == ulRegTest1LoopCounter )
		{
			ulErrorFound = pdTRUE;
		}
		ulLastRegTest1Value = ulRegTest1LoopCounter;

		// Check that the register test 2 task is still running.
		if( ulLastRegTest2Value == ulRegTest2LoopCounter )
		{
			ulErrorFound = pdTRUE;
		}
		ulLastRegTest2Value = ulRegTest2LoopCounter;

		// Toggle the check LED to give an indication of the system status.  If
		// the LED toggles every mainNO_ERROR_CHECK_TASK_PERIOD milliseconds then
		// everything is ok.  A faster toggle indicates an error.
		//vParTestToggleLED( mainCHECK_LED );

		if( ulErrorFound != pdFALSE )
		{
			// An error has been detected in one of the tasks - flash the LED
			// at a higher frequency to give visible feedback that something has
			// gone wrong (it might just be that the loop back connector required
			// by the comtest tasks has not been fitted).
			xDelayPeriod = mainERROR_CHECK_TASK_PERIOD;
		}
	}
}


static void prvRegTestTaskEntry1( void *pvParameters )
{
	// Although the regtest task is written in assembler, its entry point is
	// written in C for convenience of checking the task parameter is being passed
	// in correctly.
	if( pvParameters == mainREG_TEST_TASK_1_PARAMETER )
	{
		// The reg test task also tests the floating point registers.  Tasks
		// that use the floating point unit must call vPortTaskUsesFPU() before
		// any floating point instructions are executed.
		vPortTaskUsesFPU();

		// Start the part of the test that is written in assembler.
		vRegTest1Implementation();
	}

	// The following line will only execute if the task parameter is found to
	// be incorrect.  The check timer will detect that the regtest loop counter is
	// not being incremented and flag an error.
	vTaskDelete( NULL );
}


static void prvRegTestTaskEntry2( void *pvParameters )
{
	// Although the regtest task is written in assembler, its entry point is
	// written in C for convenience of checking the task parameter is being passed
	// in correctly.
	if( pvParameters == mainREG_TEST_TASK_2_PARAMETER )
	{
		// The reg test task also tests the floating point registers.  Tasks
		// that use the floating point unit must call vPortTaskUsesFPU() before
		// any floating point instructions are executed.
		vPortTaskUsesFPU();

		// Start the part of the test that is written in assembler.
		vRegTest2Implementation();
	}

	// The following line will only execute if the task parameter is found to
	// be incorrect.  The check timer will detect that the regtest loop counter is
	// not being incremented and flag an error.
	vTaskDelete( NULL );
}


static void prvPseudoRandomiser( void *pvParameters )
{
const uint32_t ulMultiplier = 0x015a4e35UL, ulIncrement = 1UL, ulMinDelay = ( 35 / portTICK_PERIOD_MS );
volatile uint32_t ulNextRand = ( uint32_t ) &pvParameters, ulValue;

	// This task does nothing other than ensure there is a little bit of
	// disruption in the scheduling pattern of the other tasks.  Normally this is
	// done by generating interrupts at pseudo random times.
	for( ;; )
	{
		ulNextRand = ( ulMultiplier * ulNextRand ) + ulIncrement;
		ulValue = ( ulNextRand >> 16UL ) & 0xffUL;

		if( ulValue < ulMinDelay )
		{
			ulValue = ulMinDelay;
		}

		vTaskDelay( ulValue );

		while( ulValue > 0 )
		{
			__asm volatile( "NOP" );
			__asm volatile( "NOP" );
			__asm volatile( "NOP" );
			__asm volatile( "NOP" );

			ulValue--;
		}
	}
}
*/





