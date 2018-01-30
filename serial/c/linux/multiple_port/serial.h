/**
 * Copyright (c) 2017 willydlw
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See `main.c` for details.
 */


/**@file serial.h
* 
* @brief Serial communiciation functions for Linux
*
*
* @author willydlw
* @date 30 Jan 2018
* @bugs No known bugs
*
*/

#ifndef SERIAL_H
#define SERIAL_H


#include <stdint.h>					// uint8_t
#include <termios.h>				// speed_t
#include <sys/types.h>				// ssize_t



/**
* @brief Serial read states
*
*/
typedef enum read_state_t {	READ_UNTIL = 0, 		/**< successfully read until desired byte */
							READ_FAILURE = -1, 		/**< failed to read any bytes */
							READ_TIMEOUT = -2		/**< timed out before reading all bytes */
} SerialReadState;




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
*
* @param[in] serial_device_name     	serial device path name
*                                       example: "/dev/ttyUSB0"
*
* @param[in] baud_rate                  baud rate 
*
* @return success     serial port file descriptor.
*         failure     -1
*
*/
int serial_init(const char *serial_device_name, int baud_rate);



/**
* @brief returns corresponding baud speed for baud rate
*
* @param[in]         baud_rate               baud rate (bps) 
* 
* @return success     baud speed
*         failure     -1 when parameter baud_rate is not a 
*					  recognized speed
*
* @notes Supported Baud Rates
*		 9600, 19200, 38400, 57600, 115200
*/
speed_t  set_baud_speed(int baud_rate);



/**
* NAME : ssize_t serial_read(int fd, uint8_t *buf, size_t count)
*
* @brief Attempts to read up to count bytes and store in the buffer,
*        starting at buf
*
* @param[in]	fd                      file descriptor
* @param[in]	count                   max number of bytes to read 
*
* @param[out]	buf                     bytes read are stored in buffer
* 
* @return 	success     number of bytes read
*           failure     -1 and errno is set appropriately
*       
*
*/
ssize_t serial_read(int fd, uint8_t *buf, size_t count);



/**
* @brief Attempts to read bytes from the file descriptor
*        into the buffer starting at buf.
*
*        Stops after reading and storing the until byte.
*        Stops after reading buf_max bytes.
*        Stops after timeout.
*
*  
* @param[in]	fd                      file descriptor
* @param[in]    buf_max                 maximum bytes to read
* @param[in]    until                   byte that ends read
* @param[in]    timeout                 stop reading after this much time (msec)
*
* @param[out]	buf                     bytes read are stored in buffer
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
                            int timeout, ssize_t *bytes_read);



/**
* @brief Writes up to count bytes from the buffer pointed to by buf,
*        to the file pointed to by the file descriptor fd
*
* @param[in] 	fd                      file descriptor
* @param[in]  	buf                     pointer to buffer
* @param[in]    count                   number of bytes to write
*
* @return success     number of bytes written
*         failure     -1 and errno is set appropriately
*               
* @notes
*       number of bytes written may be less than count
*
*
*/
ssize_t serial_write(int fd, const char *buf, size_t numbytes);



/**
* @brief Closes serial connection
*
* @param[in]            fd                      file descriptor
*
* @return success     zero
*         failure     -1 and errno is set appropriately
*
*/
int serial_close(int fd);

#endif