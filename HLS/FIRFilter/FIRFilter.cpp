#include "FIRFilter.h"
#include <systemc.h>

static int32_t bn[DATA_CHANNELS*NUM_TAPS];
static sigType xn[DATA_CHANNELS*NUM_TAPS];

// Clear delay line
void initFIR(void) {

	for (int16_t ch = 0; ch < DATA_CHANNELS; ch++)
		for (int16_t i = 0; i < NUM_TAPS;  i++)
			xn[i + NUM_TAPS*ch] = INIT_VALUE;
}

/**
* @brief Processing FIR filtering for a specific channel
*
* @param int32_t results :          Pointer to the FIR output is stored
* @param int32_t sample : 			Input sample
* @param int32_t channel :          Channel number to perform filtering
*
* @retval int32_t : FIR filtered sample
*/
sigType fir(sigType sample, int16_t channel)
{
	sc_int<48> yn;
	sc_int<48> x;
	sc_int<48> b;
	sigType result;
	int16_t i;

	// Shift delay line
	fir_label1:for (i = NUM_TAPS-1; i > 0; i--)
		xn[i + NUM_TAPS*channel] = xn[i-1 + NUM_TAPS*channel];

	// Insert new sample
	xn[NUM_TAPS*channel] = sample;

	// Perform filtering
	yn = 0;
	fir_label2:for (i = 0; i < NUM_TAPS; i++) {
		x = xn[i + NUM_TAPS*channel];
		b = bn[i + NUM_TAPS*channel];
		yn += b*x;
	}

	// Scale filter result and return
	result = (yn+0x400000) >> ALGO_BITS; // Round
	//printf("%d, %d\r\n", channel, result);
	return result;
}

/**
* @brief Processing FIR filtering for number DATA_CHANNELS
*
* @param int32_t results :          Pointer to the FIR output is stored
* @param int32_t samples : 			Pointer to the input sample array - 1D
* @param int32_t coeff :            Pointer to the input coefficients array - 1D
* @param int32_t operation :        Operation = 0-7 update coefficients, Operation > 7 filter
*
* @retval void : none
*/
void FIRFilter (sigType results[DATA_CHANNELS],
				sigType samples[DATA_CHANNELS],
				int32_t coeff[NUM_TAPS],
				int32_t operation)
{

    if (operation < DATA_CHANNELS)
    {  // Update coefficients
    	for (int16_t i = 0; i < NUM_TAPS; i++)
    		bn[i + NUM_TAPS*operation] = coeff[i];
    }
    else
    {  // Processing EQ filter
		FIRFilter_label0:for (int16_t ch = 0; ch < DATA_CHANNELS; ch++) {
			results[ch] = fir(samples[ch], ch);
		}
    }
}
