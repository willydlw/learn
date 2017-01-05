/*
*
*   Filename: linuxSerialSide.c
*
*   Program written to test RS-232 serial communication between this program
*   runnning on a Linux OS and an atmega microprocessor
*
*/

#include <aio.h>
#include <cstdlib>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdint.h>
#include <string>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <unistd.h>



enum ErrorConditions { SERIAL_OPEN_ERROR = -1, SIG_ACTION_ERROR = -2, COMMAND_LINE_ERROR = -3};

 // Program constants
const char *SERIAL_DEVICE_NAME = "/dev/ttyUSB0";
const long ONE_SEC_IN_USEC = 1000000L;




// IO Signals

/* struct sigaction{
    void (*sa_handler)(int);
    void (*sa_sigaction)(int, siginfo_t *, void*);
    sigset_t	sa_mask;
    int		sa_flags;
    void (*sa_restorer)(void);
}
*/
struct sigaction saio;
static volatile sig_atomic_t wait_flag = 1;


/****** Function Prototypes *****/
int set_baud_speed(int serial_baud_rate);
int initialize_serial(const char *serial_device_name, int baud_rate);

void signal_handler_IO(int status);
ssize_t read_serial(int serial_port_fd, long int delay_time, uint8_t* readbuffer, size_t numbytes);

bool parse_command_line(int argc, char* argv[], std::string& serialDevice, int *baudRate);


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
        fprintf(stderr, "Error: Init_LinuxSerialSide, serial port not open, errno -%s\n", strerror(errno));
        fprintf(stderr, "serial device name: %s\n", serial_device_name);
        return SERIAL_OPEN_ERROR;
    }

    else    fprintf(stderr, "serial port fd = %d\n", serial_port_fd);

    // install the signal handler before making the device asynchronous
    memset(&saio, '\0', sizeof(saio));
    saio.sa_handler = signal_handler_IO;
    saio.sa_flags = 0;
    saio.sa_restorer = NULL;
    /* int sigaction (int signum, const struct sigaction *act, struct sigaction *oldact); */
    if(sigaction(SIGIO, &saio, NULL) < 0)
    {
        fprintf(stderr, "Init_Serial, sigaction error\n");
        return SIG_ACTION_ERROR;
    }

    int fcntlReturn;

    // allow the process to receive SIGIO
    fcntlReturn = fcntl(serial_port_fd, F_SETOWN, getpid());
    fprintf(stderr,"fcntlReturn = %d\n", fcntlReturn);
    // make the file descriptor asynchronous
    fcntl(serial_port_fd, F_SETFL, FASYNC);
    fprintf(stderr,"fcntlReturn = %d\n", fcntlReturn);



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
            fprintf(stderr, "%s, error on select - %s\n", __FUNCTION__, strerror(errno));
            fprintf(stderr, "select_return_value = %d\n",select_return_value);
            return -1;
    }
}



void signal_handler_IO(int sig)
{
    // uncomment psignal for debugging.
    psignal(sig, "signal_handler_IO");
    wait_flag = 0;
}


bool parse_command_line(int argc, char* argv[], std::string& serialDevice, int *baudRate)
{
    int i = 1;
    if(argc < 2)
        return true;
    while(i < argc-1)
    {
        /// strcmp returns 0 when they are equal
        if(!strcmp(argv[i],"--baud") || !strcmp(argv[i],"-baud"))
           {
            *baudRate = atoi(argv[i+1]);
           }
        else if(!strcmp(argv[i],"--device") || !strcmp(argv[i],"-device"))
            serialDevice = argv[i+1];
        else
            fprintf(stderr, "Unknown command line argument: %s\n", argv[i]);
        i = i + 2;
    }
    return true;
}


int main(int argc, char *argv[])
{
    int fd;
    ssize_t bytes_read;
    uint8_t dataBuffer[1];
    std::string serialDeviceInput(SERIAL_DEVICE_NAME);
    int baudRateInput = 9600;

    uint16_t countReceived = 0;

    // set wait because a signal interrupt has not yet been recognized
    wait_flag = 1;

    if(!parse_command_line(argc, argv, serialDeviceInput, &baudRateInput))
    {
        fprintf(stderr, "main: program terminating, command line error\n");
        fprintf(stderr, "usage: ./eserial -device /dev/ttyS0 -baud 9600\n");
        return COMMAND_LINE_ERROR;
    }

    fd = initialize_serial(serialDeviceInput.c_str(), baudRateInput);

    switch(fd)
    {
        case SERIAL_OPEN_ERROR:
            fprintf(stderr, "main: SERIAL_OPEN_ERROR returned, fd = %d, program terminating\n", fd);
            return SERIAL_OPEN_ERROR;
            break;
        case SIG_ACTION_ERROR:
            fprintf(stderr, "main: SIG_ACTION_ERROR returned, fd = %d, program terminating\n",fd);
            return SIG_ACTION_ERROR;
            break;
        default:
            fprintf(stderr, "main: initialize_serial returned valid file descriptor %d\n",fd);
            break;
    }

    fprintf(stderr, "")
    while(countReceived < 5)
    {
        if(wait_flag == 0)      // signal interrupt sets wait flag to zero
        {
            bytes_read = read_serial(fd, 100, &dataBuffer[0], 1);
            fprintf(stderr, "dataBuffer[0] %d\n", dataBuffer[0]);
            wait_flag = 1;
        }

        if(bytes_read > 0)
        {
            ++countReceived;
        }
        else
        {
            fprintf(stderr, "error: bytes_read %ld", bytes_read);
        }
    } // end while

    close(fd);		// close serial port
    fd = -1;

    return 0;
}

