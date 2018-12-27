#include <stdio.h>
#include <math.h>
#include "FIRFilter.h"
#define FIR_TAPS NUM_TAPS
#include "FIRFilter_coeffs.h"

#define SAMPLES   	100  // Impulse response test
#define NUM_SAMPLES 30000 // Filter test on real neuron signals

float m_data[NUM_SAMPLES][DATA_CHANNELS*4];
int m_idx;

void setCoeff(int32_t *coeff)
{
	int i;

	initFIR();

	for (i = 0; i < NUM_TAPS; i++) {
		//coeff[i] = (i*1000) << (ALGO_BITS-15); // Samples is 1.15 format
		coeff[i] = (int32_t)round(FIR_coeffs[i] * pow(2, ALGO_BITS));
		//coeff[i] = -8388608+i-NUM_TAPS;
	}

	FIRFilter(NULL,
			  NULL,
			  coeff,
			  0);
}

void readDataSamples(void)
{
	FILE *fp;
	fp = fopen("DATA.bin", "r+b");
	size_t sz = fread(m_data, sizeof(float), NUM_SAMPLES*32, fp);
	if (sz != NUM_SAMPLES*32) {
		printf("Error reading DATA.bin file\r\n");
	}
	fclose(fp);
	m_idx = 0;
}

void getNextSample(int32_t *signal)
{
	int ch;
	for (ch = 0; ch < DATA_CHANNELS; ch++) {
		signal[ch] = m_data[m_idx][ch];
	}
	m_idx++;
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

#if 0
  /* Impulse response test */
  for (i=0; i<SAMPLES; i++) {
	  if(i==0)
		  setValue(samples, 0x8000); // Impulse response with 1.15 format
	  else
		  setValue(samples, 0);
#else
  /* Data signal test */
  readDataSamples();
  for (i=0; i<NUM_SAMPLES; i++) {

	  getNextSample(samples);

#endif

	  FIRFilter(output, samples, NULL, 1);
	  
   	  printf("%i %d %d\n", i, samples[0], output[0]);
   	  fprintf(fp,"%03i %05d %05d\r\n", i, (int)samples[0], (int)output[0]);
  }
  fclose(fp);

  return 0;
}
