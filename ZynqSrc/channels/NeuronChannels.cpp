#include <stdio.h>
#include "NeuronChannels.h"

// Design for 4 channels
#define AUDIO_CDMA_BRAM_MEMORY_CH1  0xC2000000 // Pulse BRAM Port A mapped through BRAM Controller 1 accessed by CDMA

int NeuronChannels::init(int id)
{
  id_ = id;
  /*
  switch (id_) {
	  case 0:
	  case 1:
		  audioCDMAAddr_ = AUDIO_CDMA_BRAM_MEMORY_CH1;
		  break;
	  default:
		  return XST_FAILURE;
		  break;
  }

  if (XAdc_merge_Initialize(&adcLtc2380_, XPAR_XADC_MERGE_0_DEVICE_ID) != XST_SUCCESS)
	  return XST_FAILURE;
  */

  testMode_ = getTestMode();
  avgSkip_ = getAverage();
  downSkip_ = getDownSample();
  streamSizeSec_ = 10; // Default stream size is 10 seconds
  //return XST_SUCCESS;
  return 0;
}

// Read the counter value used to detect pulses
int NeuronChannels::readCounter(void)
{
	//return XAdc_merge_GetWrcount(&adcLtc2380_);
	return 0;
}

void NeuronChannels::setSampleRate(int rate)
{
	unsigned short clockDelay;

	sampleRate_ = rate;
	clockDelay = ((float)50000000/rate + 0.5); // Counts 100 Mhz/2
	printf("Clock delay %d\n", clockDelay);

	//XAdc_merge_SetTestclocks(&adcLtc2380_, clockDelay);
}

int NeuronChannels::getSampleRate(void)
{
	if (testMode_ == 1)
		return sampleRate_;
	else
		return (sampleRate_/(avgSkip_+1))/(downSkip_+1);
}

void NeuronChannels::setTestMode(int on)
{
	testMode_ = on;
	//XAdc_merge_SetTestmode(&adcLtc2380_, on);
}

int NeuronChannels::getTestMode(void)
{
	//return XAdc_merge_GetTestmode(&adcLtc2380_);
}

void NeuronChannels::enable(int on)
{
	//XAdc_merge_SetStart(&adcLtc2380_, on);
}

int NeuronChannels::getEnable(void)
{
	//return XAdc_merge_GetStart(&adcLtc2380_);
}

void NeuronChannels::setAverage(int avg)
{
	avgSkip_ = avg;
	//XAdc_merge_SetAverage(&adcLtc2380_, avg);
}

int NeuronChannels::getAverage(void)
{
	//return XAdc_merge_GetAverage(&adcLtc2380_);
	return 0;
}

void NeuronChannels::setDownSample(int skip)
{
	downSkip_ = skip;
	//XAdc_merge_SetDownsample(&adcLtc2380_, skip);
}

int NeuronChannels::getDownSample(void)
{
	//return XAdc_merge_GetDownsample(&adcLtc2380_);
	return 0;
}

void NeuronChannels::enableChannel(int channel, bool enable)
{
	/*
	int disabled = XAdc_merge_GetChanneldisable(&adcLtc2380_);
	if (enable)
		disabled = disabled & (~(1 << channel));
	else
		disabled = disabled | (1 << channel);
	XAdc_merge_SetChanneldisable(&adcLtc2380_, disabled);
	*/
}

int NeuronChannels::getChannelsDisabled(void)
{
	//return XAdc_merge_GetChanneldisable(&adcLtc2380_);
	return 0;
}

void NeuronChannels::startTransfer(void)
{
	state_ = WAIT_FOR_BLOCK;
	blockId_ = 0;
	//printf("SAUDIO%d\n", AUDIO_TEST_MODE);
	//SetTestMode(AUDIO_TEST_MODE);
	enable(1);
}

void NeuronChannels::stopTransfer(void)
{
	enable(0);
	//SetTestMode(0);
}

int NeuronChannels::transferAudioDMA(int *stream_buf)
{
	int completed = 0;
	/*
	int blockPos, numofbytes;
	u8 * cdma_memory_source, * cdma_memory_destination;
	//int dmaId = pDMACtrl_->getId();


	switch (state_) {

	   case WAIT_FOR_BLOCK:
			blockPos = readCounter();
        	//printf("POS%d\n", blockPos);
        	if (blockId_%2 == 1) { // Every odd blockId transfer from upper half of BRAM
				if (blockPos < BLOCK_SIZE/4) { // Check BRAM is full
					state_ = WAIT_DMA_FREE;
				}
        	} else {
				if (blockPos > BLOCK_SIZE/4) { // Check BRAM is half full
					state_ = WAIT_DMA_FREE;
				}
        	}
			break;

		case WAIT_DMA_FREE:
			//printf("DMAS%d.%d\n", id, blockId);
			cdma_memory_source = (u8 *)getDMAAddr();
			cdma_memory_destination = (u8 *)stream_buf; //BlockMemory(id, blockId);
			numofbytes = BLOCK_SIZE; // Lower half of BRAM to transfer
			if (blockId_%2 == 1) { // Every odd blockId transfer from upper half of BRAM
				cdma_memory_source += numofbytes; // Upper half of BRAM to transfer
			}
			pDMACtrl_->startDMATransfer(cdma_memory_source, cdma_memory_destination, numofbytes);
			state_ = WAIT_DMA_FINISHED;
			break;

		case WAIT_DMA_FINISHED:
			if (pDMACtrl_->checkDMAComplete()) {
            	//printf("DMAE%d.%d\n", dmaId, blockId_);
				//if (blockId_ < MEMORY_BLOCKS-1)
				blockId_ = blockId_ + 1;
				//else blockId_ = 0;
				completed = 1;
				state_ = WAIT_FOR_BLOCK;
			}
			break;

	}
	*/

	return completed;
}

