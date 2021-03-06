/*
 * DataUDPThread.cpp
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

void print_ip(char *msg, ip_addr_t *ip);

/* print_app_header: function to print a header at start time */
void DataUDPThread::print_app_header()
{
	xil_printf("\n\r\n\r------ lwIP UDP Neuron Data Transmitter  ------\n\r");
	xil_printf("UDP packets send to port %d will be transmitted\n\r", DATA_UDP_PORT);
}

// Global variable to synchronize recv_callback
static volatile short SendResults = 0;

/* recv_callback: function that handles responding to UDP packets */
static void recv_callback(void *arg, struct udp_pcb *upcb, struct pbuf *p,
#ifdef RTOS10 // Freertos10
						  const
#endif
					      ip_addr_t *addr, u16_t port)
{
	u16_t    			RemotePort;
	ip_addr_t  			RemoteAddr;
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

void DataUDPThread::PrintRecords(int cnt)
{
	for (int i = 0; i < cnt; i++)
		xil_printf("LX[%02d]%8u,%11u,%7d,%7d,%7d,%7d\r\n",
				i,
				lxRecord[i].header.packetId,
				lxRecord[i].header.timestampLow,
				lxRecord[i].board[0].data[0],
				lxRecord[i].board[0].data[9],
				lxRecord[i].board[0].data[19],
				lxRecord[i].board[0].data[31]
				);
}

// Global Variables for Ethernet handling
static u16_t    			RemotePort = DATA_UDP_PORT;
static ip_addr_t  			RemoteAddr;
static struct udp_pcb 		send_pcb;

void DataUDPThread::run()
{
	struct pbuf * psnd;
	err_t udpsenderr;
	int status = 0;
	u8			Error;
    int start_tick, end_tick;
	bool ledOn = true;

    print_app_header();

    REMOTE_IP_CFG;
	printf("Remote IP settings: \r\n");
	print_ip((char*)"Board IP: ", &RemoteAddr);

	/* start the receiving application server */
	//status = create_bind_socket(RemotePort); // Do not receive UDP data yet!
	if (status != 0){
		printf("Error in creating and binding UDP receive socket with code: %d\n\r", status);
		goto ErrorOrDone;
	}

    Error = 0;
    counter = 0;
    running = true;
	leds.setOn(Leds::LED0, ledOn);
	start_tick = xTaskGetTickCount();

	/* transmit packets */
	while (Error==0 && running) {

		if (sw.isOn(Switch::SW0)) {

			//printf("%d\r", counter++);
			//ledOn = !ledOn;
			if (ledOn == false) {
				ledOn = true;
				leds.setOn(Leds::LED0, ledOn);
			}

			// Send out the lxRecord over UDP
			for (int i = 0; i < NUM_TX_RECORDS; i++)
				pNeuronData->GenerateSampleRecord(&lxRecord[i]);

			psnd = pbuf_alloc(PBUF_TRANSPORT, sizeof(lxRecord), PBUF_REF);
			psnd->payload = lxRecord;
			udpsenderr = udp_sendto(&send_pcb, psnd, &RemoteAddr, RemotePort);
			pbuf_free(psnd);
			if (udpsenderr != ERR_OK){
				printf("UDP Send failed with Error %d\n\r", udpsenderr);
				Error = 1;
			}
			counter++;

			// For debugging
			if (counter%100 == 0) // Print every ~1 seconds
				PrintRecords(10);

		} else {
			ledOn = !ledOn;
			leds.setOn(Leds::LED0, ledOn);
			vTaskDelay( pdMS_TO_TICKS( 200 ) );
		}

#ifdef ZEDBOARD_DEBUG
		//vTaskDelay( pdMS_TO_TICKS( 0.0333333 ) ); // Sample rate 30 kHz Ts=0.0333 ms
		vTaskDelay( pdMS_TO_TICKS( 10 ) ); // Max. rate for PC to receive data Ts=10 ms
#endif

	}

	// Jump point for failure
ErrorOrDone:
	end_tick = xTaskGetTickCount();
	printf("Tick start %d and tick end %d, duration = %d ms\r\n", start_tick, end_tick, (1000*(end_tick-start_tick))/configTICK_RATE_HZ);

	while (Error != 0 && running) {
		// Blinking with led when error
		ledOn = !ledOn;
		leds.setOn(Leds::LED0, ledOn);
		vTaskDelay( pdMS_TO_TICKS( 200 ) );
		printf(".");
	}

	leds.setOn(Leds::LED0, false);
	printf("UDP data thread shutting down and exit\n\r");
    vTaskDelete(NULL);
}



