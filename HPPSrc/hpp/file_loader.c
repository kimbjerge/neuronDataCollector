/*
 * file_loader.c
 *
 *  Created on: July 7, 2014
 *      Author: stevem
 */

#include "file_loader.h"
#include "string.h"
#include "xil_types.h"
#include "xil_printf.h"
#include "hpp.h"
#include "freertos.h"
#include "task.h"
#include "fsbl.h"
#include "semphr.h"
#include "xgpiops.h"
#include <stdlib.h>
#include "FreeRTOS_CLI.h"
#include "sleep.h"
#include "xil_cache.h"
#include "xqspips.h"
#include "ctype.h"


//Added STM 22Oct14
static XQspiPs QspiInst;

extern SemaphoreHandle_t xHPP_File_Sem;
extern XGpioPs xGpiops;
extern u32 gpio_bank2;
extern const struct HPP_File_Command* HPP_File_Cmd;

struct File_Params* File_Parameters;

struct File_Params file_info;

u32 num_files_loaded = 0;
u32 current_file_address = FILE_STORAGE_BASE_ADDR;


u32 QspiFlashMake;
u32 QspiFlashSize;

static struct Flash_Params Flash_Parameters;

/*
 * The following variables are used to read and write to the flash and they
 * are global to avoid having large buffers on the stack
 */
static u8 ReadBuffer[MAX_DATA + DATA_OFFSET + DUMMY_SIZE];
static u8 WriteBuffer[PAGE_SIZE + DATA_OFFSET];

void vFile_System_Task(void *pvNotUsed)
{
	(void)pvNotUsed;
	//u32 bitmask = 0;
	char temp_filename[256];
	//char temp_extension[4];
	u32 temp_size = 0;
	u32 temp_type = 0;
	s8 status = 0;
	u32 checksum = 0;
	u32 expected_checksum = 0;
	u32 temp_addr = 0;
	//u32 i = 0;
	//int Status = XST_SUCCESS;

	//Status = InitPcap();
	//if(Status != XST_SUCCESS)
	//{
	//	xil_printf("/n/rProblem with InitPcap.  Status = %d/n/r", Status);
	//}

	//SetPSReady(1);

	SetFTReady(PS_Ctrl);

	for(;;)
	{
		if(xSemaphoreTake(xHPP_File_Sem, portMAX_DELAY) == pdTRUE)
		{
			//Set PS Control
			gpio_bank2 |= 1 << CONTROL_BIT;
			XGpioPs_Write(&xGpiops, GPIO_BANK_2, gpio_bank2);

			//Write the acks and respond before processing commands

			/*bitmask = 7 << FILE_CMD_ID_LSB;
			bitmask = ~bitmask;
			gpio_bank2 &= bitmask;*/

			/*
			switch(HPP_File_Cmd[0].CommandID)
			{
				case 1:
						xil_printf("CommandID = 1\n\r");
						gpio_bank2 |= 1 << FILE_CMD_ID_LSB;
						gpio_bank2 |= 1 << FILE_INT_ACK_BIT;
						XGpioPs_Write(&xGpiops, GPIO_BANK_2, gpio_bank2);

						gpio_bank2 ^= 1 << FILE_CMD_ID_LSB;
						gpio_bank2 ^= 1 << FILE_INT_ACK_BIT;
						XGpioPs_Write(&xGpiops, GPIO_BANK_2, gpio_bank2);
						break;

				case 2:
						xil_printf("CommandID = 2\n\r");
						gpio_bank2 |= 2 << FILE_CMD_ID_MSB;
						gpio_bank2 |= 1 << FILE_INT_ACK_BIT;
						XGpioPs_Write(&xGpiops, GPIO_BANK_2, gpio_bank2);

						gpio_bank2 ^= 2 << FILE_CMD_ID_MSB;
						gpio_bank2 ^= 1 << FILE_INT_ACK_BIT;
						XGpioPs_Write(&xGpiops, GPIO_BANK_2, gpio_bank2);
						break;

				case 3:
						xil_printf("CommandID = 3\n\r");
						gpio_bank2 |= 3 << FILE_CMD_ID_LSB;
						gpio_bank2 |= 1 << FILE_INT_ACK_BIT;
						XGpioPs_Write(&xGpiops, GPIO_BANK_2, gpio_bank2);

						gpio_bank2 ^= 3 << FILE_CMD_ID_LSB;
						gpio_bank2 ^= 1 << FILE_INT_ACK_BIT;
						XGpioPs_Write(&xGpiops, GPIO_BANK_2, gpio_bank2);
						break;

				case 4:
						xil_printf("CommandID = 4\n\r");
						gpio_bank2 |= 4 << FILE_CMD_ID_LSB;
						gpio_bank2 |= 1 << FILE_INT_ACK_BIT;
						XGpioPs_Write(&xGpiops, GPIO_BANK_2, gpio_bank2);

						gpio_bank2 ^= 4 << FILE_CMD_ID_LSB;
						gpio_bank2 ^= 1 << FILE_INT_ACK_BIT;
						XGpioPs_Write(&xGpiops, GPIO_BANK_2, gpio_bank2);
						break;
				default:
						xil_printf("File Transfer CommandID %d is unknown\n\r", HPP_File_Cmd[0].CommandID);
						gpio_bank2 |= 1 << FILE_INT_ACK_BIT;
						XGpioPs_Write(&xGpiops, GPIO_BANK_2, gpio_bank2);
						gpio_bank2 ^= 1 << FILE_INT_ACK_BIT;
						XGpioPs_Write(&xGpiops, GPIO_BANK_2, gpio_bank2);
						break;
			}*/

			//Process commands
			switch(HPP_File_Cmd[0].CommandID)
			{
				case 1:
						//Making the Filename [256] in length and then having a u32 value for size was problematic due
						//to the whole size value not being aligned across a word...the code below fixes this and the
						//processor now interprets the size correctly


						SetPSReady(Logic_Ctrl);

						temp_type = (u32)HPP_File_Cmd[0].Filename[256] << 24;
						temp_type |= (u32)HPP_File_Cmd[0].Filename[257] << 16;
						temp_type |= (u32)HPP_File_Cmd[0].Filename[258] << 8;
						temp_type |= (u32)HPP_File_Cmd[0].Filename[259];
						File_Parameters[num_files_loaded].type = temp_type;


						//For testing only
//						if(File_Parameters[num_files_loaded].type == 0)
//						{
//							File_Parameters[num_files_loaded].address = BIT_EXE_ADDR;
//						}
//						else
//						{
//							File_Parameters[num_files_loaded].address = FILE_EXE_ADDR;
//						}



						temp_addr = (u32)HPP_File_Cmd[0].Filename[260] << 24;
						temp_addr |= (u32)HPP_File_Cmd[0].Filename[261] << 16;
						temp_addr |= (u32)HPP_File_Cmd[0].Filename[262] << 8;
						temp_addr |= (u32)HPP_File_Cmd[0].Filename[263];

						//Will need to uncomment the following when the address has been added to the message
						temp_size = (u32)HPP_File_Cmd[0].Filename[264] << 24;
						temp_size |= (u32)HPP_File_Cmd[0].Filename[265] << 16;
						temp_size |= (u32)HPP_File_Cmd[0].Filename[266] << 8;
						temp_size |= (u32)HPP_File_Cmd[0].Filename[267];



						File_Parameters[num_files_loaded].address = temp_addr;
						File_Parameters[num_files_loaded].size = temp_size;
						strcpy(File_Parameters[num_files_loaded].filename, HPP_File_Cmd[0].Filename);
						xil_printf("\n\rFile command 1 processed correctly for %s, with size = %d and address = 0x%x", File_Parameters[num_files_loaded].filename, File_Parameters[num_files_loaded].size, File_Parameters[num_files_loaded].address);
						break;

				case 3:
					/*
						temp_size = File_Parameters[num_files_loaded].size;
						temp_size = temp_size & 0x1FF;
						if(temp_size > 0)
						{
							temp_size = ((File_Parameters[num_files_loaded].size >> 9) + 1);
						}
						else
						{
							temp_size = (File_Parameters[num_files_loaded].size >> 9);
						}
						for(i = 0; i < temp_size; i++)
						{
							memcpy((int *)current_file_address, &HPP_File_Data[i].Data[0], 512);
							current_file_address = current_file_address + 512;
						}

						//update current_file_address
						if(current_file_address < FILE_HIGH_ADDR)
						{
							current_file_address = current_file_address + 0x1000;
							current_file_address = current_file_address & 0xFFFFF000;
						}
						else
						{
							current_file_address = FILE_STORAGE_BASE_ADDR;
							xil_printf("\n\rPROBLEM: Ran out of space for files\n\r");
						}*/

						checksum = calculate_checksum();

						expected_checksum = (u32)HPP_File_Cmd[0].Filename[3] << 24;
						expected_checksum |= (u32)HPP_File_Cmd[0].Filename[2] << 16;
						expected_checksum |= (u32)HPP_File_Cmd[0].Filename[1] << 8;
						expected_checksum |= (u32)HPP_File_Cmd[0].Filename[0];

						if(checksum == expected_checksum)
						{
							//update num_files_loaded
							File_Parameters[num_files_loaded].checksum = checksum;
							num_files_loaded++;
							xil_printf("\n\rFile command 3 processed correctly with %d files now loaded", num_files_loaded);
						}
						else
						{
							xil_printf("\n\rERROR: Transmitted checksum (0x%x) does not equal calculated checksum (0x%x)...problem with file transfer!!!!", expected_checksum, checksum);

							print_checksum();

							xil_printf("\n\rERROR: Transmitted checksum (0x%x) does not equal calculated checksum (0x%x)...problem with file transfer!!!!", expected_checksum, checksum);
						}
						break;
						//strcpy(temp_filename, "file_test.bin");
						//strcpy(temp_extension, "bit");
						//Load_File(temp_filename, temp_extension);

						SetPSReady(PS_Ctrl);

				case 4:
						//SetPSReady(0);

						//Added STM 16Oct14
						ClearPSGPIO();

						strcpy(temp_filename, HPP_File_Execute[0].Filename);
						//strcpy(temp_extension, HPP_File_Execute[0].Extension);
						//Load_File(temp_filename, temp_extension);
						//xil_printf("\n\rFile command 4 processed correctly.  Now executing %s", temp_filename);
						status = Load_File(temp_filename);
						xil_printf("\n\rLoad_File returned with status = %d", (int)status);

						//Added STM 16Oct14
						vSetup_HPP_GPIO();
						SetFTReady(PS_Ctrl);
						break;

				default:
						xil_printf("\n\rPROBLEM: Unknown File Transfer Command.  Command = %d", HPP_File_Cmd[0].CommandID);
						break;
			}
		}
	}
	vTaskDelete(NULL);
}

