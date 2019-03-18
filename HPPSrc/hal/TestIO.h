/*
 * TestIO.h
 *
 *  Created on: 29. dec. 2018
 *      Author: Kim Bjerge
 */

#ifndef SRC_HAL_TESTIO_H_
#define SRC_HAL_TESTIO_H_

class TestIO
{
public:

	enum IOTypes {
		JB1 = 16, // Port 2, bit 0
		JB2 = 17,
		JB3 = 18,
		JB4 = 19,
		JB5 = 20,
		JB6 = 21,
		JB7 = 22,
		JB8 = 23, // Port 2, bit 7
		JB9 = 25, // Port 3, bit 1
		JB10 = 24 // Port 3, bit 0
	};

	TestIO()
	{
		ttlPortValues = 0;
	};

	void setOn(IOTypes io, bool on);

	private:
	    // Actual TTL values of port outputs
	    // Bit 0-7   - port 0 - not working
		// Bit 8-15  - port 1 - not working
    	// Bit 16-23 - port 2 - working (JB1 - JB8)
	    // Bit 24-31 - port 3 - working (JB9 - JB10)
		int ttlPortValues;

};


#endif /* SRC_HAL_TESTIO_H_ */
