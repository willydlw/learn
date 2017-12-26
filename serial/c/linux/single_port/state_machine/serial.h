/****************************************************************
* FILENAME:     serial.h
*
* DESCRIPTION:
*		Serial communiciation functions for Linux
*
*
* AUTHOR:   willydlw        START DATE: 12/24/17
*
* CHANGES:
*
* DATE          WHO        DETAIL
*
*/

#ifndef SERIAL_H
#define SERIAL_H


#include <stdint.h>					// uint8_t
#include <sys/types.h>				// ssize_t



/******     GLOBAL   *************/

typedef enum read_state_t {	READ_UNTIL = 0, 
							READ_FAILURE = -1, 
							READ_TIMEOUT = -2} ReadState;



/*******                 Function Prototypes              ******/


/**
* NAME : int serial_init(const char *serial_device_name, int baud_rate)
*
* DESCRIPTION: 
*		Establishes serial port connection to serial_device_name, at
*       the specified baud rate.
*
*       8 data bits, no parity, one stop bit
*       raw mode, so binary data can be sent/received
*
*       Non-blocking mode. No mimimum characters to read. Zero time
*       waiting for data.
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
* NOTES:
*       Error messages are sent to stderr stream
*
*/
int serial_init(const char *serial_device_name, int baud_rate);



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
*           failure     -1 when parameter baud_rate is not a 
*						recognized speed
*
*      
* NOTES:
*       Error messages are sent to stderr stream
*
*/
int  set_baud_speed(int baud_rate);


/**
* NAME : ssize_t serial_read(int fd, uint8_t *buf, size_t count)
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
*/
ssize_t serial_read(int fd, uint8_t *buf, size_t count);




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
                            int timeout, ssize_t *bytes_read);



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
ssize_t serial_write(int fd, const char *buf, size_t numbytes);


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

int serial_close(int fd);

#endif