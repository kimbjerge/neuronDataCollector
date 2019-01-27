/*
 * IRQ.cpp
 *
 *  Created on: 16. aug. 2017
 *      Author: Kim Bjerge
 */

#include "IRQ.h"


int IRQ::Init(void)
{
	   //This functions sets up the interrupt on the ARM
	   int result;
	   XScuGic_Config *pCfg = XScuGic_LookupConfig(mDeviceId);
	   if (pCfg == NULL){
	      print("Interrupt Configuration Lookup Failed\n\r");
	      return XST_FAILURE;
	   }
	   result = XScuGic_CfgInitialize(&mScuGic,pCfg,pCfg->CpuBaseAddress);
	   if(result != XST_SUCCESS){
	      return result;
	   }
	   // self test
	   result = XScuGic_SelfTest(&mScuGic);
	   if(result != XST_SUCCESS){
	      return result;
	   }
	   // Initialize the exception handler
	   Xil_ExceptionInit();
	   // Register the exception handler
	   Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,(Xil_ExceptionHandler)XScuGic_InterruptHandler,&mScuGic);
	   //Enable the exception handler
	   Xil_ExceptionEnable();

	   return XST_SUCCESS;
}
