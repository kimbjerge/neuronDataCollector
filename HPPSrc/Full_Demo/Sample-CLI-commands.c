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
 *
 * See the following URL for information on the commands defined in this file:
 * http://www.FreeRTOS.org/FreeRTOS-Plus/FreeRTOS_Plus_UDP/Embedded_Ethernet_Examples/Ethernet_Related_CLI_Commands.shtml
 *
 ******************************************************************************/


/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"

/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* FreeRTOS+CLI includes. */
#include "FreeRTOS_CLI.h"
#include "file_execution_test.h"
#include "Nlx_Demos.h"
#include "file_loader.h"

#ifndef  configINCLUDE_TRACE_RELATED_CLI_COMMANDS
	#define configINCLUDE_TRACE_RELATED_CLI_COMMANDS 0
#endif


/*
 * Implements the task-stats command.
 */
static portBASE_TYPE prvTaskStatsCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );

/*
 * Implements the run-time-stats command.
 */
static portBASE_TYPE prvRunTimeStatsCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );

/*
 * Implements the echo-three-parameters command.
 */
static portBASE_TYPE prvThreeParameterEchoCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );

/*
 * Implements the echo-parameters command.
 */
static portBASE_TYPE prvParameterEchoCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );

/*
 * Implements the "trace start" and "trace stop" commands;
 */
#if configINCLUDE_TRACE_RELATED_CLI_COMMANDS == 1
	static portBASE_TYPE prvStartStopTraceCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString );
#endif

/* Structure that defines the "run-time-stats" command line command.   This
generates a table that shows how much run time each task has */
static const CLI_Command_Definition_t xRunTimeStats =
{
	"run-time-stats", /* The command string to type. */
	"\r\nrun-time-stats:\r\n Displays a table showing how much processing time each FreeRTOS task has used\r\n",
	prvRunTimeStatsCommand, /* The function to run. */
	0 /* No parameters are expected. */
};

/* Structure that defines the "task-stats" command line command.  This generates
a table that gives information on each task in the system. */
static const CLI_Command_Definition_t xTaskStats =
{
	"task-stats", /* The command string to type. */
	"\r\ntask-stats:\r\n Displays a table showing the state of each FreeRTOS task\r\n",
	prvTaskStatsCommand, /* The function to run. */
	0 /* No parameters are expected. */
};

/* Structure that defines the "echo_3_parameters" command line command.  This
takes exactly three parameters that the command simply echos back one at a
time. */
static const CLI_Command_Definition_t xThreeParameterEcho =
{
	"echo-3-parameters",
	"\r\necho-3-parameters <param1> <param2> <param3>:\r\n Expects three parameters, echos each in turn\r\n",
	prvThreeParameterEchoCommand, /* The function to run. */
	3 /* Three parameters are expected, which can take any value. */
};

/* Structure that defines the "echo_parameters" command line command.  This
takes a variable number of parameters that the command simply echos back one at
a time. */
static const CLI_Command_Definition_t xParameterEcho =
{
	"echo-parameters",
	"\r\necho-parameters <...>:\r\n Take variable number of parameters, echos each in turn\r\n",
	prvParameterEchoCommand, /* The function to run. */
	-1 /* The user can enter any number of commands. */
};

/* Structure that defines the "file_execution_test" command line command.  This generates
a table that gives information on each task in the system.
static const CLI_Command_Definition_t xbit_execution_test =
{
	"bit_execution_test", // The command string to type.
	"\r\nbit_execution_test:\r\n Executes hpp_top.bit.bin from address 0x20000000\r\n",
	bit_execution_test, // The function to run.
	0 // No parameters are expected.
};

 Structure that defines the "file_execution_test" command line command.  This generates
a table that gives information on each task in the system.
static const CLI_Command_Definition_t xelf_execution_test =
{
	"elf_execution_test", // The command string to type.
	"\r\nelf_execution_test:\r\n Executes hello_world.elf.bin from address 0x20000000\r\n",
	elf_execution_test, // The function to run.
	0 // No parameters are expected.
};*/

