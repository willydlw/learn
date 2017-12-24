#include <stdio.h>      // Standard input/output definitions
#include <stdint.h>     // uint8_t
#include <unistd.h>     // usleep



#include "serial.h"



int main(){

	int fd = -1;

	const char* serial_device_path = "/dev/ttyACM0";

	int baudrate = 9600;

	ssize_t bytes_read;

    ReadState readUntilState;

	uint8_t buf[16];


	fd = serial_init(serial_device_path, baudrate);

	if(fd == -1){
		puts("serial port did not open");
		return -1;
	}

	while(1){

		//bytes_read = serial_read(fd, buf, 5);

		readUntilState = serial_read_until(fd, buf, 16, (uint8_t)'>', 100, &bytes_read);


        if(bytes_read){
            fprintf(stderr, "\nbytes_read: %ld\n", bytes_read);

        }  // end if


        usleep(1000);           // suspend program execution for microseconds
	} // end while


	return 0;
}
