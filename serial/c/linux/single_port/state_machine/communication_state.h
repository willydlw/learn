/****************************************************************
* FILENAME:     communication_state.h
*
* DESCRIPTION:
*       Finite state machine definitions for communication
*       with another program.
*
*
*
*
* AUTHOR:   Diane Williams        START DATE: 1/4/18
*
* CHANGES:
*
* DATE          WHO        DETAIL
*
*/

#ifndef COMMUNICATION_STATE_H
#define COMMUNICATION_STATE_H


#include <sys/select.h>					// fd_set

/* ==========     Preprocessor Directives     ==========*/

#define NUM_MESSAGE_STATES 7			// number of message states definec in enum MessageState

#define MESSAGE_LENGTH_BYTES 5			// every message received will be this length 



/* ==========     External Global Variables     ========== */


// external globals must be initialized in the c file 
// and declared extern in the h file


// string arrays used for debugging
// states are numeric, when writing an error message
// displaying a string provides more meaningful information

extern const char* debug_comm_read_state_string[];
extern const char* debug_comm_write_state_string[];

extern const char* debug_message_state_string[];
extern const char* debug_error_condition_string[];


// command messages transmitted to other program
extern const char* readyCommand;
extern const char* resetCommand;
extern const char* stopCommand;

// response messages received from other program
extern const char* ackResponse;					// acknowledge message received, command implemented
extern const char* nackResponse;				// nack - not acknowledge
extern const char* helloMessage;				// confirms connection



// communication states

typedef enum comm_read_state_t { 
	NO_READ = 0,
	WAIT_FOR_CONNECTION = 1,
	READ_ACK = 2,
	READ_SENSOR = 3 
}CommReadState;

typedef enum comm_write_state_t { 
	NO_WRITE = 0,
	SEND_READY_SIGNAL = 1,
	SEND_RESET = 2, 
	SEND_STOP = 3
}CommWriteState;


/*  messages are received byte by byte. As each byte is recevied,
    the message state is changed to reflect which bytes have
    been received, and which byte is expected next.
*/
typedef enum message_state_t { 
	AWAITING_START_MARKER, AWAITING_SENSOR_ID, AWAITING_DATA_BYTE_ONE,
	AWAITING_DATA_BYTE_TWO, AWAITING_DATA_BYTE_THREE,
	AWAITING_END_MARKER, MESSAGE_COMPLETE
} MessageState;


/*  error conditions help indicate exact sources of failure */
typedef enum error_conditions_t { 
	SUCCESS, SELECT_FAILURE, SELECT_ZERO_COUNT, 
	SERIAL_WRITE_ERROR, FD_ISSET_ERROR
} ErrorCondition;



/* ==========     Function Prototypes     ========= */




ErrorCondition check_select_return_value(int selectfds, int errnum, int *zeroCount);

ssize_t read_message(int fd, fd_set readfds, uint8_t *buf);

ssize_t process_received_message_bytes(MessageState *msgState, const uint8_t *buf, 
											ssize_t bytes_read, uint8_t *responseData);


ErrorCondition write_message(int fd, fd_set writefds, CommWriteState commWriteState);


bool valid_sensor_id(uint8_t id);


void process_sensor_data_received(uint16_t theData);

void print_array_contents_as_hex(uint8_t* buf, ssize_t buflength);

void print_array_contents_as_char(char* buf, ssize_t buflength);




#endif