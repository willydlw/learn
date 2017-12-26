#include <stdio.h>
#include <string.h>


#include "serial.h"
#include "communication_state.h"

void send_ready_signal(int fd){

	const static ssize_t expected_bytes = (ssize_t)strlen(comm_state_string[SIGNAL_READY]);

    ssize_t bytes_written = 0;

    bytes_written = serial_write(fd, comm_state_string[SIGNAL_READY], 
    								expected_bytes) ;

    // keep sending until all bytes have been transmitted
    while( bytes_written!= expected_bytes){
        perror("send_ready_signal: not all bytes written ");
        fprintf(stderr, "bytes_written: %ld\n", bytes_written);
        bytes_written = serial_write(fd, comm_state_string[SIGNAL_READY], 
        								expected_bytes) ;
    }

}
