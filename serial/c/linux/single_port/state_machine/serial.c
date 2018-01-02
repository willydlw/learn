/****************************************************************
* FILENAME:     serial.c
*
* DESCRIPTION:
*       Serial communiciation functions for Linux
*
*
* AUTHOR:   willydlw        START DATE: 12/24/17
*
* CHANGES:
*
* DATE          WHO        DETAIL
*
*/


#include <stdio.h>     
#include <string.h>         // strerror 
#include <unistd.h>         // UNIX standard function definitions
#include <fcntl.h>          // File control definitions
#include <errno.h>          // Error number definitions
#include <termios.h>        // POSIX terminal control definitions
#include <sys/ioctl.h>


#include "serial.h"


/**
* NAME : int serial_init(const char *serial_device_name, int baud_rate)
*
* DESCRIPTION: opens serial port
*
* INPUTS: 
*   Parameters:
*       const char*     serial_device_name              serial device path name
                                                        example: "/dev/ttyUSB0"
*       int             baud_rate                       baud rate 
*   Globals:
*       none
*
* OUTPUTS:
*   Return:
*       Type: int
*       Values:            
*           success     serial port file descriptor.
*           failure     -1
*
* PROCESS:
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
* NOTES:
*       Error messages are sent to stderr stream
*
*/
int serial_init(const char *serial_device_name, int baud_rate)
{
    int serial_port_fd;
    int bspeed; 
    struct termios terminalSettings;

    // Open the serial port nonblocking (read returns immediately)
    serial_port_fd = open(serial_device_name, O_RDWR | O_NOCTTY | O_NDELAY);
    if(serial_port_fd < 0)             // open returns -1 on error
    {
        fprintf(stderr, "error: %s, serial port not open, errno -%s\n", 
                    __FUNCTION__, strerror(errno));

        fprintf(stderr, "serial device name: %s\n", serial_device_name);
        return -1;
    }

    else    fprintf(stderr, "serial port fd = %d\n", serial_port_fd);


    // Get current serial port settings
    if(tcgetattr(serial_port_fd, &terminalSettings) < 0){
        perror("initialize_serial: could not get current serial port settings");
        return -1;
    }


    // set baud rate
    bspeed = set_baud_speed(baud_rate);
    if(bspeed != -1){
        cfsetispeed(&terminalSettings, bspeed);
        cfsetospeed(&terminalSettings, bspeed);
    }
    else{
        return -1;
    }

    // Flag settings from https://en.wikibooks.org/wiki/Serial_Programming/termios

    // input flags - turn off input processing

    // convert break to null byte, no CR to NL translation,
    // no NL to CR translation, don't mark parity errors or breaks
    // no input parity check, don't strip high bit off,
    // no XON/XOFF software flow control
    //
    terminalSettings.c_iflag &= ~(IGNBRK | BRKINT | ICRNL |
                         INLCR | PARMRK | INPCK | ISTRIP | IXON);


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
     terminalSettings.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);

     //
     // Turn off character processing
     //
     // clear current char size mask, no parity checking,
     // no output processing, force 8 bit input
     //
     terminalSettings.c_cflag &= ~(CSIZE | PARENB);
     terminalSettings.c_cflag |= CS8;


    /* prior version, remove if above output settings work
    // set control flags
    terminalSettings.c_cflag &= ~PARENB;            // no parity
    terminalSettings.c_cflag &= ~CSTOPB;            // 1 stop bit
    terminalSettings.c_cflag &= ~CSIZE;             // reset character size bits
    terminalSettings.c_cflag |= CS8;                // data size 8 bit
  
    terminalSettings.c_cflag &= ~CRTSCTS;           // no flow control
    */
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
        perror("init_serialport: Couldn't set term attributes");
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
    usleep(10000);                                  // 10 ms
    tcflush(serial_port_fd, TCIOFLUSH);

    return serial_port_fd;
}




/**
* NAME : int  set_baud_speed(baud_rate)
*
* DESCRIPTION: returns corresponding baud speed for baud rate
*
* INPUTS: 
*   Parameters:
*       int             baud_rate               baud rate (bps) 
*   Globals:
*       none
*
* OUTPUTS:
*   Return:
*       type:           int            
*       values:
*           success     baud speed
*           failure     -1 when parameter baud_rate is not
*                       a recognized speed
*
*      
* NOTES:
*       Error messages are sent to stderr stream
*
*/
int  set_baud_speed(int baud_rate)
{
    switch(baud_rate)
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
        fprintf(stderr, "error: %s, baud rate %d not supported\n", 
                    __FUNCTION__, baud_rate);
        return -1;
    }
}