/*
s8 Load_File(char* file_name, char* extension)
{
	char* result = NULL;
	s8 status = 0;
	struct File_Params file_info;
	
	status = Calc_File_Params(file_name, &file_info);
	
	if(status != 0)
	{
		xil_printf("\n\rcalc_file_params() returned with status = %d\n\r", status);
		return status;
	}
	
	result = strstr(extension, "bit");
	if(result == NULL)
	{
		result = strstr(extension, "elf");
		if(result == NULL)
		{
			return -1;
		}
		else
		{
			Load_Application(file_info);
		}	
	}
	else
	{
		status = Load_Bitstream(file_info);
	}
	return status;
}*/

s8 Load_File(char* file_name)
{
	s8 status = 0;
	int xReturn = 0;

	//Debug only
	xil_printf("\n\rLoad_File called for %s", file_name);

	status = Calc_File_Params(file_name);


	if(status != 0)
	{
		xil_printf("\n\rcalc_file_params() returned with status = %d\n\r", status);
		return status;
	}


	SetPSReady(Logic_Ctrl);

	xil_printf("\n\rPS Ready Disabled");
	usleep(500000);

	xil_printf("\n\rnum_data_buffers_loaded set to 0");
	num_data_buffers_loaded = 0;


	if(file_info.type == 0)
	{
		//Debug only
		xil_printf("\n\rLoad_Bitstream called for %s", file_name);
		status = Load_Bitstream();
		xil_printf("\n\rLoad_Bitstream() returned with value %d", (int)status);
		//Assert PS control over messages to DLSX at start of day
		SetPSControl(PS_Ctrl);
		num_data_buffers_loaded = 0;
	}
	else
	{
		if(file_info.type == 1)
		{
			memcpy((int *)FILE_SWITCH_ADDR, &file_info.address, 4);
			memcpy((int *)FILE_SWITCH_SIZE, &file_info.size, 4);
			memcpy((int *)FILE_SWITCH_CHECKSUM, &file_info.checksum, 4);
			memcpy((int *)FILE_SWITCH_NAME, file_info.filename, 256);

			//Debug only
			xil_printf("\n\rLoad_Application called for %s", file_name);
			Load_Application();
			xil_printf("\n\rLoad_Application task created with return value of %d\n\r", xReturn);
		}
		else
		{
			return -1;
		}
	}
	return status;
}


