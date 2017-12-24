#ifndef SERIAL_H
#define SERIAL_H

int initialize_serial(const char *serial_device_name, int baud_rate);
int  set_baud_speed(int baud_rate);


#endif