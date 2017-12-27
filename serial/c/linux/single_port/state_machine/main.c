/****************************************************************
* FILENAME:     main.c
*
* DESCRIPTION:  
*   Establishes serial connection to /dev/ttyACM0, 9600 baud
*       
*
*
* AUTHOR:   willydlw        START DATE: 12/24/17
*
* CHANGES:
*
* DATE          WHO        DETAIL
*
*/



#include <stdio.h>      // Standard input/output definitions
#include <stdint.h>     // uint8_t
#include <unistd.h>     // usleep

#include "communication_state.h"
#include "serial.h"


#include <sys/select.h>
#include <sys/time.h>





int main(){

    // serial

	int fd = -1;
    int baudrate = 9600;

	const char* serial_device_path = "/dev/ttyACM0";
	
	ssize_t bytes_read;
	uint8_t buf[16];


    // file selector
    fd_set readfds;
    struct timeval timeout;
    int max_fd;               // largest file descriptor value
    int num_fd_pending;


    int select_zero_count = 0;
    int select_fail_count = 0;

    int loopCount = 0;


    MessageState  receive_message_state = AWAITING_START_MARKER;


    uint16_t sensorData; // eventually this will become an array for multiple sensors

        
    // establish serial connection
	fd = serial_init(serial_device_path, baudrate);

	if(fd == -1){
		puts("serial port did not open");
		return -1;
	}

   
    //send_ready_signal(fd);
    //wait_for_ack(fd);

    

    max_fd = fd + 1;

    

    /*
    int select(int max_fd, fd_set *input, fd_set *output, fd_set *error,
           struct timeval *timeout);

           max_fd specifies highest numbered file descriptor in the input, 
                    output, and error sets

            timeout argument specifies maximum time select should wait 
            before returning. Will return sooner if fd is available

    select returns the number of file descriptors that have a pending condition
        or -1 if there was an error
    */

    FD_ZERO(&readfds);

    while(loopCount < 100){



        /** must call FD_ZERO and FD_SET every time through loop.
        *
        *   When select returns, it has updated the sets to show which file
        *   descriptors are ready for read/write/exception. All other flags
        *   have been cleared. Must call FD_SET to re-enable the file 
        *   descriptors that were cleared.
        */
        FD_ZERO(&readfds);
        FD_SET(fd, &readfds);

        // re-initialize the timeout structure or it will eventually become zero
        // as time is deducted from the data members. timeval struct represents
        // an elapsed time
        timeout.tv_sec = 0;
        timeout.tv_usec = 500000;


        num_fd_pending = select(max_fd, &readfds, NULL, NULL, &timeout);
        fprintf(stderr, "select returned %d\n", num_fd_pending);

        if(FD_ISSET(fd, &readfds)){
            bytes_read = serial_read(fd, buf, 5);
            if(bytes_read > 0){
                receive_message_state = process_received_bytes(receive_message_state, buf, bytes_read,
                                                                &sensorData);
            }
            fprintf(stderr, "\nbytes_read: %ld, bytes: ", bytes_read);
            for(int i = 0; i < bytes_read; ++i){
                fprintf(stderr, "%x ", buf[i] );
            }
            fprintf(stderr, "\n\n");
        }

        if(num_fd_pending > 0){
            
        }
        else if(num_fd_pending == 0){
            ++select_zero_count;
        }
        else{
            ++select_fail_count;
        }

        ++loopCount;
        if(loopCount % 10 == 0){
            fprintf(stderr, "loopCount: %d\n", loopCount);
        }
    }


    fprintf(stderr, "select_zero_count: %d, select_fail_count: %d\n", select_zero_count, select_fail_count);




    serial_close(fd);
    fd = -1;


	return 0;
}