/* Structure that defines the "file_execution_test" command line command.  This generates
a table that gives information on each task in the system. */
static const CLI_Command_Definition_t xexp_ctrl_test =
{
	"exp_ctrl_test", // The command string to type.
	"\r\nexp_ctrl_test:\r\n Starts experiment control pulses\r\n",
	exp_ctrl_test, // The function to run.
	0 // No parameters are expected.
};

/* Structure that defines the "file_execution_test" command line command.  This generates
a table that gives information on each task in the system. */
static const CLI_Command_Definition_t xprint_num_ints =
{
	"print_num_ints", // The command string to type.
	"\r\nprint_num_ints:\r\n Prints the number of data buffers loaded to the PS\r\n",
	print_num_ints, // The function to run.
	0 // No parameters are expected.
};

/* Structure that defines the "vPrintTimeStamp" command line command.  This generates
a table that gives information on each task in the system. */
static const CLI_Command_Definition_t xprint_time_stamp =
{
	"printtimestamp", // The command string to type.
	"\r\nprinttimestamp:\r\n Prints the timestamp of the most recent record loaded to the PS\r\n",
	(const pdCOMMAND_LINE_CALLBACK)vPrintTimeStamp, // The function to run.
	0 // No parameters are expected.
};

/* Structure that defines the "Start_Spike_Detect" command line command.  This generates
a table that gives information on each task in the system. */
static const CLI_Command_Definition_t xstart_spike_detect =
{
	"start_spike_detect", // The command string to type.
	"\r\nstart_spike_detect:\r\n Starts the PS-based Spike Detection Demo\r\n",
	Start_Spike_Detect, // The function to run.
	0 // No parameters are expected.
};

/* Structure that defines the "stop_spike_detect" command line command.  This generates
a table that gives information on each task in the system. */
static const CLI_Command_Definition_t xstop_spike_detect =
{
	"stop_spike_detect", // The command string to type.
	"\r\nstop_spike_detect:\r\n Stops the PS-based Spike Detection Demo\r\n",
	Stop_Spike_Detect, // The function to run.
	0 // No parameters are expected.
};

/* Structure that defines the "Start_Ensemble_Detect" command line command.  This generates
a table that gives information on each task in the system. */
static const CLI_Command_Definition_t xstart_ensemble_detect =
{
	"start_ensemble_detect", // The command string to type.
	"\r\nstart_ensemble_detect:\r\n Starts the PS-based Ensemble Detection Demo\r\n",
	Start_Ensemble_Detect, // The function to run.
	0 // No parameters are expected.
};

/* Structure that defines the "Stop_Ensemble_Detect" command line command.  This generates
a table that gives information on each task in the system. */
static const CLI_Command_Definition_t xstop_ensemble_detect =
{
	"stop_ensemble_detect", // The command string to type.
	"\r\nstop_ensemble_detect:\r\n Stops the PS-based Ensemble Detection Demo\r\n",
	Stop_Ensemble_Detect, // The function to run.
	0 // No parameters are expected.
};

/* Structure that defines the "Start_Ensemble_Detect" command line command.  This generates
a table that gives information on each task in the system. */
static const CLI_Command_Definition_t xstart_pl_demos =
{
	"start_pl_demos", // The command string to type.
	"\r\nstart_pl_demos:\r\n Starts the PL-based Demos\r\n",
	Start_PL_Demos, // The function to run.
	0 // No parameters are expected.
};

/* Structure that defines the "Stop_Ensemble_Detect" command line command.  This generates
a table that gives information on each task in the system. */
static const CLI_Command_Definition_t xstop_pl_demos =
{
	"stop_pl_demos", // The command string to type.
	"\r\nstop_pl_demos:\r\n Stops the PL-based Demos\r\n",
	Stop_PL_Demos, // The function to run.
	0 // No parameters are expected.
};

/* Structure that defines the "Stop_Burst_Analysis" command line command.  This generates
a table that gives information on each task in the system. */
static const CLI_Command_Definition_t xstart_burst_analysis =
{
	"start_burst_analysis", // The command string to type.
	"\r\nstart_burst_analysis:\r\n Starts the PS-based Burst Analysis Demo\r\n",
	Start_Burst_Analysis, // The function to run.
	0 // No parameters are expected.
};

