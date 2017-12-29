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




static volatile int exit_request = 0;


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
    sigset_t mask;
    sigset_t orig_mask;
    


    /** struct sigaction{
    void (*sa_handler)(int);
    void (*sa_sigaction)(int, siginfo_t *, void*);
    sigset_t    sa_mask;
    int     sa_flags;
    void (*sa_restorer)(void);
    }
    */
    struct sigaction saterm;

    struct sigaction saint;             // SIGINT caused by ctrl + c

    // register the signal handler function
    memset(&saterm, 0, sizeof(saterm));
    saterm.sa_handler = signal_handler_term;

    /*  The sigaction() system call is used to change the action taken by a
       process on receipt of a specific signal.
    */
    if(sigaction(SIGTERM, &saterm, NULL) < 0){
        perror("sigaction saterm ");
        return 1;
    }
    

    memset(&saint, 0, sizeof(saint));
    saint.sa_handler = signal_handler_term;
    if(sigaction(SIGINT, &saint, NULL) < 0){
        perror("sigaction saint");
        return 1;
    }


    sigemptyset(&mask);
    sigaddset(&mask, SIGTERM);
    sigaddset(&mask, SIGINT);

    // set as blocking so that pselect can receive event
    if(sigprocmask(SIG_BLOCK, &mask, &orig_mask) < 0){
        perror("sigprocmask");
        return 1;
    }


    // serial

	int fd = -1;
    int baudrate = 9600;

	const char* serial_device_path = "/dev/ttyACM0";
	
	ssize_t bytes_read;
	uint8_t buf[16];


    // file selector
    fd_set readfds;
    struct timespec timeout;
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
  

    while(!exit_request){

        /*  can get SIGTERM at this poin, but it will be delivered while
            in pselect, because SIGTERM is blocked.
        */


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
        timeout.tv_sec = 2;                     // seconds
        timeout.tv_nsec = 0;                    // nanoseconds




        /*
        int pselect(int nfds, fd_set *readfds, fd_set *writefds,
            fd_set *exceptfds, const struct timespec *timeout,
            const sigset_t *sigmask);

                nfds nfds is the highest-numbered file descriptor in any of the 
                    three sets, plus 1.

                timeout argument specifies maximum time select should wait 
                before returning. Will return sooner if fd is available

        select returns the number of file descriptors that have a pending condition
            or -1 if there was an error
        */
        num_fd_pending = pselect(max_fd, &readfds, NULL, NULL, &timeout, &orig_mask);
        fprintf(stderr, "\n\n%s, select returned %d\n", __FUNCTION__, num_fd_pending);

        if(num_fd_pending < 0 && errno != EINTR){   // EINTR means signal was caught
            perror("pselect");
            ++select_fail_count;
            // should program end here? Don't think we can recover
            /* Errors:  
                EBADF - invalid file descriptor. Possible fd closed, or in error state
                EINTR - signal was caught
                EINVAL - nfds is negative or timeout value is invalid
                ENOMEM - unable to allocate memory for internal tables.
            */
            break;
        }
        else if(exit_request){
            fprintf(stderr, "received exit request\n");
            break;
        }
        else if(num_fd_pending == 0){
             ++select_zero_count;
             continue;
        }

        if(FD_ISSET(fd, &readfds)){

            // restricting to read 5 bytes at a time, the length of a complete
            // message. 
            bytes_read = serial_read(fd, buf, 5);

            // debug
            fprintf(stderr, "bytes_read: %ld, bytes: ", bytes_read);
            for(int i = 0; i < bytes_read; ++i){
                fprintf(stderr, "%#x ", buf[i] );
            }
            fprintf(stderr, "\n\n");

            // end debug

            if(bytes_read > 0){
                receive_message_state = process_received_bytes(receive_message_state, buf, bytes_read,
                                                                &sensorData);
            }
            
        }


        ++loopCount;
        if(loopCount % 5 == 0){
            fprintf(stderr, "loopCount: %d\n", loopCount);
            raise(SIGTERM);  // for testing SIGTERM
        }
    }

    fprintf(stderr, "\n\n**** End of Run  *****\n");

    fprintf(stderr, "select_zero_count: %d, select_fail_count: %d\n", select_zero_count, select_fail_count);




    serial_close(fd);
    fd = -1;


	return 0;
}