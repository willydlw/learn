

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <termios.h>		// speed_t
#include <sys/time.h>

#include "serial.h"
#include "communication_state.h"


/* ========== Initialize External Global Constants     ========= */

const char* debug_operational_state_string[] =
{
	"WAIT_FOR_CONNECTION" ,
	"ACKNOWLEDGE_CONNECTION",
	"WAIT_FOR_SENSOR_ID",
	"SENSOR_REGISTRATION_COMPLETE",
	"RECEIVE_SENSOR_DATA",
	"NOT_OPERATIONAL"
};


const char* debug_read_write_message_state_string[] = 
		{ "START_MARKER", "DATA_BYTE_ONE",
		  "DATA_BYTE_TWO", "DATA BYTE THREE",
		  "END_MARKER"};



const char* debug_comm_read_state_string[] = 
	{	"NO_READ", "WAIT_FOR_CONNECTION", "READ_ACK", "READ_SENSOR" };



const char* debug_comm_write_state_string[] = 
	{	"NO_WRITE", "SEND_READY_SIGNAL", "SEND_RESET",  "SEND_STOP"};



const char* debug_error_condition_string[] = { 
	"SUCCESS", "SELECT_FAILURE", "SELECT_ZERO_COUNT", 
	"SERIAL_WRITE_ERROR", "FD_ISSET_ERROR"  };


/** Note: The messages and commands are in common with the 
          paired Arduino programs. These would be easier to 
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
const char* helloMessageHEX = "3C 48 4C 4F 3E";


// message markers
const uint8_t start_marker = '<';
const uint8_t end_marker = '>';







/**
* NAME : ErrorCondition check_select_return_value(int selectfds, int errnum, int *zeroCount)
*
* DESCRIPTION: Checks value of selectfds and returns error state.
*              In the case of a select failure error, an error message is 
*			   written to stderr, including the string describing errnum.
*
* INPUTS: 
*   Parameters:
*       int             selectfds		  pselect return value, number of file descriptors 
*	    int 			errnum            errno generated by an error condition
*	    
*
* OUTPUTS:
*		int*			zeroCount         pointer to counter that totals how many times
*	    								  pselect returns zero
*   Return:
*       type:			ErrorCondition
*
*		returns SUCCESS when selectfds > 0
*		returns SELECT_ZERO_COUNT when selectfds equals zero, adds 1 to zeroCount
*       return SELECT_FAILURE 	when selectfds < 0
*
*      
* NOTES:
*       
*
*/

ErrorCondition check_select_return_value(int selectfds, int errnum, int *zeroCount)
{
	if(selectfds < 0 && errnum != EINTR){   // EINTR means signal was caught
		
		log_error("select failure, errno: %s", strerror(errnum));
        
        /* Errors:  
            EBADF - invalid file descriptor. Possible fd closed, or in error state
            EINTR - signal was caught
            EINVAL - nfds is negative or timeout value is invalid
            ENOMEM - unable to allocate memory for internal tables.
        */
        return SELECT_FAILURE;
     }
    else if(selectfds == 0){
    	// in the long run, may not need to maintain this stat
        // there may be times when no data has been received
        // nor is there data to write, depends on timing of this loop
        // With a two second timeout, it is unlikely that there will
        // never be sensor data to read when the operational state
        // is RECEIVE_SENSOR_DATA
    	log_debug("select returned zero");
    	*zeroCount = *zeroCount + 1;
    	return SELECT_ZERO_COUNT;
    }

    return SUCCESS;
}

/**
* NAME : ssize_t read_message(int fd, fd_set readfds, uint8_t *buf)
*
* DESCRIPTION: Reads data bytes from the serial file descriptor fd.
*              A maximum of MESSAGE_LENGTH_BYTES are read and stored in buf.
*
*              
* INPUTS: 
*   Parameters:
*       int             fd		  		  file descriptor
*	    fd_set          readfds			  read file descriptor set
*	    
*
* OUTPUTS:
*		uint8_t*		buf 			  buffer which stores bytes read
*	    								  
*   Return:
*       type:			ssize_t
*
*		returns bytes read
*      
* NOTES:
*       
*
*/
ssize_t read_message(int fd, fd_set readfds, uint8_t *buf)
{

	ssize_t bytes_read = 0;

	if(FD_ISSET(fd, &readfds)){

        // restricting to read maximum of the length of a complete
        // message. 
        bytes_read = serial_read(fd, buf, READ_MESSAGE_LENGTH_BYTES );

        for(int j = 0; j < bytes_read; ++j){
            log_trace("buf[%d] %hhu", j, buf[j]);
        }

        // debug
        char debugBuffer[3*bytes_read + 1];
        convert_array_to_hex_string(debugBuffer, 3*bytes_read+1, buf, bytes_read);

        log_trace("bytes_read: %ld, debugBuffer: %s\n", bytes_read, debugBuffer);

        // end debug
        
    }
    else{
    	log_debug("FD_ISSET(fd, &readfds) not true for fd %d", fd);
    }

    return bytes_read;
            
}


/**
* NAME : ErrorCondition write_message(int fd, fd_set writefds, CommWriteState commWriteState)
*
* DESCRIPTION: Transmits a message, dependent on commWriteState
*              
*
*              
* INPUTS: 
*   Parameters:
*       int             fd		  		  file descriptor
*	    fd_set          writefds		  write file descriptor set
*	    CommWriteState  CommWriteState    communication write state
*
* OUTPUTS:
*	    								  
*   Return:
*       type:			ErrorCondition
*
*		returns SUCCESS				when all message bytes are transmitted
*
*		returns SERIAL_WRITE_ERROR 	when the fd is available, but not
*								   	all bytes are transitted
*
*		returns FD_ISSET_ERROR 		when the file descriptor is not available 
*							   		in the write set 
*      
* NOTES:
*       
*
*/
ssize_t write_message(int fd, fd_set writefds, uint8_t *buf, size_t numBytes)
{
	ssize_t bytesWritten = 0;

    if(FD_ISSET(fd, &writefds)){

    	bytesWritten = serial_write(fd, (const char*)buf, numBytes);
   		   		
    }
    else{
    	log_warn("FD_ISSET(fd, &writefds) not true for fd %d", fd);
    }

    return bytesWritten;
    
}






void process_sensor_data_received(uint16_t theData)
{
	log_trace("theData: %u", theData);	
	log_trace("TODO: this is only a stub function, need to determine how to handle"
					" real time data updates\n");
}


void convert_array_to_hex_string(char* destination, ssize_t dlength, const uint8_t* source, ssize_t slength)
{
	ssize_t i;
	memset(destination, 0, (size_t)dlength);
	for(i = 0; i < slength; ++i){
		sprintf(&destination[i*3], "%02x ", source[i]); 
	}
}


/*
void process_read_state_error_message(CommReadState commReadState, 
	const uint8_t *responseData, ssize_t rlength)
{
	//char hexmsg[3*MESSAGE_LENGTH_BYTES + 1];
	char hexmsg[3*rlength+1];

    convert_array_to_hex_string(hexmsg, 
        3*rlength + 1, 
            responseData, rlength);

    log_warn("commReadState: %s\n"
             "\t\thexadecimal expected: %s, received: %s\n"
             "\t\tcharacters  expected: %s, received: %s",
            debug_comm_read_state_string[commReadState],
            helloMessageHEX, hexmsg,
            helloMessage, (const char*)responseData);
}

*/