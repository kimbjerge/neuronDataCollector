//http://forums.xilinx.com/t5/Embedded-Processor-System-Design/How-to-use-PCAP-to-config-the-PL-in-zynq/td-p/280230
/*
#include "xparameters.h"
#include "xdevcfg.h"
#include "file_loader.h"
#include "FreeRTOS.h"

extern XDcfg *DcfgInstPtr;

#define SLCR_LOCK	0xF8000004 // SLCR Write Protection Lock
#define SLCR_UNLOCK	0xF8000008 // SLCR Write Protection Unlock
#define SLCR_LVL_SHFTR_EN 0xF8000900 // SLCR Level Shifters Enable

#define SLCR_LOCK_VAL	0x767B
#define SLCR_UNLOCK_VAL	0xDF0D

s8 Load_Bitstream(struct File_Params file_info)
{
	int Status;
	volatile u32 IntrStsReg = 0;

	u32 WordLength = 0;
	WordLength = (file_info.size >> 2);  //Need to pass in the word size, not size in bytes


	//portDISABLE_INTERRUPTS();


	//Added STM 9July14 based on forums.xilinx.com/t5/Zynq-All-Programmable-Soc/Zynq-PCAP/td-p/288717
	//If things don't work with the devcfg interface, try the line below...
	//XDcfg_SetLockRegister(DcfgInstPtr, 0x757BDF0D);

//	Status = XDcfg_SelfTest(DcfgInstPtr);
//	if (Status != XST_SUCCESS) {
//		return Status;
//	}


	Xil_Out32(SLCR_UNLOCK, SLCR_UNLOCK_VAL);
	Xil_Out32(SLCR_LVL_SHFTR_EN, 0xA);
	//Xil_Out32(FPGA_RST_CTRL, 0xFFFFFFFF);
	Xil_Out32(SLCR_LOCK, SLCR_LOCK_VAL);

	IntrStsReg = XDcfg_IntrGetStatus(DcfgInstPtr);


	// Clear the interrupt status bits
	XDcfg_IntrClear(DcfgInstPtr, (XDCFG_IXR_PCFG_DONE_MASK | XDCFG_IXR_D_P_DONE_MASK | XDCFG_IXR_DMA_DONE_MASK));

	IntrStsReg = XDcfg_IntrGetStatus(DcfgInstPtr);

	// Download bitstream in non secure mode
	Status = XDcfg_Transfer(DcfgInstPtr, (u32 *)file_info.address, WordLength,
				   	   	   	(u32 *)XDCFG_DMA_INVALID_ADDRESS, 0, XDCFG_NON_SECURE_PCAP_WRITE);
	if (Status != XST_SUCCESS)
	{
		return Status;
	}

	IntrStsReg = XDcfg_IntrGetStatus(DcfgInstPtr);

	// Poll DMA Done Interrupt
	while ((IntrStsReg & XDCFG_IXR_DMA_DONE_MASK) != XDCFG_IXR_DMA_DONE_MASK)
	{
		IntrStsReg = XDcfg_IntrGetStatus(DcfgInstPtr);
	}


	// Poll PCAP Done Interrupt
	while ((IntrStsReg & XDCFG_IXR_D_P_DONE_MASK) != XDCFG_IXR_D_P_DONE_MASK)
	{
		IntrStsReg = XDcfg_IntrGetStatus(DcfgInstPtr);
	}


	Xil_Out32(SLCR_UNLOCK, SLCR_UNLOCK_VAL);
	Xil_Out32(SLCR_LVL_SHFTR_EN, 0xF);
	Xil_Out32(SLCR_LOCK, SLCR_LOCK_VAL);
	//portENABLE_INTERRUPTS();

	return XST_SUCCESS;
}*/


//Code was borrowed from devcfg polled example, per Xilinx's suggestion
#include "xparameters.h"
#include "xdevcfg.h"
#include "file_loader.h"
#include "freertos.h"
#include "task.h"
#include "xil_io.h"
#include "hpp.h"
#include "xgpiops.h"
#include "sleep.h"
//#include "pcap.h"


#define SLCR_LOCK	0xF8000004 /**< SLCR Write Protection Lock */
#define SLCR_UNLOCK	0xF8000008 /**< SLCR Write Protection Unlock */
#define SLCR_LVL_SHFTR_EN 0xF8000900 /**< SLCR Level Shifters Enable */

