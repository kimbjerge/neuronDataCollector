#pragma once
#include "stdafx.h"

class PulseParameters
{
public:
	PulseParameters(void);
	~PulseParameters(void);
	short *getBlock(int &size);
	int incBlock(void);
	int getNumBlocks(void);
	int getBlockSize(void) { return PARAM_BLOCK_SIZE; };
	void clearBlocks(void);
	short *getBlocks(int &size);
private:
	short Blocks_[PARAM_BLOCK_SIZE*MAX_BLOCKS];
	int idx_;
};

