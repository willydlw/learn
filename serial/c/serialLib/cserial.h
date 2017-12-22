#ifndef CSERIAL_H_INCLUDED
#define CSERIAL_H_INCLUDED


#include <stdint.h>
#include <sys/types.h>


enum SerialErrorConditions { SERIAL_OPEN_ERROR = -1};


int set_baud_speed(int serial_baud_rate);
int initialize_serial(const char *serial_device_name, int baud_rate);
ssize_t read_serial(int serial_port_fd, long int delay_time, uint8_t* readbuffer, size_t numbytes);
ssize_t write_byte(int fd, uint8_t byte);
ssize_t write_buffer(int fd, const char* str);
int close_serial(int fd);


#endif // SERIAL_H_INCLUDED
