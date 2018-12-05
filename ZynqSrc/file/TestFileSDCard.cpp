/*
 * TestFileSDCard.cpp
 *
 *  Created on: 3. dec. 2018
 *      Author: Kim Bjerge
 */

#include <stdio.h>
#include <string.h>
#include "FileSDCard.h"

static const char writeText[] = "This is a read me file\r\nWith more lines\r\n";
static char readText[256];

void TestFileSDCard(void)
{
	int result;
	FileSDCard file((char*)"0:/");

	result = file.mount();
	if (result != XST_SUCCESS) printf("Failed to mount SD card\r\n");

	// Create a new file if doesn't exist
	result = file.open((char*)"README.txt", FA_CREATE_ALWAYS | FA_WRITE);
	if (result != XST_SUCCESS) printf("Failed open file for writing\r\n");

	// Write to start of file
	result = file.write((void *)writeText, sizeof(writeText));
	if (result != XST_SUCCESS) printf("Failed writing to file\r\n");

	result = file.close();
	if (result != XST_SUCCESS) printf("Failed closing file\r\n");

	// Open created test file
	result = file.open((char*)"README.txt", FA_OPEN_EXISTING | FA_READ);
	if (result != XST_SUCCESS) printf("Failed open file for reading\r\n");

	// Read contents of test file
	result = file.read((void *)readText, sizeof(readText));
	if (result != XST_SUCCESS) printf("Failed reading from file\r\n");

	printf(readText);

	result = file.close();
	if (result != XST_SUCCESS) printf("Failed closing file\r\n");
}




