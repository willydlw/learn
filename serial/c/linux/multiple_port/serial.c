/**
 * Copyright (c) 2017 willydlw
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See `main.c` for details.
 */


/**@file serial.c
* 
* @brief Serial communiciation functions for Linux
*
*
* @author willydlw
* @date 30 Jan 2018
* @bugs No known bugs
*
*/


// must be defined before including header files
// http://man7.org/linux/man-pages/man2/nanosleep.2.html
#define  _POSIX_C_SOURCE 199309L         // nanosleep is not IS0-C


#include <string.h>         // strerror 
#include <unistd.h>         // UNIX standard function definitions
#include <fcntl.h>          // File control definitions
#include <errno.h>          // Error number definitions
#include <termios.h>        // POSIX terminal control definitions
#include <time.h>           // nanosleep
#include <sys/ioctl.h>

#include <debuglog.h>

#include "serial.h"




/**
* @brief Establishes serial port connection to serial_device_name, at
*        the specified baud rate.
*
*        8 data bits, no parity, one stop bit
*        raw mode, so binary data can be sent/received
*
*        Non-blocking mode. No mimimum characters to read. Zero time
*        waiting for data.
*
* \par Process
*
*       [1] open serial port connection 
*       [2] sets speed to specified baud rate
*           If baud rate is not valid, sets to default rate 9600
*       [3] 8 data bits, No parity, 1 stop bit
*       [4] Raw mode, so binary data can be sent
*       [5] minimum characters to read: 0
*       [6] time to wait for data: 0
*       [7] flushes data from input & output lines
*       [8] returns valid file descriptor or -1 on error
*      
**
* @param[in] serial_device_name         serial device path name
*                                       example: "/dev/ttyUSB0"
*
* @param[in] baud_rate                  baud rate 
*
* @return success     serial port file descriptor.
*         failure     -1
*
*/
int serial_init(const char *serial_device_name, int baud_rate)
{
    int serial_port_fd;
    speed_t bspeed; 
    struct termios terminalSettings;

    int millisec = 10;      // length of time to sleep, in milliseconds
    struct timespec sleepTime = {0};
    sleepTime.tv_sec = 0;
    sleepTime.tv_nsec = millisec * 1000000L;

    // Open the serial port nonblocking (read returns immediately)
    serial_port_fd = open(serial_device_name, O_RDWR | O_NOCTTY | O_NDELAY);
    if(serial_port_fd < 0)             // open returns -1 on error
    {
        log_fatal("serial port not open, errno - %s", strerror(errno));
        log_fatal("serial device name: %s", serial_device_name);
        return -1;
    }


    // Get current serial port settings
    if(tcgetattr(serial_port_fd, &terminalSettings) < 0){
        log_fatal("get current serial port settings, %s", strerror(errno));
        return -1;
    }


    // set baud rate
    bspeed = set_baud_speed(baud_rate);
    if(bspeed != 0){
        int setspeed;

        setspeed = cfsetispeed(&terminalSettings, bspeed);
        if(setspeed != 0){
            log_fatal("cfsetispeed failure. baud_rate: %d, bspeed: %d",
                        baud_rate, bspeed);
            return -1;
        }

        setspeed = cfsetospeed(&terminalSettings, bspeed);
        if(setspeed != 0){
            log_fatal("cfsetospeed failure, baud_rate %d, bspeed %d",
                        baud_rate, bspeed);
            return -1;
        }
    }
    else{
        // no logging here, log message generated in set_baud_speed
        return -1;
    }

    // Flag settings from https://en.wikibooks.org/wiki/Serial_Programming/termios

    // input flags - turn off input processing

    // convert break to null byte, no CR to NL translation,
    // no NL to CR translation, don't mark parity errors or breaks
    // no input parity check, don't strip high bit off,
    // no XON/XOFF software flow control
    //
    terminalSettings.c_iflag &= (unsigned int)(~(IGNBRK | BRKINT | ICRNL |
                         INLCR | PARMRK | INPCK | ISTRIP | IXON));


    //
    // Output flags - Turn off output processing
    //
    // no CR to NL translation, no NL to CR-NL translation,
    // no NL to CR translation, no column 0 CR suppression,
    // no Ctrl-D suppression, no fill characters, no case mapping,
    // no local output processing
    //
    // config.c_oflag &= ~(OCRNL | ONLCR | ONLRET |
    //                     ONOCR | ONOEOT| OFILL | OLCUC | OPOST);
    terminalSettings.c_oflag = 0;

     //
     // No line processing
     //
     // echo off, echo newline off, canonical mode off, 
     // extended input processing off, signal chars off
     //
     terminalSettings.c_lflag &= (unsigned int)(~(ECHO | ECHONL | ICANON 
                                    | IEXTEN | ISIG));

     //
     // Turn off character processing
     //
     // clear current char size mask, no parity checking,
     // no output processing, force 8 bit input
     //
     terminalSettings.c_cflag &= (unsigned int)(~(CSIZE | PARENB));
     terminalSettings.c_cflag |= CS8;

    /* CREAD - enable receiver
       CLOCAL should be enabled to ensure that your program does not become 
       the 'owner' of the port subject to sporatic job control and hangup signals
    */
    //terminalSettings.c_cflag |= CREAD | CLOCAL;     // enable receiver, ignore ctrl lines
    

    // VMIN sets minimum characters to read
    terminalSettings.c_cc[VMIN]  = 0;

    // time to wait for data (tenths of seconds)
    terminalSettings.c_cc[VTIME] = 0;


    // Load new settings
    if( tcsetattr(serial_port_fd, TCSAFLUSH, &terminalSettings) < 0){
        log_fatal("did not set term attributes, errno %s", strerror(errno));
        return -1;
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
    nanosleep(&sleepTime, (struct timespec *)NULL);


    if( tcflush(serial_port_fd, TCIOFLUSH) != 0){
        log_warn("tcflush failed, errno %s", strerror(errno));
    }

    return serial_port_fd;
}



/**
* @brief returns corresponding baud speed for baud rate
*
* @param[in]         baud_rate               baud rate (bps) 
* 
* @return success     baud speed
*         failure     -1 when parameter baud_rate is not a 
*                     recognized speed
*
*/
speed_t  set_baud_speed(int baud_rate)
{
    switch(baud_rate)
    {
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
        log_fatal("baud rate %d not supported", baud_rate);
        return 0;
    }
}



/**
* @brief Attempts to read up to count bytes and store in the buffer,
*        starting at buf
*
* @param[in]    fd                      file descriptor
* @param[in]    count                   max number of bytes to read 
*
* @param[out]   buf                     bytes read are stored in buffer
* 
* @return   success     number of bytes read
*           failure     -1 and errno is set appropriately
*       
*/
ssize_t serial_read(int fd, uint8_t *buf, size_t count){

    // when operating in raw mode, read(2) system call will return however many 
    // characters are actually available in the serial input buffers
    return read(fd, buf, count);
}




/**
* @brief Attempts to read bytes from the file descriptor
*        into the buffer starting at buf.
*
*        Stops after reading and storing the until byte.
*        Stops after reading buf_max bytes.
*        Stops after timeout.
*
*  
* @param[in]    fd                      file descriptor
* @param[in]    buf_max                 maximum bytes to read
* @param[in]    until                   byte that ends read
* @param[in]    timeout                 stop reading after this much time (msec)
*
* @param[out]   buf                     bytes read are stored in buffer
* @param[out]   bytes_read              number of bytes read and stored in buf
*
* @return success     READ_UNTIL
*         failure     READ_FAILURE or READ_TIMEOUT
*    
* @notes
*
*       It is possible that bytes will be read and the function will timeout.
*       Check the bytes_read value to avoid losing bytes on a timeout condition.
*
*/
SerialReadState serial_read_until(int fd, uint8_t* buf, int buf_max, uint8_t until, 
                            int timeout, ssize_t *bytes_read)
{
    uint8_t b[1];           // read requires an array
    ssize_t i = 0;

    *bytes_read = 0;

    do {
        ssize_t n = read(fd, b, 1);         // read a char at a time

        if( n == -1){
            return READ_FAILURE;                      // read failed
        }

        if( n == 0 ) {  

            int millisec = 1;      // length of time to sleep, in milliseconds
            struct timespec sleepTime = {0};
            sleepTime.tv_sec = 0;
            sleepTime.tv_nsec = millisec * 1000000L;                    
            nanosleep(&sleepTime, (struct timespec*)NULL);  // wait 1 msec, try again

            timeout--;
            if( timeout==0 ){
                return READ_TIMEOUT;
            }

            continue;
        }

        buf[i] = b[0];
        i++;
        *bytes_read = i;

    } while( b[0] != until && i < buf_max && timeout > 0 );

    
    return READ_UNTIL;
}



/**
* @brief Writes up to count bytes from the buffer pointed to by buf,
*        to the file pointed to by the file descriptor fd
*
* @param[in]    fd                      file descriptor
* @param[in]    buf                     pointer to buffer
* @param[in]    count                   number of bytes to write
*
* @return success     number of bytes written
*         failure     -1 and errno is set appropriately
*               
* @notes
*       number of bytes written may be less than count
*
*/
ssize_t serial_write(int fd, const char *buf, size_t numbytes){
   return write(fd, buf, numbytes);
}



/**
* @brief Closes serial connection
*
* @param[in]            fd                      file descriptor
*
* @return success     zero
*         failure     -1 and errno is set appropriately
*
*/
int serial_close(int fd){
    return close(fd);
}