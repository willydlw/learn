/*
*
*   Filename: linuxSerialSidePolling.c
*
*   Program written to test RS-232 serial communication between this program
*   runnning on a Linux OS and an atmega microprocessor
*
*
*   The atmega microprocessor program sends a two byte count at approximate
*   one second intervals.
*
*   An infinite while loop in the main function reads and displays the serial
*   data received, then sleeps for one second.
*
*   Note: If this continuous polling method is used, there are timing issues
*         with receiving all the data from the microprocessor. If the microprocessor
*         timing is changed, this program must be changed as well.
*
*   Suggested method for testing. Send a known serial count from to this program.
*   Test the data received against the incremental count to determine how much data
*   is lost if any.
*/

#include <aio.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>



enum ErrorConditions { SERIAL_OPEN_ERROR = -1};

 // Program constants
const char *SERIAL_DEVICE_NAME = "/dev/ttyUSB0";
const long ONE_SEC_IN_USEC = 1000000L;



/****** Function Prototypes *****/
int  Configure_Serial(int serial_baud_rate);
int Init_Serial(const char *serial_device_name, int baud_rate);

ssize_t ReadSerial(int serial_port_fd, long int delay_time, uint8_t* readbuffer, size_t numbytes);



/*
*   Input:
*   	serial_device_name example: "/dev/ttyS1"
*   	baud_rate is the baud rate in bits per second
*
*   Output:
*       Success - returns the serial port file descriptor.
*       Failure - returns -1
*
*/
int Init_Serial(const char *serial_device_name, int baud_rate)
{
    int serial_port_fd;
    int serial_speed = Configure_Serial(baud_rate);
    struct termios oldtio,newtio;

    // Open the serial port nonblocking (read returns immediately)
    serial_port_fd = open(serial_device_name, O_RDWR | O_NOCTTY ); //| O_NONBLOCK);
    if(serial_port_fd < 0)             // open returns -1 on error
    {
        fprintf(stderr, "Error: Init_LinuxSerialSide, serial port not open, errno -%s\n", strerror(errno));
        return SERIAL_OPEN_ERROR;
    }

    else    fprintf(stderr, "serial port fd = %d\n", serial_port_fd);


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

    // Empty serial port buffers
    tcflush(serial_port_fd, TCIFLUSH);

    // Load new settings
    tcsetattr(serial_port_fd, TCSANOW, &newtio);
    printf("*** Linux Serial ready ***");

    return serial_port_fd;
}

int  Configure_Serial(int serial_baud_rate)
{
    switch(serial_baud_rate)
    {
    case 2400:
        return B2400;
    case 9600:
        return B9600;
    default:
        return B9600;
    }
}



ssize_t ReadSerial(int serial_port_fd, long int delay_time, uint8_t* readbuffer, size_t numbytes)
{
    /*  The GNU C library provides two data types specifically for representing an elapsed time.
        Data Type: struct timeval

        The struct timeval structure represents an elapsed time. It is declared in sys/time.h and has the following members:

        long int tv_sec     This represents the number of whole seconds of elapsed time.
        long int tv_usec    This is the rest of the elapsed time (a fraction of a second),
                            represented as the number of microseconds. It is always less than one million.

*/

    int select_return_value;
    timeval  timeout;
    fd_set read_set;                                    // file descriptor set

    timeout.tv_sec = delay_time / ONE_SEC_IN_USEC;
    timeout.tv_usec = delay_time % ONE_SEC_IN_USEC;


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
            fprintf(stderr, "ReadSerial, error on select - %s\n", strerror(errno));
            fprintf(stderr, "select_return_value = %d\n",select_return_value);
            return -1;
    }
}


int main(void)
{
    int fd;
    ssize_t bytes_read;
    uint8_t dataBuffer[2];


    fd = Init_Serial("/dev/ttyUSB0", 9600);
    switch(fd)
    {
        case SERIAL_OPEN_ERROR:
            fprintf(stderr, "main: SERIAL_OPEN_ERROR returned, fd = %d, program terminating\n", fd);
            return SERIAL_OPEN_ERROR;
            break;

        default:
            fprintf(stderr, "main: Init_Serial returned valid file descpritor %d\n",fd);
            break;
    }


    while(1)
    {
        bytes_read = ReadSerial(fd, 1000000, dataBuffer, 2);
        fprintf(stderr, "bytes_read = %d, ",bytes_read);
        if(bytes_read > 0)
            fprintf(stderr, "%x %x\n", dataBuffer[1], dataBuffer[0]);

        usleep(1000000);
    }

    close(fd);		// close serial port
    fd = -1;

    return 0;
}
