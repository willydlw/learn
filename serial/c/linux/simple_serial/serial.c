#include "serial.h"



/**
* NAME : int initialize_serial(const char *serial_device_name, int baud_rate)
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
int initialize_serial(const char *serial_device_name, int baud_rate)
{
    int serial_port_fd;
    int bspeed; 
    struct termios terminalSettings;

    // Open the serial port nonblocking (read returns immediately)
    serial_port_fd = open(serial_device_name, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if(serial_port_fd < 0)             // open returns -1 on error
    {
        fprintf(stderr, "Error: %s, serial port not open, errno -%s\n", __FUNCTION__, strerror(errno));
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
        cfsetispeed(&terminalSettings, brate);
        cfsetospeed(&terminalSettings, brate);
    }
    else{
        return -1;
    }


    // set control flags
    terminalSettings.c_cflag &= ~PARENB;            // no parity
    terminalSettings.c_cflag &= ~CSTOPB;            // 1 stop bit
    terminalSettings.c_cflag &= ~CSIZE;             // reset character size bits
    terminalSettings.c_cflag |= CS8;                // data size 8 bit
  
    terminalSettings.c_cflag &= ~CRTSCTS;           // no flow control

    /* CREAD - enable receiver
       CLOCAL should be enabled to ensure that your program does not become 
       the 'owner' of the port subject to sporatic job control and hangup signals
    */
    terminalSettings.c_cflag |= CREAD | CLOCAL;     // enable receiver, ignore ctrl lines

    /** turn off software flow control (outgoing, incoming)
        allow any character to start flow again
    */
    terminalSettings.c_iflag &= ~(IXON | IXOFF | IXANY);    


    // configure for raw input
    terminalSettings.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); 

    // configure for raw output
    terminalSettings.c_oflag &= ~OPOST; 

    // VMIN sets minimum characters to read
    terminalSettings.c_cc[VMIN]  = 0;

    // time to wait for data (tenths of seconds)
    terminalSettings.c_cc[VTIME] = 0;

    // flush data from the lines
    tcflush(serial_port_fd, TCIOFLUSH);

    // Load new settings
    if( tcsetattr(serial_port_fd, TCSAFLUSH, &terminalSettings) < 0){
        perror("init_serialport: Couldn't set term attributes");
        return -1;
    }

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
*           failure     -1
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
        fprintf(stderr, "error: %s, baud rate %d not supported\n", __FUNCTION__, baud_rate);
        return -1;
    }
}