#include "FIRFilter.h"

static int32_t bn[NUM_TAPS];
static int32_t xn[DATA_CHANNELS*NUM_TAPS];

// Clear delay line
void initFIR(void) {

	for (int16_t ch = 0; ch < DATA_CHANNELS; ch++)
		for (int16_t i = 0; i < NUM_TAPS;  i++)
			xn[i + DATA_CHANNELS*ch] = INIT_VALUE;
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
int32_t fir(int32_t sample, int16_t channel)
{
	int32_t yn;
	int16_t i;

	// Shift delay line
	fir_label1:for (i = NUM_TAPS-1; i > 0; i--)
		xn[i + DATA_CHANNELS*channel] = xn[i-1 + DATA_CHANNELS*channel];

	// Insert new sample
	xn[DATA_CHANNELS*channel] = sample;

	// Perform filtering
	yn = 0;
	fir_label2:for (i = 0; i < NUM_TAPS; i++)
		yn += bn[i]*xn[i + DATA_CHANNELS*channel];

	// Scale filter result and return
	yn >>= ALGO_BITS;
	return yn;
}

/**
* @brief Processing FIR filtering for number DATA_CHANNELS
*
* @param int32_t results :          Pointer to the FIR output is stored
* @param int32_t samples : 			Pointer to the input sample array - 1D
* @param int32_t coeff :            Pointer to the input coefficients array - 1D
* @param int32_t operation :        Operation = 0 update coefficients, Operation > 0 filter
*
* @retval void : none
*/
void FIRFilter (int32_t results[DATA_CHANNELS],
				int32_t samples[DATA_CHANNELS],
				int32_t coeff[NUM_TAPS],
				int32_t operation)
{

    if (operation == 0)
    {  // Update coefficients
    	for (int16_t i = 0; i < NUM_TAPS; i++)
    		bn[i] = coeff[i];
    }
    else
    {  // Processing EQ filter
		FIRFilter_label0:for (int16_t ch = 0; ch < DATA_CHANNELS; ch++) {
			results[ch] = fir(samples[ch], ch);
		}
    }
}
