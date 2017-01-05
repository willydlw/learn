/** Illustrate usage of serial port functions
*/

#include <stdint.h>
#include "serial.h"


int main()
{
	char* serial_device_name = "/dev/ttyUSB0";
   int baud_rate = 9600;

   int fd;			// serial port file descriptor

	fd = init_serial(serial_device_name, baud_rate);

	// terminate program if serial port fails to open
   if (fd < 0)
		return -1;	

   uint8_t command_buffer[3] = { 1, 2, 3 };

   write_buffer(fd, command_buffer, 3);

   shutdown_serial(fd);


	return 0;
}	
