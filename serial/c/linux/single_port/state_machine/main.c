/****************************************************************
* FILENAME:     main.c
*
* DESCRIPTION:  
*   Program controls communication flow with another program.
*
*   The other program's primary purpose is to send data to 
*   this program. 
*
*   Communication method: serial
*       Default device path:  /dev/ttyACM0
*       Default baud rate:    9600 baud
*       
*   Signal handling is implemented for SIGINT and SIGTERM
*       ctrl+c produces the SIGINT
*
*       Either of these signals breaks out of the infinite
*       while loop and properly closes connections, and 
*       writes any end of program data that should be recorded.       
*
*       signal handling references:
*           http://www.linuxprogrammingblog.com/all-about-linux-signals?page=show
*           http://man7.org/linux/man-pages/man2/select_tut.2.html
*
*
*   Order of operations:
*
*       initialize serial communication
*       register the signal handlers
*       set intial state to WAIT_FOR_CONNECTION
*
*
*       while( no exit request from signal interrupt)
*
*           if in a read state, read available data
*           else if in a write state, write data
*
*           state WAIT_FOR_CONNECTION
*               remain in this state until hello message received
*               transition to SEND_READY_SIGNAL state 
*
*           state SEND_READY_SIGNAL
*               write ready message
*               transition to READ_ACK state
*
*           state READ_ACK
*               remain in this state until acknowledge message received
*               transition to READ_SENSOR state
*
*           state READ_SENSOR
*               read received message
*               upon receiving a complete message
*                   validate sensor id
*                   extra sensor data
*                   process sensor data
*               remain in READ_SENSOR state until a reset or stop condition
*            
*           state SEND_RESET
*           state SEND_STOP
*               neither of these states has been programmed yet
*               they currently serve as a place holder and will
*               be implemented in the future if needed.
*               
*
*       write summary statistics
*       close serial connection
*       
*
* NOTE: Program contains a number of debug messages that are written to
*       the standard error stream. This allows the user to see every step
*       of the program flow to ensure messages are correctly received and
*       indicate all error conditions.
*
*
* PURPOSE: This program is meant as a generic framework for serial communication
*        with another program. This program's purpose is to receive data
*        from the other program and prepare it for further processing.
*
*
* AUTHOR:   Diane Williams        START DATE: 1/4/2018
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
#include <sys/select.h>
#include <sys/time.h>

#include "communication_state.h"
#include "serial.h"


/*============== Global Variable Declarations =============================*/


/** You have to be careful about the fact that access to a single datum is not necessarily atomic. 
    This means that it can take more than one instruction to read or write a single object. 
    In such cases, a signal handler might be invoked in the middle of reading or writing the object.

    To avoid uncertainty about interrupting access to a variable, you can use a particular data 
    type for which access is always atomic: sig_atomic_t. 

    Reading and writing this data type is guaranteed to happen in a single instruction, 
    so there’s no way for a handler to run “in the middle” of an access.

    The type sig_atomic_t is always an integer data type, but which one it is, and how many bits 
    it contains, may vary from machine to machine.

    https://www.gnu.org/software/libc/manual/html_node/Atomic-Types.html#Atomic-Types
*/

static volatile sig_atomic_t exit_request = 0;



/*========================= Function Definitions ==========================*/



