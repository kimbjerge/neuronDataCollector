/*
 * file_execution_test.h
 *
 *  Created on: Sep 3, 2014
 *      Author: stevem
 */

#ifndef FILE_EXECUTION_TEST_H_
#define FILE_EXECUTION_TEST_H_

void bit_execution_test(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
void elf_execution_test(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
void elf_test(void);

extern struct File_Params file_info;


#endif /* FILE_EXECUTION_TEST_H_ */
