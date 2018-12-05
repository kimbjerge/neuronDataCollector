#ifndef HPP_IIC_H_
#define HPP_IIC_H_

#include "xparameters.h"
#include "xiicps.h"
#include "xscugic.h"
#include "xil_exception.h"
#include "xil_printf.h"


#define IIC_DEVICE_ID		XPAR_XIICPS_0_DEVICE_ID
#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
#define IIC_INT_VEC_ID		XPAR_XIICPS_0_INTR

#define IIC_SLAVE_ADDR		0x55
#define IIC_SCLK_RATE		100000

extern XIicPs Iic;

s8 vSetupHPPIIC(u16 DeviceId);
s8 SendHeadstageStimCommand(char *SendBuffer, char *RecvBuffer);


void Handler(void *CallBackRef, u32 Event);

#endif
