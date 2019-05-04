#include "IIRFilter.h"
#include <systemc.h>

#define NUM_COEF     (NUM_SOS*NUM_TAPS)

static int32_t bn[DATA_CHANNELS*NUM_COEF];
static int32_t an[DATA_CHANNELS*NUM_COEF];
static int32_t xin[DATA_CHANNELS*NUM_COEF];
static int32_t yout[DATA_CHANNELS*NUM_COEF];

// Clear delay line
void initIIR(void) {

	for (int8_t ch = 0; ch < DATA_CHANNELS; ch++)
		for (int8_t i = 0; i < NUM_COEF;  i++) {
			xin[i + NUM_COEF*ch] = INIT_VALUE;
			yout[i + NUM_COEF*ch] = INIT_VALUE;
		}
}

/**
* @brief Processing IIR filtering for a specific channel and second order section
*
* @param int32_t results :          IIR output result
* @param int32_t sample : 			Input sample
* @param int8_t  sos :              Second Order IIR filter index
* @param int8_t  channel :          Channel number to perform filtering
*
* @retval int32_t : IIR filtered sample
*/
int32_t IIR(int32_t sample, int8_t sos, int8_t channel)
{
	//sc_int<48> ysum;
	int64_t ysum;
	int32_t y;
	int32_t x;
	int32_t b;
	int32_t a;
	int32_t result;
	int8_t i;

	// Index to IIR filter in SOS and channel
	int16_t idx = NUM_TAPS*sos + NUM_COEF*channel;

	// Shift delay line
	IIR_label1:for (i = NUM_TAPS-1; i > 0; i--) {
		xin[i + idx] = xin[i-1 + idx];
		yout[i + idx] = yout[i-1 + idx];
	}

	// Insert new sample
	xin[idx] = sample;
	// Insert zero for a0 coefficient
	yout[idx] = 0;

	// Perform filtering
	ysum = 0;
	IIR_label2:for (i = 0; i < NUM_TAPS; i++) {
		x = xin[i + idx];
		b = bn[i + idx];
		ysum += b*x;
		y = yout[i + idx];
		a = an[i + idx];
		ysum -= a*y;
	}

	// Scale filter result and return
	result = (ysum + (1<<(ALGO_BITS-1))) >> ALGO_BITS; // Round
	//result = ysum >> ALGO_BITS; // Truncate

	// Insert new result
	yout[idx] = result;

	//printf("%d, %d\r\n", channel, result);
	return result;
}

/**
* @brief Processing IIR filtering for number DATA_CHANNELS
*
* @param int32_t results :          Pointer to the IIR output is stored
* @param int32_t samples : 			Pointer to the input sample array - 1D
* @param int32_t coeff :            Pointer to the input coefficients array - 1D
* @param int32_t operation :        Operation = 0-7 update coefficients, Operation > 7 filter
*
* @retval void : none
*/
void IIRFilter (sigType results[DATA_CHANNELS],
				sigType samples[DATA_CHANNELS],
				int32_t coeff[NUM_TAPS*2*NUM_SOS],
				int32_t operation)
{

    if (operation < DATA_CHANNELS)
    {  // Update b and a coefficients
		for (int8_t j = 0; j < NUM_SOS; j++){
			for (int8_t i = 0; i < NUM_TAPS; i++) {
    			bn[i + j*NUM_TAPS + NUM_COEF*operation] = coeff[i + j*(NUM_TAPS*2)];// First b coefficients (forward xn)
    			an[i + j*NUM_TAPS + NUM_COEF*operation] = coeff[i + j*(NUM_TAPS*2) + NUM_TAPS]; // Second a coefficients (backward yn)
    		}
    	}
    }
    else
    {  // Processing SOS IIR filters
    	int32_t inSample, outSample;
		for (int8_t ch = 0; ch < DATA_CHANNELS; ch++) {
			inSample = samples[ch];
			for (int8_t sos = 0; sos < NUM_SOS; sos++) {
				outSample = IIR(inSample, sos, ch);
				inSample = outSample;
			}
			results[ch] = outSample;
		}
    }
}