/**
* NAME : ssize_t serial_read(int fd, void *buf, size_t count)
*
* DESCRIPTION: attempts to read up to count bytes from the file descriptor fd 
*              into the buffer starting at buf
*
* INPUTS: 
*   Parameters:
*       int             fd                      file descriptor
*       size_t          count                   max number of bytes to read 
*   Globals:
*       none
*
* OUTPUTS:
*   Parameters:
*       uint8_t*        buf                     bytes read are stored in buffer
*   Return:
*       type:           ssize_t            
*       values:
*           success     number of bytes read
*           failure     -1 and errno is set appropriately
*
*      
* NOTES:
*       
*
*/
ssize_t serial_read(int fd, uint8_t *buf, size_t count){

    // when operating in raw mode, read(2) system call will return however many 
    // characters are actually available in the serial input buffers
    return read(fd, buf, count);
}




/**
* NAME : ReadState serial_read_until(int fd, uint8_t* buf, int buf_max, uint8_t until, int timeout, ssize_t *bytes_read)
*
* DESCRIPTION: attempts to read bytes from the file descriptor fd
*              into the buffer starting at buf.
*
*              Stops after reading and storing the until byte.
*              Stops after reading buf_max bytes.
*              Stops after timeout.
*
* INPUTS: 
*   Parameters:
*       int             fd                      file descriptor
*       int             buf_max                 maximum bytes to read
*       uint8_t         until                   byte that ends read
*       int             timeout                 stop reading after this much time (msec)
*   Globals:
*       ReadState
*
* OUTPUTS:
*   Parameters:
*       uint8_t*        buf                     bytes read are stored in buffer
*       ssize_t*        bytes_read              number of bytes read and stored in buf
*   Return:
*       type:           ssize_t            
*       values:
*           success     READ_UNTIL
*           failure     READ_FAILURE
*           failure     READ_TIMEOUT
*
*      
* NOTES:
*
*       It is possible that bytes will be read and the function will timeout.
*       Check the bytes_read value to avoid losing bytes on a timeout condition.
*
*/
ReadState serial_read_until(int fd, uint8_t* buf, int buf_max, uint8_t until, 
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
            usleep( 1 * 1000 );             // wait 1 msec, try again
            timeout--;
            if( timeout==0 ){
                return READ_TIMEOUT;
            }

            continue;
        }

        // debug
        //fprintf(stderr, "%s: i = %ld, bytes_read = %ld, b = %x\n", __FUNCTION__, i, *bytes_read, b[0]); 

        buf[i] = b[0];
        i++;
        *bytes_read = i;

    } while( b[0] != until && i < buf_max && timeout > 0 );

    
    return READ_UNTIL;
}


/**
* NAME : ssize_t serial_write(int fd, const uint8_t *buf, size_t count)
*
* DESCRIPTION: writes up to count bytes from the buffer pointed to by buf,
*              to the file pointed to by the file descriptor fd
*
* INPUTS: 
*   Parameters:
*       int             fd                      file descriptor
*       const uint8_t*  buf                     pointer to buffer
*       size_t          count                   number of bytes to write
*   Globals:
*       none
*
* OUTPUTS:
*   Return:
*       type:           ssize_t          
*       values:
*           success     number of bytes written
*           failure     -1 and errno is set appropriately
*               
*  NOTES:
*       number of bytes written may be less than count
*
*
*/
ssize_t serial_write(int fd, const char *buf, size_t numbytes){
   return write(fd, buf, numbytes);

}


/**
* NAME : int serial_close(int fd)
*
* DESCRIPTION: attempts to close file descriptor
*
* INPUTS: 
*   Parameters:
*       int             fd                      file descriptor
*   Globals:
*       none
*
* OUTPUTS:
*   Return:
*       type:           int           
*       values:
*           success     zero
*           failure     -1 and errno is set appropriately
*
*
*/

int serial_close(int fd){
    return close(fd);
}