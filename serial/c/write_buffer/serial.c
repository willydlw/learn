/*
*
*   Filename: serial.c
*
*   serial port functions
*		intialize serial port
*     write bytes to serial port
*     close serial port
*/

#include <string.h>				// memset
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#include "serial.h"


int  set_baud_speed(int serial_baud_rate)
{
    switch(serial_baud_rate)
    {
    case 2400:
        return B2400;
    case 9600:
        return B9600;
	 case 19200:
	     return B19200;
    case 38400:
		  return B38400;
    case 57600:
        return B57600;
    default:
        return B9600;
    }
}

/* 
*
*   Parameters:
*		serial_device_name example: "/dev/ttyS1"
*   	baud_rate is the baud rate in bits per second
*
*   Sets port for 8 data bits, no parity, 1 stop bit
*
*   Returns serial port file descriptor
*/
int init_serial(const char *serial_device_name, int baud_rate)
{
    int serial_port_fd;
    int serial_speed = set_baud_speed(baud_rate);
    struct termios oldtio,newtio;

    // Open the serial port for read and write, not the process's controlling terminal
    serial_port_fd = open(serial_device_name, O_RDWR | O_NOCTTY);
    if(serial_port_fd < 0)             // open returns -1 on error
    {
        fprintf(stderr, "Error: init_serial, serial port not open\n");
        return serial_port_fd;
    }
    
    // Get current serial port settings
    tcgetattr(serial_port_fd, &oldtio);

    // New port settings
    memset(&newtio, 0, sizeof(newtio));           // set all struct values to zero
    /* Set Control mode
    *  CS8      - 8 data bits, no parity, 1 stop bit
    *  CLOCAL   - local connnection, no modem control
    *  CREAD    - enable receiving characters
    *  IGNBRK   - ignore break condition
    */
    newtio.c_cflag = serial_speed | CS8 | CLOCAL | CREAD | IGNBRK;

    /* IGNPAR - ignore bytes with parity errors
    *  ICRNL -  map CR to NL (otherwise a CR input on the other computer
                    will not terminate input)
    *
    */
    newtio.c_iflag = IGNPAR | ICRNL;
    newtio.c_oflag = 0;                     // raw output
    /*
    * ICANON  : enable canonical input
    *           disable all echo functionality, and don't send signals to calling program
    */
    newtio.c_lflag = ICANON;

    // Empty serial port buffers
    tcflush(serial_port_fd, TCIFLUSH);

    // Load new settings
    tcsetattr(serial_port_fd, TCSANOW, &newtio);
    fprintf(stderr, "*** serial port initialized ready ***");

    return serial_port_fd;
}


void shutdown_serial(int serial_file_descriptor)
{
    fprintf(stderr, "closing serial port\n");
    close(serial_file_descriptor);
}



/*** 
*
*  Parameters:
*		serial_file_descriptor - serial port file descriptor 
*     buffer: data to be written 
*     buffer_length: number of bytes in the buffer array 
*/
void write_buffer(int serial_file_descriptor, uint8_t *buffer, int buffer_length)
{
    ssize_t bytes_written;
    bytes_written = write(serial_file_descriptor, buffer, buffer_length);
   	
    if(bytes_written <= 0)
      fprintf(stderr, "write_buffer failed\n");
    else
      fprintf(stderr, "serial bytes written: %zd\n", bytes_written);

}


