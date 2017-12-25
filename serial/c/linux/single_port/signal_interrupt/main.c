/*  Purpose: signal I0 interrupts detect when serial bytes are received.
*            displays message each time signal_handler_IO function executes
*
*            serial bytes received are stored and written to stderr
*            after MAX_DATA_BYTES have been read.
*            
*            When there is a signalIO interrupt and a byte is not
*            successfully read, the read error count is incremented.
*
*            read error count only displays when greater than zero
*
*            program terminates after reading MAX_DATA_BYTES
*
*
*   Input:
*
*       - Serial connection
*           - Default serial device setting: /dev/ttyACM0
*               - Command line override of default requires string arguments of
*                   -device followed by the device path.
*
*               Example: for a device path of /dev/ttyUSB0, command line arguments
*                        are -device /dev/ttyUSB0
*
*           - Default baud rate is 9600.
*               - Command line override of default requires string arguments of
*                 -baud followed by the baud rate.
*
*                 Example: for a baud rate of 57600, command line arguments are
*                           -baud 57600
*
*
*   Filename: main.c
*
*   Program written to test serial communication between this program
*   runnning on a Linux OS and an Arduino UNO R3
*
*
*
*/


#include <aio.h>
#include <stdlib.h>
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

#include <cserial.h>


#include "command_line.h"


typedef enum signal_error_t { SIG_ACTION_ERROR = -2 } SignalError;


// Program constants
const char *SERIAL_DEVICE_NAME = "/dev/ttyACM0";
const long ONE_SEC_IN_USEC = 1000000L;


#define MAX_DATA_BYTES 25
#define SERIAL_DEVICE_NAME_LENGTH  64



/// IO Signals

/** struct sigaction{
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
void signal_handler_IO(int status);



void signal_handler_IO(int sig)
{
    /* psignal used for debugging purposes
       debugging console output lets us know this function was
       triggered. Prints the string message and a string for the
       variable sig
       */
    psignal(sig, "signal_handler_IO");
    wait_flag = 0;
}





int main(int argc, char *argv[])
{
    int fd;                     // serial file descriptor
    ssize_t bytes_read;
    uint8_t dataBuffer[2];      // stores data byte received

    uint8_t allData[MAX_DATA_BYTES] = {0};    // stores all data bytes received
    uint8_t readErrorCount;
    	

    char serialDevicePath[SERIAL_DEVICE_NAME_LENGTH];
    strcpy(serialDevicePath, SERIAL_DEVICE_NAME);

    int baudRate = 9600;

    uint16_t countReceived = 0;

    // set wait because a signal interrupt has not yet been recognized
    wait_flag = 1;

    // handle command line arguments
    process_command_line_arguments(argc, argv, serialDevicePath, &baudRate, SERIAL_DEVICE_NAME_LENGTH);

    
    // display serial device and baud rate input 
    fprintf(stderr, "\n\n*****  INPUT SETTINGS  *****\n\n");
    fprintf(stderr, "serial device: %s, baud rate: %d\n\n", serialDevicePath, baudRate);


    // install the signal handler before making the device asynchronous
    memset(&saio, '\0', sizeof(saio));
    saio.sa_handler = signal_handler_IO;
    saio.sa_flags = 0;
    saio.sa_restorer = NULL;

    /* int sigaction (int signum, const struct sigaction *act, struct sigaction *oldact); */
    if(sigaction(SIGIO, &saio, NULL) < 0){
        fprintf(stderr, "Init_Serial, sigaction error\n");
        return SIG_ACTION_ERROR;
    }

    fd = initialize_serial(serialDevicePath, baudRate);

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


    // now that serial port is open, and asynchronous signal interrupts are set up
    // we enter the main processing loop.

    fprintf(stderr, "\n\n******  PROCESSING  *****\n\n");
    fprintf(stderr, "main: entering while loop, remains here until %u bytes are received\n\n", MAX_DATA_BYTES);

    readErrorCount = 0;

    while(countReceived < MAX_DATA_BYTES)
    {
        if(wait_flag == 0)      // signal interrupt sets wait flag to zero
        {
            bytes_read = read_serial(fd, 100, &dataBuffer[0], 1);
            if(bytes_read > 0){
                allData[countReceived] = dataBuffer[0];
                ++countReceived;
            }
            else {              // signal interrupt occurred, but no bytes were read
                ++readErrorCount;
            }
            wait_flag = 1;      // reset wait flag to 1 after byte has been processed
        }


    } // end while

    close_serial(fd);		// close serial port
    fd = -1;                // to avoid accidental use, make invalid

    // Display data received
    fprintf(stderr, "\ncountReceived: %u\n", countReceived);
    fprintf(stderr, "\nList of all data received\n");

    for(int i = 0; i < countReceived; i++){
    	if( !(i%5)) fprintf(stderr, "\n");
        fprintf(stderr, "%5d ", allData[i]);
    }

    fprintf(stderr, "\n\n");


    // only display non-zero values of read error count
    if(readErrorCount != 0){
        fprintf(stderr, "\nread error count: %d\n", readErrorCount);
    }

    return 0;
}

