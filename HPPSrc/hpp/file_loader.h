/*
 * file_loader.h
 *
 *  Created on: July 7, 2014
 *      Author: stevem
 */

#ifndef FILE_LOADER_H_
#define FILE_LOADER_H_

#include "xil_types.h"
#include "xparameters.h"
#include "xqspips.h"
#include "FreeRTOS.h"


#define PS_LVL_SHFTR_EN			(XPS_SYS_CTRL_BASEADDR + 0x900)
#define LVL_PS_PL 				0x0000000A
#define PCAP_CTRL_PCFG_AES_FUSE_EFUSE_MASK	0x1000

#define FILE_PARAM_BASE			0x2FFFFE00
#define FILE_SWITCH_ADDR 		0x2FFFFEF0
#define FILE_SWITCH_SIZE 		0x2FFFFEF4
#define FILE_SWITCH_CHECKSUM	0x2FFFFEF8
#define FILE_SWITCH_NAME 		0x2FFFFF00

struct File_Params
{
	char		filename[256];
	u32			address;
	u32			type;  //0 = bit file, 1 = application
	u32			size;  //Size in bytes
	u32			checksum;
	char		reserved[240];
};


extern struct File_Params* File_Parameters;
extern u32 num_files_loaded;
extern u32 current_file_address;
extern struct File_Params file_info;

void vFile_System_Task(void *pvNotUsed);

void Load_Application();
s8 Load_Bitstream();
//s8 Load_File(char* file_name, char* extension);
s8 Load_File(char* file_name);

s8 Calc_File_Params(char* file_name);

u32 calculate_checksum();
void print_checksum();

void FsblHandoff(u32 FsblStartAddr);

void HPPFabricInit(void);

portBASE_TYPE Load_App(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);

portBASE_TYPE Store_File(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);

void Write_File_to_FLASH(XQspiPs *QspiInstancePtr, u32 flash_offset);

void FlashErase(XQspiPs *QspiPtr, u32 Address, u32 ByteCount);

//STM 8July14
//The idea at this point is that each file is loaded into RAM (one at a time) at FILE_BASE_ADDR.
//The transactions are concluded with an interrupt to the PS.
//Since the PL should reset the address for the next file, it should not maintain the history of which address the last file ended at, etc.
//The PS should copy each file to flash and maintain a file system there.
//The file system should look something like what is shown below:

//u32 FileSystemVersion
//u32 NumberofFiles
//Reserved until flash address 0x1000
//Starting at flash address 0x1000
//Repeated for Number of Files (Array of File_Params structs)
//char[256] FileName
//u32 StartAddr
//u32 Size
//char[248] Reserved
//...
//u32 Reserved until first 256 MB boundary 
//Starting at 256 MB, files located as described by array above

//Need to read flash at start of day to get this info

//How am I going to synchronize the datamover address to the size of the file?  The PL doesn't currently know how large the files are as it doesn't read the messages at all...
//If I can memcpy the data to flash fast enough, each transfer can overwrite the previous one in DDR.
// Flash base address = XPAR_PS7_QSPI_LINEAR_0_S_AXI_BASEADDR;
//do I need to #include xqspips_hw.h and xqspips.h?



//Added 22oct14
//QSPI flash stuff for storing and reading intermediate application switching elf to/from FLASH
/*
 * The following constants specify the page size, sector size, and number of
 * pages and sectors for the FLASH.  The page size specifies a max number of
 * bytes that can be written to the FLASH with a single transfer.
 */
#define SECTOR_SIZE		0x10000
#define NUM_SECTORS		0x100
#define NUM_PAGES		0x10000
#define PAGE_SIZE		256

#define MAX_DATA		NUM_PAGES * PAGE_SIZE


#define QUAD_READ_CMD		0x6B
#define READ_ID_CMD			0x9F

#define WRITE_ENABLE_CMD	0x06
#define BANK_REG_RD			0x16
#define BANK_REG_WR			0x17
/* Bank register is called Extended Address Reg in Micron */
#define EXTADD_REG_RD		0xC8
#define EXTADD_REG_WR		0xC5


/*
 * The following constants define the commands which may be sent to the FLASH
 * device.
 */
#define WRITE_STATUS_CMD	0x01
#define WRITE_CMD		0x02
#define READ_CMD		0x03
#define WRITE_DISABLE_CMD	0x04
#define READ_STATUS_CMD		0x05
#define WRITE_ENABLE_CMD	0x06
#define FAST_READ_CMD		0x0B
#define DUAL_READ_CMD		0x3B
#define QUAD_READ_CMD		0x6B
#define BULK_ERASE_CMD		0xC7
#define	SEC_ERASE_CMD		0xD8
#define READ_ID			0x9F


