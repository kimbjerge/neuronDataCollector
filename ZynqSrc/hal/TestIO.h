/*
 * TestIO.h
 *
 *  Created on: 29. dec. 2018
 *      Author: Kim Bjerge
 */

#ifndef SRC_HAL_TESTIO_H_
#define SRC_HAL_TESTIO_H_

class TestIO : public Gpio
{
public:

	enum IOTypes {
		JB1 = 0,
		JB2 = 1,
		JB3 = 2,
		JB4 = 3,
		JB7 = 4,
		JB8 = 5,
		JB9 = 6,
		JB10 = 7
	};

	TestIO() : Gpio(XPAR_TESTIO_DEVICE_ID)
	{
		XGpio_SetDataDirection(&mGpioHandle, mChannel, 0x00); // Set as output
		mBits = 0;
		writeio(mBits); // Turn all off
	};

	void setOn(IOTypes io, bool on)
	{
        if (on)
        	mBits |= 1 << io;
        else {
        	mBits &= ~(1 << io);
        }

		writeio(mBits);
	}

private:
	int mBits;

};


#endif /* SRC_HAL_TESTIO_H_ */
