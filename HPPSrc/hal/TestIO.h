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
		JB1 = 16, // Port 2, bit 0 - Template 1
		JB2 = 17, // Port 2, bit 1 - Template 2
		JB3 = 18, // Port 2, bit 2 - Template 3
		JB4 = 19, // Port 2, bit 3 - Template 4
		JB5 = 20, // Port 2, bit 4 - Template 5
		JB6 = 21, // Port 2, bit 5 - Template 6
		JB7 = 22, // Port 2, bit 6 - NOT USED
		JB8 = 25, // Port 3, bit 1 - NOT WORKING???
		JB9 = 23, // Port 2, bit 7 - Toggle when template 1 and 2 seen within sample window
		JB10 = 24 // Port 3, bit 0 - Processing active
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