/* Structure that defines the "Stop_Burst_Analysis" command line command.  This generates
a table that gives information on each task in the system. */
static const CLI_Command_Definition_t xstop_burst_analysis =
{
	"stop_burst_analysis", // The command string to type.
	"\r\nstop_burst_analysis:\r\n Stops the PS-based Burst Analysis Demo\r\n",
	Stop_Burst_Analysis, // The function to run.
	0 // No parameters are expected.
};

/* Structure that defines the "Load_App" command line command.  This generates
a table that gives information on each task in the system. */
static const CLI_Command_Definition_t xload_app =
{
	"load_app", // The command string to type.
	"\r\nload_app:\r\n Loads the application specified by address FILE_SWITCH_ADDR\r\n",
	Load_App, // The function to run.
	0 // No parameters are expected.
};

/* Structure that defines the "Store_File" command line command.  This generates
a table that gives information on each task in the system. */
static const CLI_Command_Definition_t xstore_app =
{
	"store_file", // The command string to type.
	"\r\nstore_file:\r\n Stores a file to flash; expects file name and destination offset (in hex, with 0x prepended)\r\n",
	Store_File, // The function to run.
	2 // No parameters are expected.
};

/* Structure that defines the "Write_FP" command line command.  This generates
a table that gives information on each task in the system. */
static const CLI_Command_Definition_t xwrite_fp =
{
	"write_fp", // The command string to type.
	"\r\nwrite_fp:\r\n Stores a file parameters to flash; expects \"defaults\" or \"new\" with new flash parameters\r\n\tParam 1: FW Version\r\n\tParam 2: ID\r\n\tParam 3: MAC Addr\r\n\tParam 4: IP Addr\n\r",
	Write_FP, // The function to run.
	-1 // No parameters are expected.
};


#if configINCLUDE_TRACE_RELATED_CLI_COMMANDS == 1
	/* Structure that defines the "trace" command line command.  This takes a single
	parameter, which can be either "start" or "stop". */
	static const CLI_Command_Definition_t xStartStopTrace =
	{
		"trace",
		"\r\ntrace [start | stop]:\r\n Starts or stops a trace recording for viewing in FreeRTOS+Trace\r\n",
		prvStartStopTraceCommand, /* The function to run. */
		1 /* One parameter is expected.  Valid values are "start" and "stop". */
	};
#endif /* configINCLUDE_TRACE_RELATED_CLI_COMMANDS */

/*-----------------------------------------------------------*/

void vRegisterSampleCLICommands( void )
{
	/* Register all the command line commands defined immediately above. */
	FreeRTOS_CLIRegisterCommand( &xTaskStats );
	FreeRTOS_CLIRegisterCommand( &xRunTimeStats );
	FreeRTOS_CLIRegisterCommand( &xThreeParameterEcho );
	FreeRTOS_CLIRegisterCommand( &xParameterEcho );
	//FreeRTOS_CLIRegisterCommand( &xbit_execution_test ); //Added STM
	//FreeRTOS_CLIRegisterCommand( &xelf_execution_test ); //Added STM
	FreeRTOS_CLIRegisterCommand( &xexp_ctrl_test ); 	 //Added STM
	FreeRTOS_CLIRegisterCommand( &xprint_num_ints ); 	 //Added STM
	FreeRTOS_CLIRegisterCommand( &xprint_time_stamp ); 	 //Added STM
	FreeRTOS_CLIRegisterCommand( &xstart_spike_detect ); 	 //Added STM
	FreeRTOS_CLIRegisterCommand( &xstop_spike_detect ); 	 //Added STM
	FreeRTOS_CLIRegisterCommand( &xstart_burst_analysis ); 	 //Added STM
	FreeRTOS_CLIRegisterCommand( &xstop_burst_analysis ); 	 //Added STM
	FreeRTOS_CLIRegisterCommand( &xstart_ensemble_detect ); 	 //Added STM
	FreeRTOS_CLIRegisterCommand( &xstop_ensemble_detect ); 	 //Added STM
	FreeRTOS_CLIRegisterCommand( &xstart_pl_demos ); 	 //Added STM
	FreeRTOS_CLIRegisterCommand( &xstop_pl_demos ); 	 //Added STM
	FreeRTOS_CLIRegisterCommand( &xload_app );	//Added STM
	FreeRTOS_CLIRegisterCommand( &xstore_app );	//Added STM
	FreeRTOS_CLIRegisterCommand( &xwrite_fp );	//Added STM


	#if( configINCLUDE_TRACE_RELATED_CLI_COMMANDS == 1 )
	{
		FreeRTOS_CLIRegisterCommand( & xStartStopTrace );
	}
	#endif
}
/*-----------------------------------------------------------*/

