#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>

#include "serial.h"
#include "communication_state.h"


// Globals must be initialized in the c file and declared extern in the h file
const char* message_state_string[] = 
		{ "AWAITING_START_MARKER", "AWAITING_SENSOR_ID", "AWAITING_DATA_BYTE_ONE",
		  "AWAITING_DATA_BYTE_TWO", "AWAITING_DATA_BYTE_THREE", 
		  "AWAITING_END_MARKER", "MESSAGE_COMPLETE"};


/** Note: The messages and commands are in common with the 
          paired Arduino program. These would be easier to 
          maintain if they were in a common file. 

          Currently, using the Arduino IDE requires that the
          file be in Arduino/libraries. As these are testing
          files, and not yet production, the data is maintained
          here and in sensor_coummunication.h
*/

// request messages
const char* readyCommand = "<RDY>";
const char* resetCommand = "<RST>";
const char* stopCommand = "<STP>";

// response messages
const char* ackResponse = "<ACK>";
const char* nackResponse = "<NCK>";


/** When another program opens the serial
    connection, the Arduino program resets.

    This message is broadcast to confirm
    the connection has been made.
**/
const char* helloMessage = "<HLO>";


static const uint8_t start_marker = '<';
static const uint8_t end_marker = '>';


// end of data in common with Arduino

static const uint8_t sensor_id[1] = {'1'};


const char* debug_comm_state_string[] = 
	{	"UNUSED", "WAIT_FOR_CONNECTION", "SEND_READY_SIGNAL", "READ_SENSOR", 
		"SEND_RESET",  "SEND_STOP"};

const char* error_state_string[] { 
	"SUCCESS", "SELECT_FAILURE", "SELECT_ZERO_COUNT", 
	"SERIAL_WRITE_ERROR", "FD_ISSET_ERROR"  };


ErrorCondition check_select_return_value(int selectfds, int errnum, int *zeroCount)
{
	if(selectfds < 0 && errnum != EINTR){   // EINTR means signal was caught
        fprintf(stderr, "error: function %s, select failure, errno: %s",
        			__FUNCTION__, strerror(errnum));
        /* Errors:  
            EBADF - invalid file descriptor. Possible fd closed, or in error state
            EINTR - signal was caught
            EINVAL - nfds is negative or timeout value is invalid
            ENOMEM - unable to allocate memory for internal tables.
        */
        return SELECT_FAILURE;
     }
    else if(selectfds == 0){
    	*zeroCount = *zeroCount + 1;
    	return SELECT_ZERO_COUNT;
    }

    return SUCCESS;
}


int readMessage(int fd, fd_set readfds, uint8_t *buf)
{
	// since this function will be called numerous times, making 
	// bytes_read static will save time popping it on/off stack
	// will use less stack space
	static int bytes_read;

	if(FD_ISSET(fd, &readfds)){
        // restricting to read maximum of the length of a complete
        // message. 
        bytes_read = serial_read(fd, buf, MESSAGE_LENGTH_BYTES );

        // debug
        fprintf(stderr, "bytes_read: %ld, bytes: ", bytes_read);
        for(int i = 0; i < bytes_read; ++i){
            fprintf(stderr, "%#x ", buf[i] );
        }
        fprintf(stderr, "\n\n");

        // end debug

        return bytes_read;
    }
            
}





ErrorCondition write_message(int fd, int writefds, CommState* commState)
{        	
    if(FD_ISSET(fd, &writefds)){
   		
   		switch(*commState){
   			case SEND_READY_SIGNAL:
   				return send_ready_signal(fd, 1);
   			break;
   		}
    }
    else{
    	fprintf(stderr, "FD_ISSET(fd, &writefds) not true for fd %d\n", fd);
    	return FD_ISSET_ERROR;
    }
	

	// returning 0. If function returns above due to an error, all errno values are
	// greater than 0
	return SUCCESS;
}


ErrorCondition send_ready_signal(int fd, int max_tries){

	ssize_t expected_bytes = (ssize_t)strlen(comm_state_string[SEND_READY_QUERY]);

    ssize_t bytes_written = 0;
    int num_tries = 0;

    bytes_written = serial_write(fd, comm_state_string[SEND_READY_QUERY], 
    								expected_bytes) ;
    ++num_tries;

    // keep sending until all bytes have been transmitted
    while( bytes_written != expected_bytes && num_tries <= max_tries){
    	fprintf(stderr, "error, function %s, did not write all bytes\n", __FUNCTION__);
        fprintf(stderr, "bytes_written: %ld, should have written %ld bytes\n", 
        				bytes_written, expected_bytes);

        bytes_written = serial_write(fd, comm_state_string[SEND_READY_QUERY], 
        								expected_bytes) ;
        ++num_tries;
    }

    if(bytes_written == expected_bytes){
    	return SUCCESS;		// message successfully transmitted
    }
    else{
    	return SERIAL_WRITE_ERROR;		// message not successfully transmitted
    }

}




ssize_t process_received_message_bytes(MessageState *msgState, const uint8_t *buf, 
											ssize_t bytes_read, uint8_t *responseData)
{
	
	fprintf(stderr, "\nstart of %s, message state: %s\n", __FUNCTION__, 
						message_state_string[msgState]);

	ssize_t i;
	
	for(i = 0; i < bytes_read; ++i){
		fprintf(stderr, "i: %ld, message state: %s, buf[i]: %#x, (char)buf[i]: %c\n", 
							i, message_state_string[msgState], buf[i], (char)buf[i]);

		switch(*msgState){
			case AWAITING_START_MARKER:
				if((char)buf[i] == start_marker){
					memset(responseData, '\0', (MESSAGE_LENGTH_BYTES+1)*sizeof(uint8_t) );
					responseData[0] = buf[i];
					*msgState = AWAITING_DATA_BYTE_ONE;
				}
				else { // log error
					fprintf(stderr, "error: %s, state: AWAITING_START_MARKER, "
									" buf[%ld]: %#x\n", __FUNCTION__, i, buf[i]);
				}
				break;
			case AWAITING_DATA_BYTE_ONE:
				responseData[1] = buf[i];
				*msgState = AWAITING_DATA_BYTE_TWO;
				break;
			case AWAITING_DATA_BYTE_TWO:
				responseData[2] = buf[i];
				*msgState = AWAITING_DATA_BYTE_THREE;
				break;
			case AWAITING_DATA_BYTE_THREE:
				responseData[3] = buf[i];
				*msgState = AWAITING_END_MARKER;
				break;
			case AWAITING_END_MARKER:
				responseData[4] = buf[i];
				responseData[5] = '\0';     // null terminate the string
				*msgState = MESSAGE_COMPLETE;
				return (bytes_read - (i+1));	// bytes not processed
			default:
				fprintf(stderr, "error: %s, reached default case, msgState: %#x,"
						" message_state_string: %s\n", __FUNCTION__, msgState, 
						msgState < NUM_MESSAGE_STATES? message_state_string[msgState]:"unknown");
		}
	}
	return msgState;
}


	


bool valid_sensor_id(uint8_t id){
	// @TODO: need a scheme for quickly validating sensor ids
	//        once multiple sensors are handled
	if(id == sensor_id[0])
		return true;
	else 
		return false;
}


void process_data_received(uint16_t theData)
{
	fprintf(stderr, "\nentering %s, theData: %u\n", __FUNCTION__, theData);
	fprintf(stderr, "TODO: this is only a stub function, need to determine how to handle"
					" real time data updates\n");
}