s8 Calc_File_Params(char* file_name)
{
	s8 status = -1;
	u32 i = 0;
	s32 result = 0;
	
	for(i = 0; i < num_files_loaded; i++)
	{
		result = strcmp(file_name, File_Parameters[i].filename);
		if(result == 0)
		{
			strcpy(file_info.filename, File_Parameters[i].filename);
			file_info.address = File_Parameters[i].address;
			file_info.type = File_Parameters[i].type;
			file_info.size = File_Parameters[i].size;
			file_info.checksum = File_Parameters[i].checksum;

			//Debug only
			xil_printf("\n\rfile_info.filename = %s", file_info.filename);
			xil_printf("\n\rfile_info.address = 0x%08X", file_info.address);
			xil_printf("\n\rfile_info.type = %d", file_info.type);
			xil_printf("\n\rfile_info.size = %d", file_info.size);
			xil_printf("\n\rfile_info.checksum = 0x%08X", file_info.checksum);

			return 0;
		}
	}
	return status;
}


void Load_Application()
{
	int Status = -2;
	Status = ReadAppSwitcher();

	if(Status == 0)
	{
		taskENTER_CRITICAL();
		xil_printf("\n\rELF TEST CRITICAL\n\r");
		ClearFSBLIn();
		vTaskSuspendAll();
		vTaskEndScheduler();
		xil_printf("\n\rELF HANDOFF\n\r\n\r");
		FsblHandoffExit(0x20000400);
	}
	else //Problem with intermediate switching application checksum
	{
		xil_printf("\n\rChecksum of intermediate switching application does not match expected checksum...ABORTING application handoff...\n\r");
	}
}


u32 calculate_checksum()
{
	//STM 29Sept14 May need to do byte swapping below...

	u32 checksum = 0;
	u32 remaining_bytes = 0;
	u32 index = 0;
	u32* data_array = (u32 *)File_Parameters[num_files_loaded].address;


	Xil_DCacheFlush();

	for(index = 0; index < (File_Parameters[num_files_loaded].size >> 2); index++)
	{
		checksum ^= data_array[index];
	}

	xil_printf("\n\rWhole word checksum (%d words) =  0x%08X", (File_Parameters[num_files_loaded].size >> 2), checksum);

	remaining_bytes = File_Parameters[num_files_loaded].size - (File_Parameters[num_files_loaded].size & 0xFFFFFFFC);

	switch(remaining_bytes)
	{
		case 0:	break;
		case 1:
				checksum ^= (data_array[(File_Parameters[num_files_loaded].size >> 2)] & 0xff000000);
				break;
		case 2:
				checksum ^= (data_array[(File_Parameters[num_files_loaded].size >> 2)] & 0xffff0000);
				break;
		case 3:
				checksum ^= (data_array[(File_Parameters[num_files_loaded].size >> 2)] & 0xffffff00);
				break;
		default:break;
	}

	xil_printf("\n\rChecksum after remainder of %d bytes = 0x%08X", remaining_bytes, checksum);

	return checksum;
}




void print_checksum()
{
	//STM 29Sept14 May need to do byte swapping below...

	u32 checksum = 0;
	u32 remaining_bytes = 0;
	u32 index = 0;
	u32* data_array = (u32 *)File_Parameters[num_files_loaded].address;


	Xil_DCacheFlush();

	for(index = 0; index < (File_Parameters[num_files_loaded].size >> 2); index++)
	{
		checksum ^= data_array[index];
		xil_printf("\n\r%08x %08x", data_array[index], checksum);
	}


	remaining_bytes = File_Parameters[num_files_loaded].size - (File_Parameters[num_files_loaded].size & 0xFFFFFFFC);

	switch(remaining_bytes)
	{
		case 0:	break;
		case 1:
				checksum ^= (data_array[(File_Parameters[num_files_loaded].size >> 2)] & 0xff000000);
				xil_printf("\n\r%08x %08x", (data_array[(File_Parameters[num_files_loaded].size >> 2)] & 0xff000000), checksum);
				break;
		case 2:
				checksum ^= (data_array[(File_Parameters[num_files_loaded].size >> 2)] & 0xffff0000);
				xil_printf("\n\r%08x %08x", (data_array[(File_Parameters[num_files_loaded].size >> 2)] & 0xffff0000), checksum);
				break;
		case 3:
				checksum ^= (data_array[(File_Parameters[num_files_loaded].size >> 2)] & 0xffffff00);
				xil_printf("\n\r%08x %08x", (data_array[(File_Parameters[num_files_loaded].size >> 2)] & 0xffffff00), checksum);
				break;
		default:break;
	}
}


portBASE_TYPE Load_App(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
	(void)pcCommandString;
	(void)xWriteBufferLen;
	//(void)pcWriteBuffer;
	configASSERT(pcWriteBuffer);

	sprintf(pcWriteBuffer,"\n\rSwitching applications...\n\r");

	Load_Application();

	return pdFALSE;
}


portBASE_TYPE Store_File(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
	(void)pcCommandString;
	(void)xWriteBufferLen;
	//(void)pcWriteBuffer;
	configASSERT(pcWriteBuffer);

	char *filename;
	BaseType_t filename_len;
	s8 status = -1;
	char *destination_offset;
	u32 offset = 0;
	//BaseType_t offset_len;

	//SetupFlash(&QspiInst);

	filename = (char *)FreeRTOS_CLIGetParameter(pcCommandString, 1L, &filename_len);
	destination_offset = (char *)(&filename[filename_len + 1]);

	filename[filename_len] = '\0'; //STM 22Oct14...adding this just in case, since the name isn't explicitly passed in as a NULL terminated string...

	status = Calc_File_Params(filename);

	if(status == 0)  //File found in DDR, store it in flash
	{
		offset = (u32)strtol(destination_offset, NULL, 0);

		xil_printf("\n\rWriting %s to flash\n\r", filename);
		//Base address of file to copy is found in file_info.address
		//Size of file to copy is found in file_info.size
		Write_File_to_FLASH(&QspiInst, offset);
		sprintf(pcWriteBuffer,"\n\rFlash write complete\n\r");
	}
	else //File not found, output error message
	{
		sprintf(pcWriteBuffer,"\n\rStore_File ERROR: %s not found\n\r", filename);
	}

	return pdFALSE;
}

