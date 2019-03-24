// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>

#define PULSE_SIZE			  8192					// Max block size is 64 kBYtes each 64 bits
#define BRAM_BYTE_SIZE	 	  32768  				// Size BRAM in pulse detector
//#define BRAM_BYTE_SIZE	 	  65536  				// Size BRAM in pulse detector
#define PARAM_BLOCK_SIZE      (BRAM_BYTE_SIZE/4)	// Size of parameter block received from RTL pulse detector 
#define MAX_BLOCKS            10                     // (10) Maxinumber of blocks received before saving in file
#define NUM_CHANNELS          12					// Number of pulse detector channels

// TODO: reference additional headers your program requires here
