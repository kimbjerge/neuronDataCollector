/*
 * defsNet.h
 *
 *  Created on: 27. nov. 2018
 *      Author: Kim Bjerge
 */

#ifndef SRC_DEFS_H_
#define SRC_DEFS_H_

// Global include file for MicroZedAXIStreamEthernetUDP project

// Debug defines
#define READ_BACK_INDEX_ARRAY

// Define port to listen on
#define FF_UDP_PORT 		7
#define FF_TCP_PORT 		7

// TIMEOUT FOR DMA AND GMM WAIT
#define RESET_TIMEOUT_COUNTER	10000

// DEFINES
#define WAVE_SIZE_BYTES    512  // Number of samples in waveform
#define INDARR_SIZE_BYTES  1024 // Number of bytes required to hold 512 fixed point floats

//HARDWARE DEFINES
#define NUMCHANNELS 		2	// Number of parallel operations done on input stream (1 OR 2)
#define BW   				32	// Total number of bits in fixed point data type
#define IW    				24	// Number of bits left of decimal point in fixed point data type
#define BITDIV			 256.0 	// Divisor to shift fixed point to int and back to float

#define THREAD_STACKSIZE 1024

void demo_udp_thread(void *);
void echo_tcp_thread(void *);

void init_net_server(void (*net_thread)(void *));

#endif /* SRC_DEFS_H_ */
