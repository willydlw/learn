/*  Purpose: Signal interrupts detect when serial bytes are received.
*            Echos serial bytes received to console.
*
*   Description:
*
*       - Serial connection
*           - Default serial device setting: /dev/ttyUSB0
*               - Command line override of default requires string arguments of
*                   -device followed by the device path.
*
*               Example: for a device path of /dev/ttyACM0, command line arguments
*                        are -device /dev/ttyACM0
*
*           - Default baud rate is 9600.
*               - Command line override of default requires string arguments of
*                 -baud followed by the baud rate.
*
*                 Example: for a baud rate of 58400, command line arguments are
*                           -baud 58400
*
*
*   Filename: linux_serial_signal_interrupt.c
*
*   Program written to test RS-232 serial communication between this program
*   runnning on a Linux OS and an atmega microprocessor
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

#include "cserial.h"


enum ErrorConditions { SIG_ACTION_ERROR = -2, COMMAND_LINE_ERROR = -3};


// Program constants
const char *SERIAL_DEVICE_NAME = "/dev/ttyUSB0";
const long ONE_SEC_IN_USEC = 1000000L;


#define MAX_DATA_BYTES 5



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
int parse_command_line(int argc, char* argv[], char* serialDevice, int *baudRate);


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


int parse_command_line(int argc, char* argv[], char* serialDevice, int *baudRate)
{
    int i = 1;
    if(argc < 2)
        return 0;
    while(i < argc-1)
    {
        /// strcmp returns 0 when they are equal
        if(!strcmp(argv[i],"--baud") || !strcmp(argv[i],"-baud"))
           {
            *baudRate = atoi(argv[i+1]);
           }
        else if(!strcmp(argv[i],"--device") || !strcmp(argv[i],"-device"))
            strcpy(serialDevice,argv[i+1]);     // does not ensure string length is correct
        else
            fprintf(stderr, "Unknown command line argument: %s\n", argv[i]);
        i = i + 2;
    }
    return 1;
}


int main(int argc, char *argv[])
{
    int fd;                     // serial file descriptor
    ssize_t bytes_read;
    uint8_t dataBuffer[1];      // stores data byte received

    uint8_t allData[MAX_DATA_BYTES];    // stores all data bytes received
    uint8_t readErrorCount;
    uint8_t first_byte;


    char serialDeviceInput[32];
    strcpy(serialDeviceInput, SERIAL_DEVICE_NAME);

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

    fd = initialize_serial(serialDeviceInput, baudRateInput);

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

    fprintf(stderr, "main: entering while loop, remains here until %u bytes are received\n\n", MAX_DATA_BYTES);
    readErrorCount = 0;
    while(countReceived < MAX_DATA_BYTES)
    {
        if(wait_flag == 0)      // signal interrupt sets wait flag to zero
        {
            bytes_read = read_serial(fd, 100, &dataBuffer[0], 1);
            if(bytes_read > 0){
                allData[countReceived] = dataBuffer[0];
                fprintf(stderr, "count received: %d, allData[%d]: %d, dataBuffer[0]: %d, %#x\n", countReceived,
                            countReceived, allData[countReceived], dataBuffer[0], dataBuffer[0]);
                if(countReceived == 0)
                    first_byte = allData[0];
                ++countReceived;
            }
            else {
                ++readErrorCount;
            }
            wait_flag = 1;
        }


    } // end while

    close_serial(fd);		// close serial port
    fd = -1;

    // Display data received
    fprintf(stderr, "\nList of all data received\n");
    fprintf(stderr, "first byte: %u\n", first_byte);
    for(int i = 0; i < countReceived; i++){
        fprintf(stderr, "allData[%d]: %u ", i, allData[i]);
    }

    fprintf(stderr, "\n");


    // only display non-zero values of read error count
    if(readErrorCount != 0){
        fprintf(stderr, "\nread error count: %d\n", readErrorCount);
    }

    return 0;
}

