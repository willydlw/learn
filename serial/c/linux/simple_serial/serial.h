#ifndef SERIAL_H
#define SERIAL_H


#include <stdint.h>					// uint8_t
#include <sys/types.h>				// ssize_t


typedef enum read_state_t {READ_UNTIL = 0, READ_FAILURE = -1, READ_TIMEOUT = -2} ReadState;

int serial_init(const char *serial_device_name, int baud_rate);
int  set_baud_speed(int baud_rate);

ssize_t serial_read(int fd, void *buf, size_t count);

ReadState serial_read_until(int fd, uint8_t* buf, int buf_max, uint8_t until, 
                            int timeout, ssize_t *bytes_read);


#endif