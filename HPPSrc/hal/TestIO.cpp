/*
 * TestIO.cpp
 *
 *  Created on: 14. marts 2019
 *      Author: Kim Bjerge
 */

#include "TestIO.h"
#include "hpp.h"

void TestIO::setOn(IOTypes io, bool on)
{
	int bitmask = 1 << (int)io;

    if (on) {
    	// Set TTL port bit
    	ttlPortValues = ttlPortValues | bitmask;
    } else {
    	// Clear TTL port bit
    	bitmask = ~bitmask;
    	ttlPortValues = ttlPortValues & bitmask;
    }

	u8 status = SetDIOAllValuesNoAck(ttlPortValues & 0xFFFF0000); // clear port 0+1 bits locks
	if(status != 0)
	{
		xil_printf("Problem with SetDIOAllValues, Status = %d\r\n", status);
	}
}

