#include <stdio.h>
#include <math.h>
#include "FIRFilter.h"

#define SAMPLES   100

void setCoeff(int32_t *coeff)
{
	int i;

	initFIR();

	for (i = 0; i < NUM_TAPS; i++) {
		coeff[i] = (i*1000) << (ALGO_BITS-15); // Samples is 1.15 format
		//coeff[i] = -8388608+i-NUM_TAPS;
		//coeff[i] = 1048000+i-NUM_TAPS;
	}

	FIRFilter(NULL,
			  NULL,
			  coeff,
			  0);
}

void setValue(int32_t *signal, int32_t value)
{
	int ch;

	for (ch = 0; ch < DATA_CHANNELS; ch++) {
		signal[ch] = value;
	}
}

int main ()
{
  FILE   *fp;
  int32_t samples[DATA_CHANNELS];
  int32_t output[DATA_CHANNELS];
  int32_t coefficients[NUM_TAPS];

  setCoeff(coefficients);

  fp=fopen("eq_impulse.dat","w");

  int i;
  for (i=0; i<SAMPLES; i++) {
	  if(i==0)
		  setValue(samples, 0x8000); // Impulse response with 1.15 format
	  else
		  setValue(samples, 0);
	  
	  FIRFilter(output, samples, NULL, 1);
	  
   	  printf("%i %d %d\n", i, samples[7], output[7]);
   	  fprintf(fp,"%03i %05d %05d\r\n", i, (int)samples[7], (int)output[7]);
  }
  fclose(fp);

  return 0;
}