//This function assumes a maximum of 1 file per sector (64kB)
void Write_File_to_FLASH(XQspiPs *QspiInstPtr, u32 flash_offset) //flash_offset needs to be on a page boundary; ideally on a sector boundary
{
	u32 index = 0;
	u32 erase_size = 0;
	u8 *bufptr = 0;
	u32 *BufferPtr;
	u32 *file_data = (u32 *)(file_info.address);
	u32 num_pages = 0;

	//Base address of file to copy is found in file_info.address
	//Size of file to copy is found in file_info.size

	erase_size = ((file_info.size >> 16) + 1) << 16; //Erase the whole sector(s) for where to put the file

	//Erase the flash.
	FlashErase(QspiInstPtr, flash_offset, erase_size); //flash_offset needs to be on a page boundary; ideally on a sector boundary

	memset(ReadBuffer, 0x00, sizeof(ReadBuffer));

	num_pages = (file_info.size >> 8);
	if ((file_info.size & 0x000000ff) > 0)
	{
		num_pages++;
	}

	for(index = 0; index < num_pages; index++) //Write the file a page at a time
	{
		memcpy(&WriteBuffer[DATA_OFFSET], (int*)(file_info.address + (index << 8)), PAGE_SIZE);

		FlashWrite(QspiInstPtr, (index * PAGE_SIZE) + flash_offset, PAGE_SIZE, WRITE_CMD);
		xil_printf(".");
	}


	 // Read the contents of the FLASH from TEST_ADDRESS, using Normal Read
	 // command. Change the prescaler as the READ command operates at a
	 // lower frequency.
	xil_printf("\n\rFile written\n\r");
	memset(ReadBuffer, 0x00, sizeof(ReadBuffer));
	FlashRead(QspiInstPtr, flash_offset, ((file_info.size >> 8) + 1) << 8, READ_CMD);

	 // Setup a pointer to the start of the data that was read into the read
	 // buffer and verify the data read is the data that was written
	bufptr = &ReadBuffer[DATA_OFFSET]; //Reading words instead of bytes, so changing the index to account for that
	BufferPtr = (u32 *)&bufptr[0]; //Reading words instead of bytes, so changing the index to account for that

	for (index = 0; index < (file_info.size >> 2); index++)
	{
		if (BufferPtr[index] != file_data[index])
		{
			xil_printf("\n\rFile transfer data mismatch at index %d...Word in flash = 0x%08X Word in RAM = 0x%08X", index, BufferPtr[index], file_data[index]);
		}
	}
}






//*****************************************************************************
//
//
// This function erases the sectors in the  serial FLASH connected to the
// QSPI interface.
//
// @param	QspiPtr is a pointer to the QSPI driver component to use.
// @param	Address contains the address of the first sector which needs to
//		be erased.
// @param	ByteCount contains the total size to be erased.
//
// @return	None.
//
// @note		None.
//
//*****************************************************************************
void FlashErase(XQspiPs *QspiPtr, u32 Address, u32 ByteCount)
{
	u8 WriteEnableCmd = { WRITE_ENABLE_CMD };
	u8 ReadStatusCmd[] = { READ_STATUS_CMD, 0 };  // must send 2 bytes
	u8 FlashStatus[2];
	uint Sector;

	 // If erase size is same as the total size of the flash, use bulk erase
	 // command
	if (ByteCount == (NUM_SECTORS * SECTOR_SIZE)) {
		 // Send the write enable command to the FLASH so that it can be
		 // written to, this needs to be sent as a seperate transfer
		 // before the erase
		XQspiPs_PolledTransfer(QspiPtr, &WriteEnableCmd, NULL,
				  sizeof(WriteEnableCmd));

		 // Setup the bulk erase command
		WriteBuffer[COMMAND_OFFSET]   = BULK_ERASE_CMD;

		 // Send the bulk erase command; no receive buffer is specified
		 // since there is nothing to receive
		XQspiPs_PolledTransfer(QspiPtr, WriteBuffer, NULL,
					BULK_ERASE_SIZE);

		 // Wait for the erase command to the FLASH to be completed
		while (1) {
			 // Poll the status register of the device to determine
			 // when it completes, by sending a read status command
			 // and receiving the status byte
			XQspiPs_PolledTransfer(QspiPtr, ReadStatusCmd,
						FlashStatus,
						sizeof(ReadStatusCmd));

			 // If the status indicates the write is done, then stop
			 // waiting; if a value of 0xFF in the status byte is
			 // read from the device and this loop never exits, the
			 // device slave select is possibly incorrect such that
			 // the device status is not being read
			if ((FlashStatus[1] & 0x01) == 0) {
				break;
			}
		}

		return;
	}

	 // If the erase size is less than the total size of the flash, use
	 // sector erase command
	for (Sector = 0; Sector < ((ByteCount / SECTOR_SIZE) + 1); Sector++) {
		 // Send the write enable command to the SEEPOM so that it can be
		 // written to, this needs to be sent as a seperate transfer
		 // before the write
		XQspiPs_PolledTransfer(QspiPtr, &WriteEnableCmd, NULL,
					sizeof(WriteEnableCmd));

		 // Setup the write command with the specified address and data
		 // for the FLASH
		WriteBuffer[COMMAND_OFFSET]   = SEC_ERASE_CMD;
		WriteBuffer[ADDRESS_1_OFFSET] = (u8)(Address >> 16);
		WriteBuffer[ADDRESS_2_OFFSET] = (u8)(Address >> 8);
		WriteBuffer[ADDRESS_3_OFFSET] = (u8)(Address & 0xFF);

		 // Send the sector erase command and address; no receive buffer
		 // is specified since there is nothing to receive
		XQspiPs_PolledTransfer(QspiPtr, WriteBuffer, NULL,
					SEC_ERASE_SIZE);

		 // Wait for the sector erase command to the FLASH to be completed
		while (1) {
			 // Poll the status register of the device to determine
			 // when it completes, by sending a read status command
			 // and receiving the status byte
			XQspiPs_PolledTransfer(QspiPtr, ReadStatusCmd,
						FlashStatus,
						sizeof(ReadStatusCmd));

			 // If the status indicates the write is done, then stop
			 // waiting, if a value of 0xFF in the status byte is
			 // read from the device and this loop never exits, the
			 // device slave select is possibly incorrect such that
			 // the device status is not being read
			if ((FlashStatus[1] & 0x01) == 0) {
				break;
			}
		}

		Address += SECTOR_SIZE;
	}
}


