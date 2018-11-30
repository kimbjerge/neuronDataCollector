/*
 * CliTCPThread.h
 *
 *  Created on: 28. nov. 2018
 *      Author: au288681
 */

#ifndef SRC_CLITCPTHREAD_H_
#define SRC_CLITCPTHREAD_H_

#include "Thread.h"
using namespace AbstractOS;
#include "CliCommand.h"

#define CLI_TCP_PORT 			7
#define RECV_BUF_SIZE 			8192 //Space to receive maximum CLI command or answer

class CliTCPThread : public Thread
{
public:

	CliTCPThread() {}

	void addCommand(CliCommand *pCmd);

	virtual void run();
	static void process_echo_request(void *p);

private:
	void print_app_header();

};

#endif /* SRC_CLITCPTHREAD_H_ */
