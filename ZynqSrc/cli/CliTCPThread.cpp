/*
 * DataThread.cpp
 *
 *  Created on: 29. nov. 2018
 *      Author: au288681
 */
/******************************************************************************
* CliTCPThread.cpp
*
*  Created on: 27. nov. 2018
*      Author: Kim Bjerge
*
* Copyright (C) 2016 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
* echoTCP.c
*
*  Created on: 27. nov. 2018
*      Author: Kim Bjerge
*******************************************************************************/
#include <stdio.h>
#include <string.h>

#include "lwip/sockets.h"
#include "netif/xadapter.h"
#include "lwipopts.h"
#include "xil_printf.h"
#include "FreeRTOS.h"
#include "task.h"

#include "defsNet.h"
#include "CliTCPThread.h"

u16_t cli_port = CLI_TCP_PORT;

// Static pointer to command line interface
static CliCommand *m_pCliCommand = 0;
// Buffer that contains received command
static char recv_buf[RECV_BUF_SIZE];

/* thread spawned for each connection */
void CliTCPThread::process_echo_request(void *p)
{
	int sd = (int)p;
	int n, nwrote;

	while (1) {
		/* read a max of RECV_BUF_SIZE bytes from socket */
		if ((n = read(sd, recv_buf, RECV_BUF_SIZE)) < 0) {
			xil_printf("%s: error reading from socket %d, closing socket\r\n", __FUNCTION__, sd);
			break;
		}

		if (m_pCliCommand)
			n = m_pCliCommand->execute(recv_buf, recv_buf, n);

		/* break if client closed connection */
		if (n <= 0) break;

		/* handle response */
		if ((nwrote = write(sd, recv_buf, n)) < 0) {
			xil_printf("%s: ERROR responding to client request. received = %d, written = %d\r\n",
					__FUNCTION__, n, nwrote);
			xil_printf("Closing socket %d\r\n", sd);
			break;
		}
	}

	/* close connection */
	close(sd);
	vTaskDelete(NULL);
}

void CliTCPThread::addCommand(CliCommand *pCmd)
{
	m_pCliCommand = pCmd;
}

void CliTCPThread::print_app_header()
{
    xil_printf("%20s %6d %s\r\n", " Commandline (CLI) ",
    			cli_port,
               "$ telnet <board_ip> 7");
}

void CliTCPThread::run()
{
	int sock, new_sd;
	struct sockaddr_in address, remote;
	int size;

	if ((sock = lwip_socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return;

	address.sin_family = AF_INET;
	address.sin_port = htons(cli_port);
	address.sin_addr.s_addr = INADDR_ANY;

	if (lwip_bind(sock, (struct sockaddr *)&address, sizeof (address)) < 0)
		return;

	/* print all application headers */
	xil_printf("\r\n");
	xil_printf("%20s %6s %s\r\n", "Server", "Port", "Connect With..");
	xil_printf("%20s %6s %s\r\n", "--------------------", "------", "--------------------");

	print_app_header();

	lwip_listen(sock, 0);

	size = sizeof(remote);

	while (1) {
		if ((new_sd = lwip_accept(sock, (struct sockaddr *)&remote, (socklen_t *)&size)) > 0) {
			sys_thread_new("CliTCPThread", process_echo_request,
				(void*)new_sd,
				THREAD_STACKSIZE,
				TCPIP_THREAD_PRIO);
		}
	}
}


