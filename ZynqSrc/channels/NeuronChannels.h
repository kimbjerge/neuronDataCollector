/*
 * AudioChannel.h
 *
 *  Created on: 09/07/2016
 *      Author: kbe
 */

#ifndef NEURONCHANNELS_H_
#define NEURONCHANNELS_H_

//#include "DMA.h"
//#include "xadc_merge.h"

#define AUDIO_TEST_MODE 	1
#define BYTES_PER_SAMPLE    4

enum STATES {
	WAIT_FOR_BLOCK = 0,
	WAIT_DMA_FREE = 1,
	WAIT_DMA_FINISHED = 2
};

class NeuronChannels
{

public:
	NeuronChannels(void) {};
	int init(int id);
	int readCounter(void);
	void setSampleRate(int rate);
	int getSampleRate(void);
	void setTestMode(int on);
	int getTestMode(void);
	void enable(int on);
	int getEnable(void);
	void setStreamSizeSec(int sizeSec) { streamSizeSec_ = sizeSec; };
	int getStreamSizeSec(void) { return streamSizeSec_; };
	int getStreamSize(void) { return streamSizeSec_*BYTES_PER_SAMPLE*getSampleRate(); };
	int getId(void) { return id_; };
	int getBlockId(void) { return blockId_; };
	//u32 getDMAAddr(void) { return audioCDMAAddr_; };
	//void addDMACtrl(DMACtrl *pDMACtrl) { pDMACtrl_ = pDMACtrl; };
	//DMACtrl *getDMACtrl(void) { return pDMACtrl_; };
	void startTransfer(void);
	int transferAudioDMA(int *stream_buf);
	void stopTransfer(void);
	void setAverage(int avg);
	int getAverage(void);
	void enableChannel(int channel, bool enable);
	int getChannelsDisabled(void);
	void setDownSample(int skip);
	int getDownSample(void);

private:
	//XAdc_merge adcLtc2380_;
	//u32 audioCDMAAddr_;
	int id_;
	//DMACtrl *pDMACtrl_;
	int blockId_;
	int streamSizeSec_;
	int sampleRate_; // Sample rate before down sampling (downSample_>1)
	int avgSkip_; // Number of skipped samples in LTC2380 IP Core - average filter on (M-1)
	int downSkip_; // Number of skipped samples in MERGE IP Core - average filter off (M-1)
	int testMode_; // Test mode on (1) off(0)
	STATES state_;
};


#endif /* NEURONCHANNELS_H_ */
