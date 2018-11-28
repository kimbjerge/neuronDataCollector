/******************************************************************************
* echoUDP.c
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

void print_ip(char *msg, struct ip_addr *ip);

// Global variables for data flow
volatile u32		EthBytesReceived;
volatile u8			SendResults;
volatile u8			Error;

// Global Variables for Ethernet handling
u16_t    			RemotePort = FF_UDP_PORT;
struct ip_addr  	RemoteAddr;
struct udp_pcb 		send_pcb;

// Global Variables to store results and handle data flow
char                HelloStr[256];

int transfer_data() {
	return 0;
}

/* print_app_header: function to print a header at start time */
void print_app_header()
{
	xil_printf("\n\r\n\r------lwIP UDP Demo Application------\n\r");
	xil_printf("UDP packets sent to port 7 will be processed\n\r");
}

/* recv_callback: function that handles responding to UDP packets */
void recv_callback(void *arg, struct udp_pcb *upcb,
                              struct pbuf *p, struct ip_addr *addr, u16_t port)
{
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
	EthBytesReceived = p->len;
	xil_printf("L%d\r\n", p->len );
	//memcpy(&WaveformArr[0], (u32*)p->payload, EthBytesReceived);
	SendResults = 1;

	/* free the received pbuf */
	pbuf_free(p);
	return;

}

/* start_application: function to set up UDP listener */
static int create_bind_socket()
{
	struct udp_pcb *pcb;
	err_t err;
	unsigned port = FF_UDP_PORT;

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

	xil_printf("UDP echo server started @ port %d\n\r", port);

	return 0;
}

void demo_udp_thread(void *)
{
	struct pbuf * psnd;
	err_t udpsenderr;
	int status = 0;
	int counter = 0;

	memset(HelloStr, 0, sizeof(HelloStr));
	strcpy(HelloStr, "UDP packet from Zynq Board\n\r");

    print_app_header();

    IP4_ADDR(&RemoteAddr,  192, 168, 1, 20);
	xil_printf("Remote IP settings: \r\n");
	//print_ip_settings(&RemoteAddr, &Remotenetmask, &Remotegw);
	print_ip((char*)"Board IP: ", &RemoteAddr);

	/* start the application (web server, rxtest, txtest, etc..) */
	status = create_bind_socket();
	if (status != 0){
		xil_printf("Error in creating and binding UDP receive socket with code: %d\n\r", status);
		goto ErrorOrDone;
	}

    SendResults = 1;

	/* receive and process packets */
	while (Error==0) {

		/* Send results back from time to time */
		if (SendResults == 1) {
			//SendResults = 0;

			// Send out the centroid result over UDP
			psnd = pbuf_alloc(PBUF_TRANSPORT, sizeof(HelloStr), PBUF_REF);
			psnd->payload = HelloStr;
			udpsenderr = udp_sendto(&send_pcb, psnd, &RemoteAddr, RemotePort);
			xil_printf("%d\r", counter++);
			if (udpsenderr != ERR_OK){
				xil_printf("UDP Send failed with Error %d\n\r", udpsenderr);
				SendResults = 0;
				goto ErrorOrDone;
			}
			pbuf_free(psnd);
		}
		vTaskDelay( pdMS_TO_TICKS( 0.0333333 ) );
		//vTaskDelay( pdMS_TO_TICKS( 100 ) );
	}

	// Jump point for failure
ErrorOrDone:
	xil_printf("Catastrophic Error! Shutting down and exiting...\n\r");
    vTaskDelete(NULL);
}