//******************************************************************************
//
//
// This function writes to the  serial FLASH connected to the QSPI interface.
// All the data put into the buffer must be in the same page of the device with
// page boundaries being on 256 byte boundaries.
//
// @param	QspiPtr is a pointer to the QSPI driver component to use.
// @param	Address contains the address to write data to in the FLASH.
// @param	ByteCount contains the number of bytes to write.
// @param	Command is the command used to write data to the flash. QSPI
//		device supports only Page Program command to write data to the
//		flash.
//
// @return	None.
//
// @note		None.
//
//******************************************************************************
void FlashWrite(XQspiPs *QspiPtr, u32 Address, u32 ByteCount, u8 Command)
{
	u8 WriteEnableCmd = { WRITE_ENABLE_CMD };
	u8 ReadStatusCmd[] = { READ_STATUS_CMD, 0 };  /* must send 2 bytes */
	u8 FlashStatus[2];


	 // Send the write enable command to the FLASH so that it can be
	 // written to, this needs to be sent as a seperate transfer before
	 // the write
	XQspiPs_PolledTransfer(QspiPtr, &WriteEnableCmd, NULL,
				sizeof(WriteEnableCmd));


	// Setup the write command with the specified address and data for the
	// FLASH
	WriteBuffer[COMMAND_OFFSET]   = Command;
	WriteBuffer[ADDRESS_1_OFFSET] = (u8)((Address & 0xFF0000) >> 16);
	WriteBuffer[ADDRESS_2_OFFSET] = (u8)((Address & 0xFF00) >> 8);
	WriteBuffer[ADDRESS_3_OFFSET] = (u8)(Address & 0xFF);


	 // Send the write command, address, and data to the FLASH to be
	 // written, no receive buffer is specified since there is nothing to
	 // receive
	XQspiPs_PolledTransfer(QspiPtr, WriteBuffer, NULL,
				ByteCount + OVERHEAD_SIZE);


	 // Wait for the write command to the FLASH to be completed, it takes
	 // some time for the data to be written
	while (1) {
		 // Poll the status register of the FLASH to determine when it
		 // completes, by sending a read status command and receiving the
		 // status byte
		XQspiPs_PolledTransfer(QspiPtr, ReadStatusCmd, FlashStatus,
					sizeof(ReadStatusCmd));


		 // If the status indicates the write is done, then stop waiting,
		 // if a value of 0xFF in the status byte is read from the
		 // device and this loop never exits, the device slave select is
		 // possibly incorrect such that the device status is not being
		 // read
		if ((FlashStatus[1] & 0x01) == 0) {
			break;
		}
	}
}


//*****************************************************************************
//
// This function reads from the  serial FLASH connected to the
// QSPI interface.
//
// @param	QspiPtr is a pointer to the QSPI driver component to use.
// @param	Address contains the address to read data from in the FLASH.
// @param	ByteCount contains the number of bytes to read.
// @param	Command is the command used to read data from the flash. QSPI
//		device supports one of the Read, Fast Read, Dual Read and Fast
//		Read commands to read data from the flash.
//
// @return	None.
//
// @note		None.
//
//******************************************************************************/
void FlashRead(XQspiPs *QspiPtr, u32 Address, u32 ByteCount, u8 Command)
{
	 // Setup the write command with the specified address and data for the
	 // FLASH
	WriteBuffer[COMMAND_OFFSET]   = Command;
	WriteBuffer[ADDRESS_1_OFFSET] = (u8)((Address & 0xFF0000) >> 16);
	WriteBuffer[ADDRESS_2_OFFSET] = (u8)((Address & 0xFF00) >> 8);
	WriteBuffer[ADDRESS_3_OFFSET] = (u8)(Address & 0xFF);

	if ((Command == FAST_READ_CMD) || (Command == DUAL_READ_CMD) ||
	    (Command == QUAD_READ_CMD)) {
		ByteCount += DUMMY_SIZE;
	}
	 // Send the read command to the FLASH to read the specified number
	 // of bytes from the FLASH, send the read command and address and
	 // receive the specified number of bytes of data in the data buffer
	XQspiPs_PolledTransfer(QspiPtr, WriteBuffer, ReadBuffer,
				ByteCount + OVERHEAD_SIZE);
}


