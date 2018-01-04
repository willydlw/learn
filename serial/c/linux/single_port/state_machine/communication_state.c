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



const char* debug_comm_read_state_string[] = 
	{	"NO_READ", "WAIT_FOR_CONNECTION", "READ_ACK", "READ_SENSOR" };


const char* debug_comm_write_state_string[] = 
	{	"NO_WRITE", "SEND_READY_SIGNAL", "SEND_RESET",  "SEND_STOP"};



const char* error_condition_string[] { 
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


ssize_t read_message(int fd, fd_set readfds, uint8_t *buf)
{
	// since this function will be called numerous times, making 
	// bytes_read static will save time popping it on/off stack
	// will use less stack space
	static ssize_t bytes_read = 0;

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
        
    }

    return bytes_read;
            
}



ErrorCondition write_message(int fd, fd_set writefds, CommWriteState commWriteState)
{        	
    if(FD_ISSET(fd, &writefds)){

    	ssize_t bytes_written;
    	ssize_t expected_bytes;
   		
   		switch(commWriteState){
   			case SEND_READY_SIGNAL:
   				expected_bytes = (ssize_t)strlen(readyCommand);
   				bytes_written = serial_write(fd, readyCommand, expected_bytes);
   				
   			break;

   			case SEND_RESET:
   				expected_bytes = (ssize_t)strlen(resetCommand);
   				bytes_written = serial_write(fd, resetCommand, expected_bytes);
   				
   			break;

   			case SEND_STOP:
   				expected_bytes = (ssize_t)strlen(resetCommand);
   				bytes_written = serial_write(fd, resetCommand, expected_bytes);
   			break;
   			default:
   				fprintf(stderr, "error, function: %s, line: %d, reached default case, "
   						"commWriteState: %#x\n", __FUNCTION__, __LINE__ , 
   						(unsigned int)commWriteState);
   		}

   		return expected_bytes == bytes_written? SUCCESS:SERIAL_WRITE_ERROR;
    }
    else{
    	fprintf(stderr, "FD_ISSET(fd, &writefds) not true for fd %d\n", fd);
    	return FD_ISSET_ERROR;
    }
    
}






ssize_t process_received_message_bytes(MessageState *msgState, const uint8_t *buf, 
											ssize_t bytes_read, uint8_t *responseData)
{
	
	fprintf(stderr, "\nstart of %s, message state: %s\n", __FUNCTION__, 
						message_state_string[*msgState]);

	ssize_t i;
	ssize_t bytes_remaining;
	
	for(i = 0, bytes_remaining = bytes_read; i < bytes_read; ++i){

		fprintf(stderr, "i: %ld, message state: %25s, buf[i]: %#4x, (char)buf[i]: %c\n", 
							i, message_state_string[*msgState], buf[i], (char)buf[i]);

		--bytes_remaining;

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
				return bytes_remaining;	// bytes not processed
			default:
				fprintf(stderr, "error: %s, reached default case, msgState: %#x,"
						" message_state_string: %s\n", __FUNCTION__, 
						(unsigned int) *msgState, 
						*msgState < NUM_MESSAGE_STATES? message_state_string[*msgState]:"unknown");
				
				// set message state to await start of new message
				// throw away the unkown message bytes
				*msgState = AWAITING_START_MARKER;
		}
	}
	return bytes_remaining;
}


	


bool valid_sensor_id(uint8_t id){
	// @TODO: need a scheme for quickly validating sensor ids
	//        once multiple sensors are handled
	if(id == sensor_id[0])
		return true;
	else 
		return false;
}


void process_sensor_data_received(uint16_t theData)
{
	fprintf(stderr, "\nentering %s, theData: %u\n", __FUNCTION__, theData);
	fprintf(stderr, "TODO: this is only a stub function, need to determine how to handle"
					" real time data updates\n");
}


void print_array_contents_as_hex(uint8_t* buf, ssize_t buflength)
{
	ssize_t i;
	for(i = 0; i < buflength-1; ++i){
		fprintf(stderr, "buf[%ld] %#x,", i, buf[i]);
	}

	fprintf(stderr, "buf[%ld] %#x\n", i, buf[i]);
}

void print_array_contents_as_char(char* buf, ssize_t buflength)
{
	ssize_t i;
	for(i = 0; i < buflength-1; ++i){
		fprintf(stderr, "buf[%ld] %c,", i, buf[i]);
	}

	fprintf(stderr, "buf[%ld] %c\n", i, buf[i]);
}