static portBASE_TYPE prvTaskStatsCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
const char *const pcHeader = "Task          State  Priority  Stack	#\r\n************************************************\r\n";

	/* Remove compile time warnings about unused parameters, and check the
	write buffer is not NULL.  NOTE - for simplicity, this example assumes the
	write buffer length is adequate, so does not check for buffer overflows. */
	( void ) pcCommandString;
	( void ) xWriteBufferLen;
	configASSERT( pcWriteBuffer );

	/* Generate a table of task stats. */
	strcpy( pcWriteBuffer, pcHeader );
	vTaskList( pcWriteBuffer + strlen( pcHeader ) );

	/* There is no more data to return after this single string, so return
	pdFALSE. */
	return pdFALSE;
}
/*-----------------------------------------------------------*/

static portBASE_TYPE prvRunTimeStatsCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
const char * const pcHeader = "Task            Abs Time      % Time\r\n****************************************\r\n";

	/* Remove compile time warnings about unused parameters, and check the
	write buffer is not NULL.  NOTE - for simplicity, this example assumes the
	write buffer length is adequate, so does not check for buffer overflows. */
	( void ) pcCommandString;
	( void ) xWriteBufferLen;
	configASSERT( pcWriteBuffer );

	/* Generate a table of task stats. */
	strcpy( pcWriteBuffer, pcHeader );

	//Commented out STM 18Sept14 since I removed this functionality within FreeRTOSConfig.h
	//vTaskGetRunTimeStats( pcWriteBuffer + strlen( pcHeader ) );

	/* There is no more data to return after this single string, so return
	pdFALSE. */
	return pdFALSE;
}
/*-----------------------------------------------------------*/

static portBASE_TYPE prvThreeParameterEchoCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
const char *pcParameter;
portBASE_TYPE xParameterStringLength, xReturn;
static portBASE_TYPE lParameterNumber = 0;

	/* Remove compile time warnings about unused parameters, and check the
	write buffer is not NULL.  NOTE - for simplicity, this example assumes the
	write buffer length is adequate, so does not check for buffer overflows. */
	( void ) pcCommandString;
	( void ) xWriteBufferLen;
	configASSERT( pcWriteBuffer );

	if( lParameterNumber == 0 )
	{
		/* The first time the function is called after the command has been
		entered just a header string is returned. */
		sprintf( pcWriteBuffer, "The three parameters were:\r\n" );

		/* Next time the function is called the first parameter will be echoed
		back. */
		lParameterNumber = 1L;

		/* There is more data to be returned as no parameters have been echoed
		back yet. */
		xReturn = pdPASS;
	}
	else
	{
		/* Obtain the parameter string. */
		pcParameter = FreeRTOS_CLIGetParameter
						(
							pcCommandString,		/* The command string itself. */
							lParameterNumber,		/* Return the next parameter. */
							&xParameterStringLength	/* Store the parameter string length. */
						);

		/* Sanity check something was returned. */
		configASSERT( pcParameter );

		/* Return the parameter string. */
		memset( pcWriteBuffer, 0x00, xWriteBufferLen );
		sprintf( pcWriteBuffer, "%d: ", ( int ) lParameterNumber );
		strncat( pcWriteBuffer, pcParameter, xParameterStringLength );
		strncat( pcWriteBuffer, "\r\n", strlen( "\r\n" ) );

		/* If this is the last of the three parameters then there are no more
		strings to return after this one. */
		if( lParameterNumber == 3L )
		{
			/* If this is the last of the three parameters then there are no more
			strings to return after this one. */
			xReturn = pdFALSE;
			lParameterNumber = 0L;
		}
		else
		{
			/* There are more parameters to return after this one. */
			xReturn = pdTRUE;
			lParameterNumber++;
		}
	}

	return xReturn;
}
/*-----------------------------------------------------------*/

