/*
 * file_execution_test.c
 *
 *  Created on: Sep 3, 2014
 *      Author: stevem
 */

#include "hpp.h"
#include "file_loader.h"
#include "FreeRTOS.h"
#include "task.h"
#include "xil_cache.h"
#include "fsbl.h"
#include "image_mover.h"
#include "xil_mmu.h"

/*

#define portCPU_IRQ_DISABLE()										\
	__asm volatile ( "CPSID i" );									\
	__asm volatile ( "DSB" );										\
	__asm volatile ( "ISB" );

#define portCPU_IRQ_ENABLE()										\
	__asm volatile ( "CPSIE i" );									\
	__asm volatile ( "DSB" );										\
	__asm volatile ( "ISB" );

// Exception handlers
static void RegisterHandlers(void);
static void Undef_Handler (void);
static void SVC_Handler (void);
static void PreFetch_Abort_Handler (void);
static void Data_Abort_Handler (void);
static void IRQ_Handler (void);
static void FIQ_Handler (void); */

void bit_execution_test(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
	( void ) pcCommandString;
	( void ) xWriteBufferLen;
	configASSERT( pcWriteBuffer );

	u8 status = 0;

	//file_info.address = FILE_BASE_ADDR;
	//strcpy(file_info.filename, "hpp_top.bit.bin");
	//file_info.size = 5979968;
	//file_info.type = 0;

	status = Load_Bitstream();
	xil_printf("\n\rLoad_Bitstream status = %d", status);

	vTaskDelete(NULL);
}




void elf_execution_test(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
	( void ) pcCommandString;
	( void ) xWriteBufferLen;
	configASSERT( pcWriteBuffer );

	file_info.address = FILE_BASE_ADDR;
	strcpy(file_info.filename, "hello_world.elf.bin");
	file_info.size = 32780;
	file_info.type = 1;


	taskENTER_CRITICAL();

	vTaskSuspendAll();
	vTaskEndScheduler();


	//Xil_DCacheFlush();
	//Xil_DCacheInvalidate();
	Xil_DCacheDisable();

	//Xil_ICacheInvalidate();
	Xil_ICacheDisable();

	//Load_Application(file_info); //KBE???
	Load_Application();

	vTaskDelete(NULL);
}

/*
void elf_test()
{
	void (*execute_new_app)(void);
	execute_new_app = FILE_EXE_ADDR + 0x54C;


	//Xil_SetTlbAttributes((FILE_EXE_ADDR & 0xfff00000), 0x07dee);

	taskENTER_CRITICAL();

	vTaskSuspendAll();
	vTaskEndScheduler();


	//Xil_DCacheFlush();
	//Xil_DCacheInvalidate();
	Xil_DCacheDisable();

	//Xil_ICacheInvalidate();
	Xil_ICacheDisable();

	Xil_DisableMMU();

	execute_new_app();
}
*/

void elf_test()
{
	taskENTER_CRITICAL();
	xil_printf("\n\rELF TEST CRITICAL\n\r");
	ClearFSBLIn();
	vTaskSuspendAll();
	vTaskEndScheduler();
	xil_printf("\n\rELF TEST : HANDOFF\n\r\n\r");
	FsblHandoffExit(file_info.address);
}


/*
void elf_test()
{
	#define PS7_POST_CONFIG

	u8 Status = 0;

	//vTaskEndScheduler();

	//portCPU_IRQ_DISABLE();
	//taskENTER_CRITICAL();

	file_info.address = FILE_BASE_ADDR;
	strcpy(file_info.filename, "hello_world.elf.bin");
	file_info.size = 32780;
	file_info.type = 1;

	xil_printf("\n\rFlushing DCache\n\r");
	Xil_DCacheFlush();

	xil_printf("Disabling DCache\n\r");
	Xil_DCacheDisable();

	xil_printf("Registering Handlers\n\r");
	RegisterHandlers();

	xil_printf("Clearing FSBL mark in reboot register\n\r");
	ClearFSBLIn();

	xil_printf("SlcrLock()\n\r");
	SlcrLock();

	//xil_printf("Getting partition header information\n\r");
	//Status = GetPartitionHeaderInfo(FILE_BASE_ADDR);

	xil_printf("\n\rLoading New Application\n\r");
	Load_Application(file_info);

	taskEXIT_CRITICAL();
	portCPU_IRQ_ENABLE();
	vTaskDelete(NULL);
}*/


/*
static void RegisterHandlers(void)
{
	Xil_ExceptionInit();


	 //Initialize the vector table. Register the stub Handler for each
	 //exception.

	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_UNDEFINED_INT,
					(Xil_ExceptionHandler)Undef_Handler,
					(void *) 0);
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_SWI_INT,
					(Xil_ExceptionHandler)SVC_Handler,
					(void *) 0);
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_PREFETCH_ABORT_INT,
				(Xil_ExceptionHandler)PreFetch_Abort_Handler,
				(void *) 0);
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_DATA_ABORT_INT,
				(Xil_ExceptionHandler)Data_Abort_Handler,
				(void *) 0);
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IRQ_INT,
				(Xil_ExceptionHandler)IRQ_Handler,(void *) 0);
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_FIQ_INT,
			(Xil_ExceptionHandler)FIQ_Handler,(void *) 0);

	Xil_ExceptionEnable();
}

static void Undef_Handler (void)
{
	fsbl_printf(DEBUG_GENERAL,"UNDEFINED_HANDLER\r\n");
	ErrorLockdown (EXCEPTION_ID_UNDEFINED_INT);
}

static void SVC_Handler (void)
{
	fsbl_printf(DEBUG_GENERAL,"SVC_HANDLER \r\n");
	ErrorLockdown (EXCEPTION_ID_SWI_INT);
}

static void PreFetch_Abort_Handler (void)
{
	fsbl_printf(DEBUG_GENERAL,"PREFETCH_ABORT_HANDLER \r\n");
	ErrorLockdown (EXCEPTION_ID_PREFETCH_ABORT_INT);
}

static void Data_Abort_Handler (void)
{
	fsbl_printf(DEBUG_GENERAL,"DATA_ABORT_HANDLER \r\n");
	ErrorLockdown (EXCEPTION_ID_DATA_ABORT_INT);
}

static void IRQ_Handler (void)
{
	fsbl_printf(DEBUG_GENERAL,"IRQ_HANDLER \r\n");
	ErrorLockdown (EXCEPTION_ID_IRQ_INT);
}

static void FIQ_Handler (void)
{
	fsbl_printf(DEBUG_GENERAL,"FIQ_HANDLER \r\n");
	ErrorLockdown (EXCEPTION_ID_FIQ_INT);
}*/
