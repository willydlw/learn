#ifndef SERIAL_H_INCLUDED
#define SERIAL_H_INCLUDED

#include <stdint.h>


// Serial Port Settings
#define DEVICE_SERIAL_PORT     "/dev/ttyUSB0"
#define DEVICE_BAUDRATE        9600


int  set_baud_speed(int serial_baud_rate);

int init_serial(const char * serial_device_name, int baud_rate);

void write_serial_byte(int serial_file_descriptor, uint8_t byte);

void shutdown(int serial_file_descriptor);

#endif // SERIAL_H_INCLUDED
