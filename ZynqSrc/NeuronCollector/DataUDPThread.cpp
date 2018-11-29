/*
 * DataThread.cpp
 *
 *  Created on: 29. nov. 2018
 *      Author: au288681
 */
/******************************************************************************
* DataUDPThread.cpp
*
*  Created on: 27. nov. 2018
*      Author: Kim Bjerge
*******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "netif/xadapter.h"
#include "lwip/err.h"
#include "lwip/udp.h"
#include "xil_printf.h"

// Include our own definitions
#include <defsNet.h>

#include "DataUDPThread.h"

void print_ip(char *msg, struct ip_addr *ip);

/* print_app_header: function to print a header at start time */
void DataUDPThread::print_app_header()
{
	xil_printf("\n\r\n\r------ lwIP UDP Neuron Data Transmitter  ------\n\r");
	xil_printf("UDP packets send to port %d will be transmitted\n\r", DATA_UDP_PORT);
}

// Global variable to synchronize recv_callback
static volatile short SendResults = 0;

/* recv_callback: function that handles responding to UDP packets */
static void recv_callback(void *arg, struct udp_pcb *upcb,
                               struct pbuf *p, struct ip_addr *addr, u16_t port)
{
	u16_t    			RemotePort;
	struct ip_addr  	RemoteAddr;
	struct udp_pcb 		send_pcb;

	/* Do not read the packet if we are not in ESTABLISHED state */
	if (!p) {
		udp_disconnect(upcb);
		return;
	}

	/* Assign the Remote IP:port from the callback on each first pulse */
	RemotePort = port;
	RemoteAddr = *addr;

	/* Keep track of the control block so we can send data back in the main while loop */
	send_pcb = *upcb;

	/********************** WAVE ARRAY ********************************/
	// Determine the number of bytes received and copy this segment to the temp array
	xil_printf("L%d\r\n", p->len );
	SendResults = 1;

	/* free the received pbuf */
	pbuf_free(p);
	return;

}

/* start_application: function to set up UDP listener */
int  DataUDPThread::create_bind_socket(unsigned port)
{
	struct udp_pcb *pcb;
	err_t err;

	/* create new UDP PCB structure */
	pcb = udp_new();
	if (!pcb) {
		xil_printf("Error creating PCB. Out of Memory\n\r");
		return -1;
	}

	/* bind to specified @port */
	err = udp_bind(pcb, IP_ADDR_ANY, port);
	if (err != ERR_OK) {
		xil_printf("Unable to bind to port %d: err = %d\n\r", port, err);
		return -2;
	}

	/* specify callback to use for incoming connections */
	udp_recv(pcb, recv_callback, NULL);

	xil_printf("UDP server started @ port %d\n\r", port);

	return 0;
}

// Global Variables for Ethernet handling
static u16_t    			RemotePort = DATA_UDP_PORT;
static struct ip_addr  		RemoteAddr;
static struct udp_pcb 		send_pcb;

void DataUDPThread::run()
{
	struct pbuf * psnd;
	err_t udpsenderr;
	int status = 0;
	u8			Error;
	bool ledOn = false;

	memset(HelloStr, 0, sizeof(HelloStr));
	strcpy(HelloStr, "UDP packet from Zynq Board\n\r");

    print_app_header();

    REMOTE_IP_CFG;
	printf("Remote IP settings: \r\n");
	print_ip((char*)"Board IP: ", &RemoteAddr);

	/* start the application server */
	status = create_bind_socket(RemotePort);
	if (status != 0){
		xil_printf("Error in creating and binding UDP receive socket with code: %d\n\r", status);
		goto ErrorOrDone;
	}

    SendResults = 1;
    Error = 0;
    counter = 0;

	/* transmit packets */
	while (Error==0) {

		if (sw.isOn(Switch::SW0)) {
			//SendResults = 0;

			// Send out the lxRecord over UDP
			dataGenerator.GenerateSampleRecord(&lxRecord);
			psnd = pbuf_alloc(PBUF_TRANSPORT, sizeof(lxRecord), PBUF_REF);
			psnd->payload = &lxRecord;
			udpsenderr = udp_sendto(&send_pcb, psnd, &RemoteAddr, RemotePort);
			//printf("%d\r", counter++);
			ledOn = !ledOn;
			leds.setOn(Leds::LED0, ledOn);
			counter++;
			if (udpsenderr != ERR_OK){
				xil_printf("UDP Send failed with Error %d\n\r", udpsenderr);
				SendResults = 0;
				goto ErrorOrDone;
			}
			pbuf_free(psnd);
		} else
		{
			if (ledOn) {
				ledOn = false;
				leds.setOn(Leds::LED0, ledOn);
			}
		}
		//vTaskDelay( pdMS_TO_TICKS( 0.0333333 ) );
		vTaskDelay( pdMS_TO_TICKS( 1 ) );
	}

	// Jump point for failure
ErrorOrDone:
	leds.setOn(Leds::LED0, false);
	xil_printf("Catastrophic Error! Shutting down and exiting...\n\r");
	while (1) {
		// Blinking with led
		ledOn = !ledOn;
		leds.setOn(Leds::LED0, ledOn);
		vTaskDelay( pdMS_TO_TICKS( 200 ) );
	}
    vTaskDelete(NULL);
}



