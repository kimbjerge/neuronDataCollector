#include <stdio.h>
#include <math.h>
#include "IIRFilter.h"
#define IIR_TAPS NUM_TAPS
#include "IIRFilter_coeffs.h"

#define SAMPLES   	100  // Impulse response test
#define NUM_SAMPLES 30000 // Filter test on real neuron signals

float m_data[NUM_SAMPLES][32];
int m_idx;

void setCoeff(int32_t *coeff)
{
	int i, ch;

	initIIR();

	for (i = 0; i < NUM_TAPS*2*NUM_SOS; i++) {
		//coeff[i] = (i*1000) << (ALGO_BITS-15); // Samples is 1.15 format
		coeff[i] = (int32_t)round(IIR_coeffs[i] * pow(2, ALGO_BITS));
		//coeff[i] = -8388608+i-NUM_TAPS;
	}

	for (ch = 0; ch < DATA_CHANNELS; ch++)
		IIRFilter(NULL, NULL, coeff, ch);
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

void getNextSample(sigType *signal)
{
	int ch;
	for (ch = 0; ch < DATA_CHANNELS; ch++) {
		signal[ch] = m_data[m_idx][ch];
	}
	m_idx++;
}

void setValue(sigType *signal, int32_t value)
{
	int ch;

	for (ch = 0; ch < DATA_CHANNELS; ch++) {
		signal[ch] = value;
	}
}

int main ()
{
  FILE   *fp;
  sigType samples[DATA_CHANNELS];
  sigType output[DATA_CHANNELS];
  int32_t coefficients[NUM_TAPS*2*NUM_SOS];

  setCoeff(coefficients);

  fp=fopen("eq_impulse.dat","w");

  int i;

#if 0
  /* Impulse response test */
  for (i=0; i<SAMPLES; i++) {
	  if(i==0)
		  setValue(samples, 0x7FFF); // Impulse response with 1.15 format
	  else
		  setValue(samples, 0);
#else
  /* Data signal test */
  readDataSamples();
  for (i=0; i<NUM_SAMPLES; i++) {

	  getNextSample(samples);

#endif

	  IIRFilter(output, samples, NULL, DATA_CHANNELS);
	  
	  for (int ch = 0; ch < DATA_CHANNELS; ch++)
		  printf("%i %d %d\n", i, samples[ch], output[ch]);
   	  fprintf(fp,"%03i %05d %05d\r\n", i, samples[0], output[0]);
  }
  fclose(fp);

  return 0;
}