static portBASE_TYPE prvParameterEchoCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
{
const char *pcParameter;
portBASE_TYPE xParameterStringLength, xReturn;
static portBASE_TYPE lParameterNumber = 0;

	/* Remove compile time warnings about unused parameters, and check the
	write buffer is not NULL.  NOTE - for simplicity, this example assumes the
	write buffer length is adequate, so does not check for buffer overflows. */
	( void ) pcCommandString;
	( void ) xWriteBufferLen;
	configASSERT( pcWriteBuffer );

	if( lParameterNumber == 0 )
	{
		/* The first time the function is called after the command has been
		entered just a header string is returned. */
		sprintf( pcWriteBuffer, "The parameters were:\r\n" );

		/* Next time the function is called the first parameter will be echoed
		back. */
		lParameterNumber = 1L;

		/* There is more data to be returned as no parameters have been echoed
		back yet. */
		xReturn = pdPASS;
	}
	else
	{
		/* Obtain the parameter string. */
		pcParameter = FreeRTOS_CLIGetParameter
						(
							pcCommandString,		/* The command string itself. */
							lParameterNumber,		/* Return the next parameter. */
							&xParameterStringLength	/* Store the parameter string length. */
						);

		if( pcParameter != NULL )
		{
			/* Return the parameter string. */
			memset( pcWriteBuffer, 0x00, xWriteBufferLen );
			sprintf( pcWriteBuffer, "%d: ", ( int ) lParameterNumber );
			strncat( pcWriteBuffer, pcParameter, xParameterStringLength );
			strncat( pcWriteBuffer, "\r\n", strlen( "\r\n" ) );

			/* There might be more parameters to return after this one. */
			xReturn = pdTRUE;
			lParameterNumber++;
		}
		else
		{
			/* No more parameters were found.  Make sure the write buffer does
			not contain a valid string. */
			pcWriteBuffer[ 0 ] = 0x00;

			/* No more data to return. */
			xReturn = pdFALSE;

			/* Start over the next time this command is executed. */
			lParameterNumber = 0;
		}
	}

	return xReturn;
}
/*-----------------------------------------------------------*/

#if configINCLUDE_TRACE_RELATED_CLI_COMMANDS == 1

	static portBASE_TYPE prvStartStopTraceCommand( char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString )
	{
	const char *pcParameter;
	portBASE_TYPE lParameterStringLength;

		/* Remove compile time warnings about unused parameters, and check the
		write buffer is not NULL.  NOTE - for simplicity, this example assumes the
		write buffer length is adequate, so does not check for buffer overflows. */
		( void ) pcCommandString;
		( void ) xWriteBufferLen;
		configASSERT( pcWriteBuffer );

		/* Obtain the parameter string. */
		pcParameter = FreeRTOS_CLIGetParameter
						(
							pcCommandString,		/* The command string itself. */
							1,						/* Return the first parameter. */
							&lParameterStringLength	/* Store the parameter string length. */
						);

		/* Sanity check something was returned. */
		configASSERT( pcParameter );

		/* There are only two valid parameter values. */
		if( strncmp( pcParameter, "start", strlen( "start" ) ) == 0 )
		{
			/* Start or restart the trace. */
			vTraceStop();
			vTraceClear();
			vTraceStart();

			sprintf( pcWriteBuffer, "Trace recording (re)started.\r\n" );
		}
		else if( strncmp( pcParameter, "stop", strlen( "stop" ) ) == 0 )
		{
			/* End the trace, if one is running. */
			vTraceStop();
			sprintf( pcWriteBuffer, "Stopping trace recording.\r\n" );
		}
		else
		{
			sprintf( pcWriteBuffer, "Valid parameters are 'start' and 'stop'.\r\n" );
		}

		/* There is no more data to return after this single string, so return
		pdFALSE. */
		return pdFALSE;
	}

#endif /* configINCLUDE_TRACE_RELATED_CLI_COMMANDS */
