#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>                     // memset
#include <termios.h>
#include <unistd.h>

#include <sys/time.h>

#include "cserial.h"



/**
*\fn int initialize_serial(const char *serial_device_name, int baud_rate)
*
*\param[in]
*   	serial_device_name - example: "/dev/ttyS1"
*   	baud_rate - baud rate in bits per second
*
*\return
*       Success - returns the serial port file descriptor.
*       Failure - returns -1
*
*
*/
int initialize_serial(const char *serial_device_name, int baud_rate)
{
    int serial_port_fd;
    int serial_speed = set_baud_speed(baud_rate);
    struct termios oldtio,newtio;

    // Open the serial port nonblocking (read returns immediately)
    serial_port_fd = open(serial_device_name, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if(serial_port_fd < 0)             // open returns -1 on error
    {
        fprintf(stderr, "Error: %s, serial port not open, errno -%s\n", __FUNCTION__, strerror(errno));
        fprintf(stderr, "serial device name: %s\n", serial_device_name);
        return SERIAL_OPEN_ERROR;
    }

    else    fprintf(stderr, "serial port fd = %d\n", serial_port_fd);


    int fcntlReturn;

    // allow the process to receive SIGIO
    fcntlReturn = fcntl(serial_port_fd, F_SETOWN, getpid());
    if(fcntlReturn < 0){
            fprintf(stderr,"error, %s, %d, %s\n", __FUNCTION__, __LINE__, strerror(errno));
    }

    /* If you set the O_ASYNC status flag on a file descriptor,
       a SIGIO signal is sent whenever input or output becomes
       possible on that file descriptor

       Reference: ftp://ftp.gnu.org/old-gnu/Manuals/glibc-2.2.3/html_chapter/libc_13.html#SEC264
    */

    // make the file descriptor asynchronous
    fcntlReturn =fcntl(serial_port_fd, F_SETFL, FASYNC);
    if(fcntlReturn < 0){
            fprintf(stderr,"error, %s, %d, %s\n", __FUNCTION__, __LINE__, strerror(errno));
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
    newtio.c_cflag = serial_speed | CS8 | CLOCAL | CREAD; //  | IGNBRK;

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
    newtio.c_lflag = ~ICANON;
    newtio.c_cc[VMIN] = 1;
    newtio.c_cc[VTIME] = 0;

    // Load new settings
    if( tcsetattr(serial_port_fd, TCSAFLUSH, &terminalSettings) < 0){
        perror("init_serialport: Couldn't set term attributes");
        return SERIAL_OPEN_ERROR;
    }

    // clear data from both input/output buffers
    /* When using a usb serial port, the USB driver does not
       know if there is data in the internal shift register, FIFO
       or USB subsystem. As a workaround, to ensure the data
       is flushed, add a sleep delay here to suspend program
       execution. This allows time for data to arrive and be
       stored in the buffers. A call to flush will then work.

       May need to experiment with sleep time.
    */
    usleep(10000);                                  // 10 ms
    tcflush(serial_port_fd, TCIOFLUSH);

    fprintf(stderr, "*** Linux Serial ready ***\n");

    return serial_port_fd;
}


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
    case 115200:
        return B115200;
    default:
        return B9600;
    }
}





ssize_t read_serial(int serial_port_fd, long int delay_time, uint8_t* readbuffer, size_t numbytes)
{
    /*  The GNU C library provides two data types specifically for representing an elapsed time.
        Data Type: struct timeval

        The struct timeval structure represents an elapsed time. It is declared in sys/time.h and has the following members:

        long int tv_sec     This represents the number of whole seconds of elapsed time.
        long int tv_usec    This is the rest of the elapsed time (a fraction of a second),
                            represented as the number of microseconds. It is always less than one million.

*/

    int select_return_value;
    struct timeval  timeout;
    fd_set read_set;                                    // file descriptor set

    timeout.tv_sec = delay_time / 1000000L;             // one sec in units of microseconds
    timeout.tv_usec = delay_time % 1000000L;


    /*  void FD_SET(int fd, fd_set *set);
        void FD_ZERO(fd_set *set);
    */
    FD_ZERO (&read_set);                       // clears the set
    FD_SET(serial_port_fd, &read_set);         // adds the file descriptor to the set

    /*  select() allows a program to monitor multiple file descriptors,
        waiting until one or more of the file descriptors become "ready" for
        some class of I/O operation (e.g., input possible). A file descriptor is
        considered ready if it is possible to perform the corresponding I/O operation
        (e.g., read(2)) without blocking.

        int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
    */

    select_return_value = select(serial_port_fd+1, &read_set, NULL, NULL, &timeout);

    if(FD_ISSET(serial_port_fd, &read_set))    // input from source available
    {
            /* ssize_t read(int fd, void *buf, size_t count);
               read attempts to read count bytes from file descriptor fd into the buffer starting at buf

               On success, the number of bytes read is returned.
               On error, -1 is returned and errno is set appropriately
            */

            memset(readbuffer, '0', 2);
            return read(serial_port_fd,readbuffer,numbytes);
    }
    else
    {
            /*  Errors
            EBADF - An invalid file descriptor was given in one of the sets.
            (Perhaps a file descriptor that was already closed, or one on which an error has occurred.)

            EINTR - A signal was caught.
            EINVAL - nfds is negative or the value contained within timeout is invalid.
            ENOMEM - unable to allocate memory for internal tables.
            */
            fprintf(stderr, "%s, error on select - %s\n", __FUNCTION__, strerror(errno));
            fprintf(stderr, "select_return_value = %d\n",select_return_value);
            return -1;
    }

}



/**
*\fn int write_byte(int fd, uint8_t byte)
*
*\param[in]
*   	fd - file descriptor
*   	byte - data byte to write
*
*\return
*       Success - returns 1
*       Failure - returns 0
*
*/
ssize_t write_byte(int fd, uint8_t byte)
{
    return write(fd, &byte, 1);
}

/**
*\fn int write_buffer(int fd, const char* str)
*
*\param[in]
*   	fd - file descriptor
*   	str - data string to write
*
*\return
*       returns number of bytes written
*
*/
ssize_t write_buffer(int fd, const char* str)
{
    return write(fd, str, strlen(str));
}



int close_serial(int fd){

    return close(fd);
}