/**
* NAME : void signal_handler_term(int sig)
*
* DESCRIPTION: Sets exit request flag to true when SIGINT is received
*              or when SIGTERM is raised
*
* INPUTS: 
*   Parameters:
*       int             sig               signal passed from operating system 
*
* OUTPUTS:
*   Globals:
*       sig_atomic_t    exit_request      flag set to 1 when signal received
*
*   Return:
*       type:           void           
*
*      
* NOTES:
*       Function is not directly invoked. Operating system calls it when
*       the registered signal is received. Signals are software interrupts.
*
*/
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
    struct sigaction saterm;            // SIGTERM raised by this program or another
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
    int sensor_id_mismatch_count = 0;
    int default_comm_read_state_count = 0;
    int default_comm_write_state_count = 0;


            
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
            fprintf(stderr, "\nnumber of read pending, selectrfds: %d\n", selectrfds);

            if(exit_request){
                fprintf(stderr, "received exit request\n");
                break;
            }

            errorCondition = check_select_return_value(selectrfds, errno, &read_select_zero_count);

            // debug 
            fprintf(stderr, "commReadState: %d, %s\n", commReadState, 
                    debug_comm_read_state_string[commReadState]);

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
                                        fprintf(stderr, "Received Ready Signal\n");
                                        commReadState = NO_READ;
                                        commWriteState = SEND_READY_SIGNAL;
                                    }
                                    else{
                                        fprintf(stderr, "warning: function: %s, line: %d, "
                                                "commReadState: %s, expected: %s, ",
                                                __FUNCTION__, __LINE__, 
                                                debug_comm_read_state_string[commReadState],
                                                helloMessage);

                                        fprintf(stderr, "received: ");
                                        print_array_contents_as_hex(responseData, 
                                                                        strlen((char*)responseData));

                                        fprintf(stderr, "received: ");
                                        print_array_contents_as_char((char*)responseData, 
                                                                        strlen((char*)responseData));

                                    }
                                break;

                                case READ_ACK:
                                    // received <ACK>?
                                    if(strcmp((const char*)responseData, ackResponse) == 0){
                                        fprintf(stderr, "Received ACK\n");
                                        commWriteState = NO_WRITE;     // redundant
                                        commReadState = READ_SENSOR;
                                    }
                                    else{
                                        fprintf(stderr, "warning: function: %s, line: %d, "
                                                "commReadState: %s, expected: %s, ",
                                                __FUNCTION__, __LINE__, 
                                                debug_comm_read_state_string[commReadState],
                                                ackResponse);

                                        fprintf(stderr, "received: ");
                                        print_array_contents_as_hex(responseData, 
                                                                        strlen((char*)responseData));

                                        fprintf(stderr, "received: ");
                                        print_array_contents_as_char((char*)responseData, 
                                                                        strlen((char*)responseData));

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
                                        "SENSOR_ID_MISMATCH, commState: %d, expected %d\n"
                                        "not processing responseData: ",
                                        __FUNCTION__, __LINE__, commReadState, sensorId);

                                        print_array_contents_as_hex(responseData, 
                                                                        strlen((char*)responseData));

                                        ++sensor_id_mismatch_count;

                                    }
                                   
                                break;

                            default:
                                fprintf(stderr, "warning: function: %s, line: %d, "
                                        "entered default case, commReadState: %d, "
                                        "not processing responseData: ", 
                                        __FUNCTION__, __LINE__, commReadState );

                                print_array_contents_as_hex(responseData, 
                                                                    strlen((char*)responseData));

                                ++default_comm_read_state_count;
                                        
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

                if(errorCondition == SELECT_ZERO_COUNT){
                    ++read_select_zero_count;
                }
            }
        } // end if read state

        
        if(commWriteState != NO_WRITE){

            FD_ZERO(&writefds);
            FD_SET(fd, &writefds);
            timeout.tv_sec = 2;                     // seconds
            timeout.tv_nsec = 0;                    // nanoseconds

            selectwfds = pselect(max_fd, NULL, &writefds, NULL, &timeout, &empty_mask);
            fprintf(stderr, "\nnumber of write pending, selectwfds: %d\n", selectwfds);

            // debug 
            fprintf(stderr, "commWriteState: %d, %s\n", commWriteState,
                                debug_comm_write_state_string[commWriteState]);

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

                    switch (commWriteState){
                    case SEND_READY_SIGNAL:
                        // The connected program does not rely on receiving the ready
                        // signal to transition to its next state of sending sensor data
                        // No need to resend this signal.
                        // Basically, this is just an example of how to write, and testing
                        // the writing ability. 
                        commReadState = READ_ACK;
                        commWriteState = NO_WRITE;  // message successfully sent
                    break;

                    case SEND_RESET:
                    case SEND_STOP: 
                        // Note: SEND_RESET, SEND_STOP have not yet been programmed
                        fprintf(stderr, "error: function %s, line %d\n", __FUNCTION__,__LINE__);
                        fprintf(stderr, "SEND_STOP, SEND_RESET not yet programmed\n");
                    break;

                    case NO_WRITE: 
                        
                        // not changing state
                    break;
                    
                    default:

                        fprintf(stderr, "warning: function: %s, line: %d, "
                                    "entered default case, commWriteState: %d",
                                    __FUNCTION__, __LINE__, commWriteState );

                    

                        ++default_comm_write_state_count;
                    }
                } 
            }
            else
            {

                fprintf(stderr, "error: function: %s, line: %d, errorState: %s\n",
                        __FUNCTION__, __LINE__, error_condition_string[errorCondition]);

                if(errorCondition == SELECT_FAILURE){
                    fprintf(stderr, "breaking out of while(exit_request) loop\n");
                    break;
            }

            
        }

        }   // end if(commWriteState)
        
    } // end while( !exit_request)

    // write debug values
    fprintf(stderr, "\n\n**** End of Run  *****\n");

    fprintf(stderr, "read select_zero_count: %d, write_select_zero_count: %d\n", 
                        read_select_zero_count, write_select_zero_count);

    fprintf(stderr, "sensor id mismatch count: %d\n", sensor_id_mismatch_count);


    // properly close serial connection
    serial_close(fd);
    fd = -1;


	return 0;
}
