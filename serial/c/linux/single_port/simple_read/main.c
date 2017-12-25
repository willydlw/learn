/****************************************************************
* FILENAME:     main.c
*
* DESCRIPTION:  
*   Establishes serial connection to /dev/ttyACM0, 9600 baud
*       
*
*
* AUTHOR:   willydlw        START DATE: 12/24/17
*
* CHANGES:
*
* DATE          WHO        DETAIL
*
*/



#include <stdio.h>      // Standard input/output definitions
#include <stdint.h>     // uint8_t
#include <unistd.h>     // usleep

#include "serial.h"



int main(){

    // serial

	int fd = -1;
    int baudrate = 9600;

	const char* serial_device_path = "/dev/ttyACM0";
	
	ssize_t bytes_read;
	uint8_t buf[16];

    ReadState readUntilState;

    // file logging 
    FILE *fp = NULL;

    int less_than_expected_count = 0;
    int readFailureCount = 0;
    int readTimeoutCount = 0;

    // loop control
    int loopCount = 0;
    const int testLength = 100;

    // establish serial connection
	fd = serial_init(serial_device_path, baudrate);

	if(fd == -1){
		puts("serial port did not open");
		return -1;
	}

    // test serial read function and log results

    fp = fopen("read_results.txt", "w");
    if(fp == NULL){
        fprintf(stderr, "file read_results.txt failed to open");
        fp = stderr;
    }


    fprintf(fp, "serial read results, attempting to read 5 bytes at one time\n");
    while(loopCount < testLength){
        bytes_read = serial_read(fd, buf, 5);
        // only log when 5 bytes are not read
        if(bytes_read != 5){
            fprintf(fp, "bytes_read: %ld\n", bytes_read);
            ++less_than_expected_count;
        }

        ++loopCount;
        usleep(10*1000);       // suspend program execution for microseconds
    }

    fprintf(fp, "\n\nnumber of times less than 5 bytes were read at one time: %4d\n", 
                less_than_expected_count);

    fprintf(fp, "\n\ntotal times read function called:                        %4d\n", 
                testLength);


    fclose(fp);
    fp = NULL;


    // test serial read until function and log results

    fp = fopen("read_until_results.txt", "w");
    if(fp == NULL){
        fprintf(stderr, "file read_until_results.txt failed to open");
        fp = stderr;
    }


    fprintf(fp, "serial read until results, attempting to read until end marker\n");
    loopCount = 0;
    readFailureCount = 0;
    readTimeoutCount = 0;

	while(loopCount < testLength){

        // 100 is timeout in msec
		readUntilState = serial_read_until(fd, buf, 16, (uint8_t)'>', 100, &bytes_read);


        if(readUntilState == READ_FAILURE){
            fprintf(stderr, "read state: READ_FAILURE, bytes_read: %ld\n", bytes_read);
            ++readFailureCount;

        }else if(readUntilState == READ_TIMEOUT){
            fprintf(stderr, "read state: READ_TIMEOUT, bytes_read: %ld\n", bytes_read);
            ++readTimeoutCount;
        }

        ++loopCount;
        usleep(10*1000);           // suspend program execution for microseconds
	} // end while


    // write totals
    fprintf(fp, "\nfunction serial_read_until called %4d times\n", loopCount);
    fprintf(fp, "\ntotal read failures               %4d \n", readFailureCount);
    fprintf(fp, "\ntotal read timeouts               %4d \n", readTimeoutCount);


    // close file and serial connection

    fclose(fp);
    fp = NULL;

    serial_close(fd);
    fd = -1;


	return 0;
}