/*
 * Identification of Flash
 * Micron:
 * Byte 0 is Manufacturer ID;
 * Byte 1 is first byte of Device ID - 0xBB or 0xBA
 * Byte 2 is second byte of Device ID describes flash size:
 * 128Mbit : 0x18; 256Mbit : 0x19; 512Mbit : 0x20
 * Spansion:
 * Byte 0 is Manufacturer ID;
 * Byte 1 is Device ID - Memory Interface type - 0x20 or 0x02
 * Byte 2 is second byte of Device ID describes flash size:
 * 128Mbit : 0x18; 256Mbit : 0x19; 512Mbit : 0x20
 */

#define MICRON_ID		0x20
#define SPANSION_ID		0x01
#define WINBOND_ID		0xEF

#define FLASH_SIZE_ID_128M		0x18
#define FLASH_SIZE_ID_256M		0x19
#define FLASH_SIZE_ID_512M		0x20
#define FLASH_SIZE_ID_1G		0x21

/*
 * Size in bytes
 */
#define FLASH_SIZE_128M			0x1000000
#define FLASH_SIZE_256M			0x2000000
#define FLASH_SIZE_512M			0x4000000
#define FLASH_SIZE_1G			0x8000000


/*
 * The following constants define the offsets within a FlashBuffer data
 * type for each kind of data.  Note that the read data offset is not the
 * same as the write data because the QSPI driver is designed to allow full
 * duplex transfers such that the number of bytes received is the number
 * sent and received.
 */
#define COMMAND_OFFSET		0 /* FLASH instruction */
#define ADDRESS_1_OFFSET	1 /* MSB byte of address to read or write */
#define ADDRESS_2_OFFSET	2 /* Middle byte of address to read or write */
#define ADDRESS_3_OFFSET	3 /* LSB byte of address to read or write */
#define DATA_OFFSET		4 /* Start of Data for Read/Write */
#define DUMMY_OFFSET		4 /* Dummy byte offset for fast, dual and quad
				     reads */
#define DUMMY_SIZE		1 /* Number of dummy bytes for fast, dual and
				     quad reads */
#define RD_ID_SIZE		4 /* Read ID command + 3 bytes ID response */
#define BULK_ERASE_SIZE		1 /* Bulk Erase command size */
#define SEC_ERASE_SIZE		4 /* Sector Erase command + Sector address */


/*
 * The following constants specify the extra bytes which are sent to the
 * FLASH on the QSPI interface, that are not data, but control information
 * which includes the command and address
 */
#define OVERHEAD_SIZE		4


void FlashErase(XQspiPs *QspiPtr, u32 Address, u32 ByteCount);
void FlashWrite(XQspiPs *QspiPtr, u32 Address, u32 ByteCount, u8 Command);
void FlashRead(XQspiPs *QspiPtr, u32 Address, u32 ByteCount, u8 Command);

int FlashReadID(void);

void HPP_Flash_Init(void);

void SetupFlash(XQspiPs *QspiInstPtr);
int ReadAppSwitcher(void);
void ReadFlashParams(XQspiPs *QspiInstPtr);
void PrintFlashParams();
void PrintFULLFlashParams();
portBASE_TYPE Write_FP(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
void print_generic_checksum(u32 address, u32 size);
int Verify_Checksum(u32 address, u32 size, u32 checksum);


//Default Flash Parameter Values
#define DEFAULT_APP_SWITCHER_BASE_ADDR	0x00010000
#define DEFAULT_APP_SWITCHER_SIZE		64576
#define DEFAULT_APP_SWITCHER_DEST_ADDR	0x20000400
#define DEFAULT_APP_SWITCHER_CHECKSUM	0xB82CB11D
#define DEFAULT_FW_VERSION				1.00
#define DEFAULT_ID						0x48505021 //ASCII "HPP!"
#define DEFAULT_MAC_ADDR				"000000" //Need to update
#define DEFAULT_IP_ADDR					0xC0A8050A //192.168.5.10


struct Flash_Params
{
	float			fw_version;
	u32				id;	//Unused at this time
	unsigned char	mac_addr[6];
	u8				ip_addr[4];
	u32				app_switcher_start_addr;
	u32				app_switcher_size;
	u32				app_switcher_dest_addr;
	u32				app_switcher_checksum;
	char			reserved[220];
};

#endif /* FILE_LOADER_H_ */