#define SLCR_LOCK_VAL	0x767B
#define SLCR_UNLOCK_VAL	0xDF0D

static XDcfg DcfgInstance;
static XDcfg *DcfgInstPtr;


s8 Load_Bitstream()
{
	u16 DeviceId = XPAR_PS7_DEV_CFG_0_DEVICE_ID;
	int Status;
	u32 IntrStsReg = 0;
	u32 StatusReg;
	u32 temp_rst = 0;
	u32 temp = 0;

	#define FSBL_DEBUG_INFO

	//Commented Out STM 8July14 because we do not want partial reconfiguration at this time
	//u32 PartialCfg = 0;

	XDcfg_Config *ConfigPtr;
	DcfgInstPtr = &DcfgInstance;


	temp = 1;
	gpio_bank2 |= temp;
	XGpioPs_Write(&xGpiops, GPIO_BANK_2, gpio_bank2);

	/*
	 * Initialize the Device Configuration Interface driver.
	 */
	ConfigPtr = XDcfg_LookupConfig(DeviceId);

	xil_printf("\n\rDevCfg lookup complete.");

	/*
	 * This is where the virtual address would be used, this example
	 * uses physical address.
	 */
	Status = XDcfg_CfgInitialize(DcfgInstPtr, ConfigPtr,
					ConfigPtr->BaseAddr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	xil_printf("\n\rDevCfg init complete.");

	//Added STM 9July14 based on forums.xilinx.com/t5/Zynq-All-Programmable-Soc/Zynq-PCAP/td-p/288717
	//If things don't work with the devcfg interface, try the line below...
	//XDcfg_SetLockRegister(DcfgInstPtr, 0x757BDF0D);


	xil_printf("\n\rDevCfg SetLockRegister complete.");


	Status = XDcfg_SelfTest(DcfgInstPtr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	xil_printf("\n\rDevCfg self test complete.");


	taskENTER_CRITICAL();

	xil_printf("\n\rEntering Critical Section\n\r");

	/*
	 * Barrier for synchronization
	 */
		__asm__(
			"dsb\n\t"
			"isb"
		);

	/*echo "down sequence"
                && mw 0xF8000008 0xDF0D
                && mw 0xF8000240 0xF                                Assert PL resets
                && mw 0xF8000900 0x0                                Disable level shifters
                && mw 0XF8000004 0x767B; */

	xil_printf("\n\rPowering Down....\n\r");
	Xil_Out32(0xF8000008, 0xDF0D);
	Xil_Out32(0xF8000240, 0xF);
	Xil_Out32(0xF8000900, 0x0);
	Xil_Out32(0XF8000004, 0x767B);

	xil_printf("\n\rDisabling level shifters.");

	/*
	 * Clear the interrupt status bits
	 */
	XDcfg_IntrClear(DcfgInstPtr, (XDCFG_IXR_PCFG_DONE_MASK |
					XDCFG_IXR_D_P_DONE_MASK |
					XDCFG_IXR_DMA_DONE_MASK));


	xil_printf("\n\rDevCfg interrupts cleared.");

	/* Check if DMA command queue is full */
	StatusReg = XDcfg_ReadReg(DcfgInstPtr->Config.BaseAddr,
				XDCFG_STATUS_OFFSET);
	if ((StatusReg & XDCFG_STATUS_DMA_CMD_Q_F_MASK) ==
			XDCFG_STATUS_DMA_CMD_Q_F_MASK) {
		return XST_FAILURE;
	}


	xil_printf("\n\rDevCfg ReadReg complete.  Transferring bitstream from %x for %d bytes.", file_info.address, file_info.size);

	//Initialize the fabric
	HPPFabricInit();
	
	/*
	 * Download bitstream in non secure mode
	 */
	XDcfg_Transfer(DcfgInstPtr, (u32 *)file_info.address,
			file_info.size,
			(u32 *)XDCFG_DMA_INVALID_ADDRESS,
			0, XDCFG_NON_SECURE_PCAP_WRITE);


	xil_printf("\n\rDevCfg transfer complete.");

	/* Poll IXR_DMA_DONE */
	IntrStsReg = XDcfg_IntrGetStatus(DcfgInstPtr);
	while ((IntrStsReg & XDCFG_IXR_DMA_DONE_MASK) !=
			XDCFG_IXR_DMA_DONE_MASK) {
		IntrStsReg = XDcfg_IntrGetStatus(DcfgInstPtr);
	}


	xil_printf("\n\rIXR_DMA_DONE asserted.");


	while ((IntrStsReg & XDCFG_IXR_PCFG_DONE_MASK) !=
				XDCFG_IXR_PCFG_DONE_MASK) {
			IntrStsReg = XDcfg_IntrGetStatus(DcfgInstPtr);
	}


	xil_printf("\n\rIXR_PCFG_DONE asserted.");
	
	/*
	 * Enable the level-shifters from PS to PL.
	 */
	Xil_Out32(SLCR_UNLOCK, SLCR_UNLOCK_VAL);
	Xil_Out32(SLCR_LVL_SHFTR_EN, 0xF);
	temp_rst = Xil_In32(FPGA_RST_CTRL);
	temp_rst = temp_rst & 0xFFFFFFF0;
	Xil_Out32(FPGA_RST_CTRL, temp_rst);
	Xil_Out32(SLCR_LOCK, SLCR_LOCK_VAL);

	xil_printf("\n\rLevel shifters enabled and reset de-asserted.");

	taskEXIT_CRITICAL();
	xil_printf("\n\rExiting Critical Section\n\r");

	return XST_SUCCESS;
}


void HPPFabricInit(void)
{
	u32 PcapReg;
	u32 PcapCtrlRegVal;
	u32 StatusReg;

	/*
	 * Set Level Shifters DT618760 - PS to PL enabling
	 */
	Xil_Out32(PS_LVL_SHFTR_EN, LVL_PS_PL);
	xil_printf("Level Shifter Value = 0x%x \r\n", Xil_In32(PS_LVL_SHFTR_EN));

	/*
	 * Get DEVCFG controller settings
	 */
	PcapReg = XDcfg_ReadReg(DcfgInstPtr->Config.BaseAddr,
				XDCFG_CTRL_OFFSET);

	/*
	 * Setting PCFG_PROG_B signal to high
	 */
	XDcfg_WriteReg(DcfgInstPtr->Config.BaseAddr, XDCFG_CTRL_OFFSET,
				(PcapReg | XDCFG_CTRL_PCFG_PROG_B_MASK));

	/*
	 * Check for AES source key
	 */
	PcapCtrlRegVal = XDcfg_GetControlRegister(DcfgInstPtr);
	if (PcapCtrlRegVal & PCAP_CTRL_PCFG_AES_FUSE_EFUSE_MASK) {
		/*
		 * 5msec delay
		 */
		usleep(5000); //KBE???
	}

	/*
	 * Setting PCFG_PROG_B signal to low
	 */
	XDcfg_WriteReg(DcfgInstPtr->Config.BaseAddr, XDCFG_CTRL_OFFSET,
				(PcapReg & ~XDCFG_CTRL_PCFG_PROG_B_MASK));

	/*
	 * Check for AES source key
	 */
	if (PcapCtrlRegVal & PCAP_CTRL_PCFG_AES_FUSE_EFUSE_MASK) {
		/*
		 * 5msec delay
		 */
		usleep(5000); //KBE???
	}

	/*
	 * Polling the PCAP_INIT status for Reset
	 */
	while(XDcfg_GetStatusRegister(DcfgInstPtr) &
				XDCFG_STATUS_PCFG_INIT_MASK);

	/*
	 * Setting PCFG_PROG_B signal to high
	 */
	XDcfg_WriteReg(DcfgInstPtr->Config.BaseAddr, XDCFG_CTRL_OFFSET,
			(PcapReg | XDCFG_CTRL_PCFG_PROG_B_MASK));

	/*
	 * Polling the PCAP_INIT status for Set
	 */
	while(!(XDcfg_GetStatusRegister(DcfgInstPtr) &
			XDCFG_STATUS_PCFG_INIT_MASK));

	/*
	 * Get Device configuration status
	 */
	StatusReg = XDcfg_GetStatusRegister(DcfgInstPtr);
	xil_printf("Devcfg Status register = 0x%x \r\n",StatusReg);

	xil_printf("PCAP:Fabric is Initialized done\r\n");
}

