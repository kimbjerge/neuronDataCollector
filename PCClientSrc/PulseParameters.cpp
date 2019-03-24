#include "PulseParameters.h"


PulseParameters::PulseParameters(void)
{
	idx_ = 0;
}


PulseParameters::~PulseParameters(void)
{
}

short *PulseParameters::getBlock(int &size) 
{ 
	size = PARAM_BLOCK_SIZE; 
	return &Blocks_[idx_]; 
}

short *PulseParameters::getBlocks(int &size) 
{ 
	size = (idx_ + PARAM_BLOCK_SIZE); 
	return Blocks_; 
}

int PulseParameters::incBlock(void) 
{ 
	if (idx_ < PARAM_BLOCK_SIZE*(MAX_BLOCKS-1)) 
	{ 
		idx_ = idx_ + PARAM_BLOCK_SIZE; 
		return 0;
	}
	else
		return 1; // Block buffer is full
}

int  PulseParameters::getNumBlocks(void)
{
	return idx_/PARAM_BLOCK_SIZE;
}

void PulseParameters::clearBlocks(void) 
{ 
	idx_ = 0; 
}




