/*
 * UserThread.h
 *
 *  Created on: 20. July 2018
 *      Author: Kim Bjerge
 */

#ifndef SRC_USERTHREAD_H_
#define SRC_USERTHREAD_H_

// HPPIO.cpp
void initIO( void );
void setLEDSOn(bool on);
void setPort2Bit0(bool on);
void setAllPortsOn(bool on);
void toggleTTLOuput(short ttl_output_bitnum);

#include "Thread.h"
using namespace AbstractOS;

class UserThread : public Thread
{
public:

	UserThread(ThreadPriority pri, string name) :
		 Thread(pri, name)
	 {
		 counter = 1000;
	 }


	virtual void run()
	{
		initIO();

		while (counter > 0) {

			setLEDSOn(true);
			setPort2Bit0(true);
			//setAllPortsOn(true); // Don't locks but no outputs
			//toggleTTLOuput(0x0004); //ToggleDigIO locks - No ack received
			Sleep(500);

			printf("%s - %d\r\n", getName().c_str(), counter--);

			setLEDSOn(false);
			setPort2Bit0(false);
			//setAllPortsOn(false);
			//toggleTTLOuput(0x0004);
			Sleep(500);
		}
	}

private:
	int counter;
};




#endif /* SRC_USERTHREAD_H_ */