/******************************************************************************
*
* This function reads serial FLASH ID connected to the SPI interface.
*
* @param	None.
*
* @return	XST_SUCCESS if read id, otherwise XST_FAILURE.
*
* @note		None.
*
******************************************************************************/
int FlashReadID(void)
{
	int Status;

	/*
	 * Read ID in Auto mode.
	 */
	WriteBuffer[COMMAND_OFFSET]   = READ_ID;
	WriteBuffer[ADDRESS_1_OFFSET] = 0x23;		/* 3 dummy bytes */
	WriteBuffer[ADDRESS_2_OFFSET] = 0x08;
	WriteBuffer[ADDRESS_3_OFFSET] = 0x09;

	Status = XQspiPs_PolledTransfer(&QspiInst, WriteBuffer, ReadBuffer,
				RD_ID_SIZE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	//xil_printf("FlashID=0x%x 0x%x 0x%x\n\r", ReadBuffer[1], ReadBuffer[2], ReadBuffer[3]);

	return XST_SUCCESS;
}


void SetupFlash(XQspiPs *QspiInstPtr)
{
	int Status = 0;
	XQspiPs_Config *QspiConfig;
	BaseType_t xStatus;


	//Added QSPI calls below, STM 22Oct14
	QspiConfig = XQspiPs_LookupConfig(XPAR_XQSPIPS_0_DEVICE_ID);
	configASSERT(QspiConfig != NULL);

	xStatus = XQspiPs_CfgInitialize(&QspiInst, QspiConfig, QspiConfig->BaseAddress);
	configASSERT( xStatus == XST_SUCCESS );

	//QspiInstPtr = &QspiInst;


	Status = XQspiPs_SelfTest(&QspiInst);
	if (Status != XST_SUCCESS)
	{
		xil_printf("\n\rProblem with XQspiPs_SelfTest...Status = %d\n\r", Status);
	}


	//Set Auto Start and Manual Chip select options and drive HOLD_B pin high.
	XQspiPs_SetOptions(QspiInstPtr, XQSPIPS_FORCE_SSELECT_OPTION | XQSPIPS_HOLD_B_DRIVE_OPTION);

	//Do we need these? STM 22Oct14
	//Set the prescaler for QSPI clock
	XQspiPs_SetClkPrescaler(QspiInstPtr, XQSPIPS_CLK_PRESCALE_8);

	// Assert the FLASH chip select.
	XQspiPs_SetSlaveSelect(QspiInstPtr);

	FlashReadID();
}


void HPP_Flash_Init()
{
	SetupFlash(&QspiInst);

	xil_printf("\n\rHPP Parameters Loaded from Flash:\n\r");
	PrintFlashParams();

	//ReadAppSwitcher();
}



int ReadAppSwitcher()
{
	u8 *bufptr = 0;
	u32 *BufferPtr;

	u32 index = 0;

	int Status = -2;

	const u32 offset = Flash_Parameters.app_switcher_start_addr;
	const u32 size = Flash_Parameters.app_switcher_size;
	const u32 destination = Flash_Parameters.app_switcher_dest_addr;
	const u32 checksum = Flash_Parameters.app_switcher_checksum;
	u32 read_size = 0;

	Xil_DCacheFlush();

	//SetupFlash(&QspiInst);
	read_size = ((size >> 8) + 1) << 8;
	xil_printf("\n\rReading intermediate application switching file from flash offset 0x%08X (size of read = %d).\n\r", offset, read_size);

	memset(ReadBuffer, 0x00, sizeof(ReadBuffer));
	FlashRead(&QspiInst, offset, read_size, READ_CMD);

	// Setup a pointer to the start of the data that was read
	bufptr = &ReadBuffer[DATA_OFFSET]; //Reading words instead of bytes, so changing the index to account for that
	BufferPtr = (u32 *)&bufptr[0]; //Reading words instead of bytes, so changing the index to account for that

	xil_printf("\n\rErasing %d bytes starting at address 0x%08X.\n\r", ((size >> 2) + 1) << 2, destination);


	u32 *data = (u32 *)destination;

	for(index = 0; index < ((size >> 2) + 1); index++)
	{
		data[index] = 0;
	}



	xil_printf("\n\rCopying intermediate application switching file to 0x%08X.\n\r", destination);

	memcpy((int *)destination, BufferPtr, size);

	Status = Verify_Checksum(destination, size, checksum);

	return Status;
}


void ReadFlashParams(XQspiPs *QspiInstPtr)
{
	u8 *temp_buf;
	memset(ReadBuffer, 0x00, sizeof(ReadBuffer));
	FlashRead(QspiInstPtr, 0x00000000, PAGE_SIZE, READ_CMD);

	temp_buf = &ReadBuffer[DATA_OFFSET];
	memcpy(&Flash_Parameters, &temp_buf[0], sizeof(struct Flash_Params));

	//Need to apply defaults to the params if the flash read turns up 0s for critical values
	if(Flash_Parameters.app_switcher_start_addr == 0)
	{
		Flash_Parameters.app_switcher_start_addr = DEFAULT_APP_SWITCHER_BASE_ADDR;
	}
	if(Flash_Parameters.app_switcher_size == 0)
	{
		Flash_Parameters.app_switcher_size = DEFAULT_APP_SWITCHER_SIZE;
	}
	if(Flash_Parameters.app_switcher_dest_addr == 0)
	{
		Flash_Parameters.app_switcher_dest_addr = DEFAULT_APP_SWITCHER_DEST_ADDR;
	}
	if(Flash_Parameters.app_switcher_checksum == 0)
	{
		Flash_Parameters.app_switcher_checksum = DEFAULT_APP_SWITCHER_CHECKSUM;
	}
	if(Flash_Parameters.fw_version == 0)
	{
		Flash_Parameters.fw_version = DEFAULT_FW_VERSION;
	}
	if(Flash_Parameters.id == 0)
	{
		Flash_Parameters.id = DEFAULT_ID;
	}
	if(Flash_Parameters.ip_addr == 0)
	{
		Flash_Parameters.ip_addr[0] = DEFAULT_IP_ADDR & 0xFF;
		Flash_Parameters.ip_addr[1] = (DEFAULT_IP_ADDR >> 8) & 0xFF;
		Flash_Parameters.ip_addr[2] = (DEFAULT_IP_ADDR >> 16) & 0xFF;
		Flash_Parameters.ip_addr[3] = (DEFAULT_IP_ADDR >> 24) & 0xFF;
	}
	if(Flash_Parameters.mac_addr == 0)
	{
		memcpy(Flash_Parameters.mac_addr, DEFAULT_MAC_ADDR, 6);
	}
}


void WriteFlashParams(XQspiPs *QspiInstPtr, struct Flash_Params flash_parameters)
{
	//Erase the first sector (this is where the flash parameters reside)
	FlashErase(QspiInstPtr, 0, SECTOR_SIZE);

	memcpy(&WriteBuffer[DATA_OFFSET], (int*)(&flash_parameters), PAGE_SIZE);

	FlashWrite(QspiInstPtr, 0, PAGE_SIZE, WRITE_CMD);

	ReadFlashParams(QspiInstPtr);

	xil_printf("\n\rNew flash parameters stored to flash...\n\r");
	PrintFULLFlashParams();
}


void PrintFlashParams()
{
	u8 ip[4];
	unsigned char mac[6];
	unsigned char id_string[5];

	Xil_DCacheFlush();

	ReadFlashParams(&QspiInst);

	memcpy(ip, Flash_Parameters.ip_addr, 4);
	memcpy(mac, Flash_Parameters.mac_addr, 6);


	id_string[0] = (Flash_Parameters.id >> 24) & 0xFF;
	id_string[1] = (Flash_Parameters.id >> 16) & 0xFF;
	id_string[2] = (Flash_Parameters.id >> 8) & 0xFF;
	id_string[3] = Flash_Parameters.id & 0xFF;
	id_string[4] = '\0';

	xil_printf("ID:\t\t\t\t\t0x%08X (\"%s\")\n\r", (unsigned int)Flash_Parameters.id, id_string);
	printf("Firmware Version:\t\t\t%1.2f\n\r", Flash_Parameters.fw_version);
	xil_printf("HPP IP: \t\t\t\t%d.%d.%d.%d\n\r", ip[3], ip[2], ip[1], ip[0]);
	//xil_printf("HPP MAC: \t\t\t\t%.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n\r" , mac[5], mac[4], mac[3], mac[2], mac[1], mac[0]);
}


void PrintFULLFlashParams()
{
	u8 ip[4];
	unsigned char mac[6];
	unsigned char id_string[5];

	Xil_DCacheFlush();

	ReadFlashParams(&QspiInst);

	memcpy(ip, Flash_Parameters.ip_addr, 4);
	memcpy(mac, Flash_Parameters.mac_addr, 6);


	id_string[0] = (Flash_Parameters.id >> 24) & 0xFF;
	id_string[1] = (Flash_Parameters.id >> 16) & 0xFF;
	id_string[2] = (Flash_Parameters.id >> 8) & 0xFF;
	id_string[3] = Flash_Parameters.id & 0xFF;
	id_string[4] = '\0';

	xil_printf("ID:\t\t\t\t\t0x%08X (\"%s\")\n\r", (unsigned int)Flash_Parameters.id, id_string);
	printf("Firmware Version:\t\t\t%1.2f\n\r", Flash_Parameters.fw_version);
	xil_printf("HPP IP: \t\t\t\t%d.%d.%d.%d\n\r", ip[3], ip[2], ip[1], ip[0]);
	xil_printf("HPP MAC: \t\t\t\t%.2x:%.2x:%.2x:%.2x:%.2x:%.2x\n\r" , mac[5], mac[4], mac[3], mac[2], mac[1], mac[0]);
	xil_printf("AS Flash Addr:\t\t\t\t0x%08X\n\r", (unsigned int)Flash_Parameters.app_switcher_start_addr);
	xil_printf("AS Dest Addr:\t\t\t\t0x%08X\n\r", (unsigned int)Flash_Parameters.app_switcher_dest_addr);
	xil_printf("AS Size:\t\t\t\t%d\n\r", (unsigned int)Flash_Parameters.app_switcher_size);
	xil_printf("AS Checksum:\t\t\t\t0x%08X\n\r", (unsigned int)Flash_Parameters.app_switcher_checksum);
}


portBASE_TYPE Write_FP(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString)
{
	//(void)pcCommandString;
	(void)xWriteBufferLen;
	//(void)pcWriteBuffer;
	configASSERT(pcWriteBuffer);

	int8_t *pcParameter;
	BaseType_t lParameterStringLength, xReturn;

	/* Note that the use of the static parameter means this function is not reentrant. */
	static BaseType_t lParameterNumber = 0;

	int result = 0;
	int index = 0;
	int byte_count = 0;
	unsigned char *charptr = NULL;
	int *intptr;
	float *fltptr;



	if( lParameterNumber == 0 )
	{
		/* lParameterNumber is 0, so this is the first time the function has been
		called since the command was entered.  Return the string "The parameters
		were:" before returning any parameter strings. */
		//sprintf( pcWriteBuffer, "Writing the following parameters to flash:\r\n" );

		/* Next time the function is called the first parameter will be echoed
		back. */
		lParameterNumber = 1L;

		/* There is more data to be returned as no parameters have been echoed
		back yet, so set xReturn to pdPASS so the function will be called again. */
		xReturn = pdPASS;
	}
	else
	{
		/* lParameter is not 0, so holds the number of the parameter that should
		be returned.  Obtain the complete parameter string. */
		pcParameter = ( int8_t * ) FreeRTOS_CLIGetParameter
									(
										/* The command string itself. */
										pcCommandString,
										/* Return the next parameter. */
										lParameterNumber,
										/* Store the parameter string length. */
										&lParameterStringLength
									);

		//struct Flash_Params
//		{
//			float			fw_version;
//			u32				id;	//Unused at this time
//			unsigned char	mac_addr[6];
//			u32				ip_addr;
//			u32				app_switcher_start_addr;
//			u32				app_switcher_size;
//			u32				app_switcher_dest_addr;
//			char			reserved[220];
//		};
		if( pcParameter != NULL )
		{
			switch(lParameterNumber)
			{
				//defaults or new
				case 1:
						memcpy(charptr, pcParameter, lParameterStringLength);
						charptr[lParameterStringLength] = '\0';

						result = strcmp((char *)charptr, "defaults");
						if(result == 0)
						{
							Flash_Parameters.app_switcher_start_addr = DEFAULT_APP_SWITCHER_BASE_ADDR;
							Flash_Parameters.app_switcher_size = DEFAULT_APP_SWITCHER_SIZE;
							Flash_Parameters.app_switcher_dest_addr = DEFAULT_APP_SWITCHER_DEST_ADDR;
							Flash_Parameters.app_switcher_checksum = DEFAULT_APP_SWITCHER_CHECKSUM;
							Flash_Parameters.fw_version = DEFAULT_FW_VERSION;
							Flash_Parameters.id = DEFAULT_ID;
							Flash_Parameters.ip_addr[0] = DEFAULT_IP_ADDR & 0xFF;
							Flash_Parameters.ip_addr[1] = (DEFAULT_IP_ADDR >> 8) & 0xFF;
							Flash_Parameters.ip_addr[2] = (DEFAULT_IP_ADDR >> 16) & 0xFF;
							Flash_Parameters.ip_addr[3] = (DEFAULT_IP_ADDR >> 24) & 0xFF;
							memcpy(Flash_Parameters.mac_addr, DEFAULT_MAC_ADDR, 6);
							/* There should be more parameters to return after this one, so again
							set xReturn to pdTRUE. */
							xReturn = pdTRUE;
							lParameterNumber++;
						}
						else
						{
							memcpy(charptr, pcParameter, lParameterStringLength);
							charptr[lParameterStringLength] = '\0';

							result = strcmp((char *)charptr, "new");
							if(result == 0)
							{
								/* There should be more parameters to return after this one, so again
								set xReturn to pdTRUE. */
								xReturn = pdTRUE;
								lParameterNumber++;
							}
							else
							{
								sprintf( pcWriteBuffer, "\n\rProblem with command...expecting \"defaults\" or \"new\" as first parameter.\r\n");

								/* There is no more data to return, so this time set xReturn to
								pdFALSE. */
								xReturn = pdFALSE;

								/* Start over the next time this command is executed. */
								lParameterNumber = 0;
							}
						}
						break;
				//Firmware Version
				case 2:
						fltptr = (float *)&pcParameter;
						Flash_Parameters.fw_version = fltptr[0];
						xReturn = pdTRUE;
						lParameterNumber++;
						break;
				//ID
				case 3:
						intptr = (int *)&pcParameter;
						Flash_Parameters.id = intptr[0];
						xReturn = pdTRUE;
						lParameterNumber++;
						break;

				//MAC Address
				case 4:
						memcpy(charptr, pcParameter, lParameterStringLength);
						charptr[lParameterStringLength] = '\0';
						byte_count = 0;
						for(index = 0; index < lParameterStringLength; index++)
						{
							if(isalnum(charptr[index]) != 0)
							{
								Flash_Parameters.mac_addr[byte_count] = charptr[index];
								byte_count++;
							}
						}
						xReturn = pdTRUE;
						lParameterNumber++;
						break;

				//IP Address
				case 5:
						memcpy(charptr, pcParameter, lParameterStringLength);
						charptr[lParameterStringLength] = '\0';
						byte_count = 0;
						for(index = 0; index < lParameterStringLength; index++)
						{
							if(isalnum(charptr[index]) != 0)
							{
								Flash_Parameters.ip_addr[byte_count] = (u8)charptr[index];
								byte_count++;
							}
						}
						xReturn = pdTRUE;
						lParameterNumber++;
						break;
				default:
						sprintf( pcWriteBuffer, "More parameters were provided than expected\n\r");
						/* There is no more data to return, so this time set xReturn to
						pdFALSE. */
						xReturn = pdFALSE;

						/* Start over the next time this command is executed. */
						lParameterNumber = 0;
						break;
			}
		}
		else
		{
			if(lParameterNumber == 1)
			{
				/* There is no more data to return, so this time set xReturn to
				pdFALSE. */
				xReturn = pdFALSE;

				/* Start over the next time this command is executed. */
				lParameterNumber = 0;
			}
			//Need to setup the flash interface for writing
			//SetupFlash(&QspiInst);

			WriteFlashParams(&QspiInst, Flash_Parameters);

			sprintf( pcWriteBuffer, "\n\rParameters written to flash.\r\n" );

			/* No more parameters were found.  Make sure the write buffer does
			not contain a valid string to prevent junk being printed out. */
			//pcWriteBuffer[ 0 ] = 0x00;

			/* There is no more data to return, so this time set xReturn to
			pdFALSE. */
			xReturn = pdFALSE;

			/* Start over the next time this command is executed. */
			lParameterNumber = 0;
		}
	}

	return xReturn;

}

void print_generic_checksum(u32 address, u32 size)
{
	u32 checksum = 0;
	u32 remaining_bytes = 0;
	u32 index = 0;
	u32* data_array = (u32 *)address;

	Xil_DCacheFlush();

	xil_printf("\n\r\n\n\n\n");

	for(index = 0; index < (size >> 2); index++)
	{
		checksum ^= data_array[index];
		xil_printf("\n\r%08x %08x", data_array[index], checksum);
	}


	remaining_bytes = size - (size & 0xFFFFFFFC);

	switch(remaining_bytes)
	{
		case 0:	break;
		case 1:
				checksum ^= (data_array[(size >> 2)] & 0xff000000);
				xil_printf("\n\r%08x %08x", (data_array[(size >> 2)] & 0xff000000), checksum);
				break;
		case 2:
				checksum ^= (data_array[(size >> 2)] & 0xffff0000);
				xil_printf("\n\r%08x %08x", (data_array[(size >> 2)] & 0xffff0000), checksum);
				break;
		case 3:
				checksum ^= (data_array[(size >> 2)] & 0xffffff00);
				xil_printf("\n\r%08x %08x", (data_array[(size >> 2)] & 0xffffff00), checksum);
				break;
		default:break;
	}


	xil_printf("\n\r\n\n\n\n");
}

int Verify_Checksum(u32 address, u32 size, u32 checksum)
{
	u32 calc_checksum = 0;
	u32 remaining_bytes = 0;
	u32 index = 0;
	u32* data_array = (u32 *)address;

	Xil_DCacheFlush();

	for(index = 0; index < (size >> 2); index++)
	{
		calc_checksum ^= data_array[index];
		//xil_printf("\n\r%08x %08x", data_array[index], calc_checksum);
	}


	remaining_bytes = size - (size & 0xFFFFFFFC);

	switch(remaining_bytes)
	{
		case 0:	break;
		case 1:
				calc_checksum ^= (data_array[(size >> 2)] & 0xff000000);
				//xil_printf("\n\r%08x %08x", (data_array[(size >> 2)] & 0xff000000), calc_checksum);
				break;
		case 2:
				calc_checksum ^= (data_array[(size >> 2)] & 0xffff0000);
				//xil_printf("\n\r%08x %08x", (data_array[(size >> 2)] & 0xffff0000), calc_checksum);
				break;
		case 3:
				calc_checksum ^= (data_array[(size >> 2)] & 0xffffff00);
				//xil_printf("\n\r%08x %08x", (data_array[(size >> 2)] & 0xffffff00), calc_checksum);
				break;
		default:break;
	}

	if(calc_checksum == checksum)
	{
		return 0;
	}
	else
	{
		xil_printf("\n\rProblem with application checksum (calculated 0x%08X but expected 0x%08X)", calc_checksum, checksum);
		return -1;
	}
}
