/****************************************************************
* FILENAME:     main.c
*
* DESCRIPTION:  
*   Establishes serial connection to /dev/ttyACM0, 9600 baud
*       
*
* signal handling references:
*   http://www.linuxprogrammingblog.com/all-about-linux-signals?page=show
*   http://man7.org/linux/man-pages/man2/select_tut.2.html
*
* AUTHOR:   willydlw        START DATE: 12/24/17
*
* CHANGES:
*
* DATE          WHO        DETAIL
*
*/


#include <errno.h>
#include <signal.h>
#include <stdio.h>      // Standard input/output definitions
#include <stdint.h>     // uint8_t
#include <string.h>     // memset
#include <unistd.h>     // usleep

#include "communication_state.h"
#include "serial.h"


#include <sys/select.h>
#include <sys/time.h>



static volatile sig_atomic_t exit_request = 0;





/* signal handler */
static void signal_handler_term(int sig)
{
     /* psignal used for debugging purposes
       debugging console output lets us know this function was
       triggered. Prints the string message and a string for the
       variable sig
       */
    psignal(sig, "signal_handler_term");
    if(sig == SIGINT || sig == SIGTERM){
        exit_request = 1;
    }
    
}



int main(){

    // signal 
    sigset_t sigmask;
    sigset_t empty_mask;
    

    /** struct sigaction{
    void (*sa_handler)(int);
    void (*sa_sigaction)(int, siginfo_t *, void*);
    sigset_t    sa_mask;
    int     sa_flags;
    void (*sa_restorer)(void);
    }
    */
    struct sigaction saterm;            // SIGTERM 
    struct sigaction saint;             // SIGINT caused by ctrl + c

    // state variable declaration
    MessageState  receive_message_state = AWAITING_START_MARKER;

    // serial
    int fd = -1;                        // file descriptor
    int baudrate = 9600;
    const char* serial_device_path = "/dev/ttyACM0";
    
    ssize_t bytes_read;
    uint8_t buf[16];


    // file descriptor sets
    fd_set readfds;
    fd_set writefds;
   

    struct timespec timeout;
    int max_fd;                     // largest file descriptor value
    int selectrfds;                 // number read file descriptors pending
    int selectwfds;                 // number write file descriptors pending

    
    // received message data
    uint8_t responseData[MESSAGE_LENGTH_BYTES + 1];  // plus 1 for null terminator

    // data received from the sensor
    uint8_t sensorId = 1;
    uint16_t sensorData; 

    
    // read and write states
    CommReadState commReadState = WAIT_FOR_CONNECTION;
    CommWriteState commWriteState = NO_WRITE;

    // select and serial error conditions
    ErrorCondition errorCondition;

    // debug variables 
    int read_select_zero_count = 0;
    int write_select_zero_count = 0;


            
    // initialize serial port connection
	fd = serial_init(serial_device_path, baudrate);

	if(fd == -1){
		puts("serial port did not open");
		return -1;
	}

    // pselect requires an argument that is 1 more than
    // the largest file descriptor value
    max_fd = fd + 1;
         

    // register the SIGTERM signal handler function
    memset(&saterm, 0, sizeof(saterm));
    saterm.sa_handler = signal_handler_term;

    /*  The sigaction() system call is used to change the action 
        taken by a process on receipt of a specific signal.
    */
    if(sigaction(SIGTERM, &saterm, NULL) < 0){
        perror("sigaction saterm ");
        return 1;
    }
    

    // register the SIGINT signal handler function
    memset(&saint, 0, sizeof(saint));
    saint.sa_handler = signal_handler_term;
    if(sigaction(SIGINT, &saint, NULL) < 0){
        perror("sigaction saint");
        return 1;
    }

    // signal mask initialization
    sigemptyset(&sigmask);
    sigemptyset(&empty_mask);

    sigaddset(&sigmask, SIGTERM);
    sigaddset(&sigmask, SIGINT);

    // set as blocking so that pselect can receive event
    if(sigprocmask(SIG_BLOCK, &sigmask, NULL) < 0){
        perror("sigprocmask");
        return 1;
    }


    // main processing loop
    while(!exit_request){

        /*  can get SIGTERM at this point, but it will be delivered while
            in pselect, because SIGTERM is blocked.
        */

        /*
        int pselect(int nfds, fd_set *readfds, fd_set *writefds,
            fd_set *exceptfds, const struct timespec *timeout,
            const sigset_t *sigmask);

                nfds nfds is the highest-numbered file descriptor in any of the 
                    three sets, plus 1.

                timeout argument specifies maximum time select should wait 
                before returning. Will return sooner if fd is available

            pselect returns the number of file descriptors that have a pending condition
            or -1 if there was an error
        */
        

        /** must call FD_ZERO and FD_SET every time through loop.
        *
        *   When select returns, it has updated the sets to show which file
        *   descriptors are ready for read/write/exception. All other flags
        *   have been cleared. Must call FD_SET to re-enable the file 
        *   descriptors that were cleared.
        */
        
        if(commReadState != NO_READ){

            FD_ZERO(&readfds);
            FD_SET(fd, &readfds);
            // re-initialize the timeout structure or it will eventually become zero
            // as time is deducted from the data members. timeval struct represents
            // an elapsed time
            timeout.tv_sec = 2;                     // seconds
            timeout.tv_nsec = 0;                    // nanoseconds

            selectrfds = pselect(max_fd, &readfds, NULL, NULL, &timeout, &empty_mask);
            fprintf(stderr, "number of read pending, selectrfds: %d\n", selectrfds);

            if(exit_request){
                fprintf(stderr, "received exit request\n");
                break;
            }

            errorCondition = check_select_return_value(selectrfds, errno, &read_select_zero_count);

            if(errorCondition == SUCCESS){

                bytes_read = read_message(fd, readfds, buf);

                if(bytes_read > 0){

                    ssize_t bytes_not_processed = bytes_read;

                    while(bytes_not_processed > 0){

                        bytes_not_processed = 
                            process_received_message_bytes(&receive_message_state, 
                                            buf, bytes_not_processed, responseData);

                        if(receive_message_state == MESSAGE_COMPLETE){

                            // reset message state for next read
                            receive_message_state = AWAITING_START_MARKER;

                            switch(commReadState){
                                case WAIT_FOR_CONNECTION:
                                    // received hello?
                                    if(strcmp((const char*)responseData, helloMessage) == 0){
                                        commWriteState = SEND_READY_SIGNAL;
                                        commReadState = READ_SENSOR;
                                    }
                                    else{
                                        fprintf(stderr, "warning: function: %s, line: %d, "
                                                "commReadState: %s, expected: %s, "
                                                "received: %s\n",
                                                __FUNCTION__, __LINE__, 
                                                debug_comm_read_state_string[commReadState],
                                                helloMessage, responseData);
                                    }
                                break;

                                case READ_SENSOR:
                                    // response data should contain sensor data
                                    // verify sensor id
                                    if(responseData[1] == sensorId){
                                         // extract sensor data
                                        // responseData[2] is msb
                                        sensorData = (uint16_t)( 
                                            (((uint16_t)responseData[2]) << 8U) |
                                            ((uint16_t)responseData[1] & 0xFF) );
                                        
                                        // here is where code needs to be added to process 
                                        // the new sensor data
                                        process_sensor_data_received(sensorData);
                                    }
                                    else{
                                        fprintf(stderr, "error: function: %s, line: %d, "
                                        "SENSOR_ID_MISMATCH, commState: %d\n"
                                        "expected %d, received: %d\n"
                                        "not processing responseData, [0] %#x,"
                                        "[1] %#x, [2] %#x, [3] %#x, [4] %#x\n",
                                        __FUNCTION__, __LINE__, commReadState,
                                        sensorId, responseData[1],
                                        responseData[0], responseData[1],
                                        responseData[2], responseData[3], responseData[4]);
                                    }
                                   
                                
                                break;

                            default:
                                fprintf(stderr, "warning: function: %s, line: %d, "
                                        "entered default case, commReadState: %d, "
                                        "not processing responseData, [0] %#x,"
                                        "[1] %#x, [2] %#x, [3] %#x, [4] %#x\n",
                                        __FUNCTION__, __LINE__, commReadState,
                                        responseData[0], responseData[1],
                                        responseData[2], responseData[3], responseData[4]);
                                        
                            } // end switch
                        } // end if receive message state == message complete
                    } // end while(bytes_not_processed)
                } // end if bytes_read > 0
            }
            else{
                fprintf(stderr, "error: function: %s, line: %d, errorState: %s\n",
                        __FUNCTION__, __LINE__, error_condition_string[errorCondition]);
                
                if(errorCondition == SELECT_FAILURE){
                    fprintf(stderr, "breaking out of while(exit_request) loop\n");
                    break;
                }
            }
        } // end if read state

        
        if(commWriteState != NO_WRITE){

            FD_ZERO(&writefds);
            FD_SET(fd, &writefds);
            timeout.tv_sec = 2;                     // seconds
            timeout.tv_nsec = 0;                    // nanoseconds

            selectwfds = pselect(max_fd, NULL, &writefds, NULL, &timeout, &empty_mask);
            fprintf(stderr, "number of write pending, selectwfds: %d\n", selectwfds);

            if(exit_request){
                fprintf(stderr, "received exit request\n");
                break;
            }

            errorCondition = check_select_return_value(selectwfds, errno, &write_select_zero_count);

            if(errorCondition == SUCCESS){

                ErrorCondition writeError;
                writeError = write_message(fd, writefds, commWriteState);

                if(writeError != SUCCESS){
                    fprintf(stderr, "error: function: %s, line: %d, writeError: %s\n",
                        __FUNCTION__, __LINE__, error_condition_string[writeError] );
                }
                else{
                    commWriteState = NO_WRITE;  // message successfully sent
                }
                
            }
            else{

                fprintf(stderr, "error: function: %s, line: %d, errorState: %s\n",
                        __FUNCTION__, __LINE__, error_condition_string[errorCondition]);

                if(errorCondition == SELECT_FAILURE){
                    fprintf(stderr, "breaking out of while(exit_request) loop\n");
                    break;
                }

                switch (commWriteState){
                    case SEND_READY_SIGNAL:
                        // The connected program does not rely on receiving the ready
                        // signal to transition to its next state of sending sensor data
                        // No need to resend this signal.
                        // Basically, this is just an example of how to write, and testing
                        // the writing ability. 
                        commWriteState = NO_WRITE;
                        commReadState = READ_SENSOR;
                        break;

                    case SEND_RESET:
                    case SEND_STOP: 
                    case NO_WRITE: 
                        // not changing state
                        // will attempt to write reset, stop next time through the loop
                    break;
                    
                }
            }

        }   // end if(commWriteState)
        
    } // end while( !exit_request)

    // write debug values
    fprintf(stderr, "\n\n**** End of Run  *****\n");

    fprintf(stderr, "read select_zero_count: %d, write_select_zero_count: %d\n", 
                        read_select_zero_count, write_select_zero_count);


    // properly close serial connection
    serial_close(fd);
    fd = -1;


	return 0;
}
