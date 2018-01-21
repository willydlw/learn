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

#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>						// ssize_t
#include <sys/select.h>					// fd_set

#include <debuglog.h>


/* ==========     Preprocessor Directives     ==========*/

#define NUM_MESSAGE_STATES 7			// number of message states defined in enum MessageState

#define READ_MESSAGE_LENGTH_BYTES 5		// every message received will be this length 
#define WRITE_MESSAGE_LENGTH_BYTES 5	// every message transmitted will be this length



/* ==========     External Global Variables     ========== */


// external globals must be initialized in the c file 
// and declared extern in the h file


// string arrays used for debugging
// states are numeric, when writing an error message
// displaying a string provides more meaningful information

extern const char* debug_comm_read_state_string[];
extern const char* debug_comm_write_state_string[];

extern const char* debug_operational_state_string[];
extern const char* debug_read_write_message_state_string[];
extern const char* debug_error_condition_string[];


// command messages transmitted to other program
extern const char* readyCommand;
extern const char* resetCommand;
extern const char* stopCommand;

// response messages received from other program
extern const char* ackResponse;					// acknowledge message received, command implemented
extern const char* nackResponse;				// nack - not acknowledge
extern const char* helloMessage;				// confirms connection
extern const char* helloMessageHEX;				// hexadecimal string of helloMessage character values


// message markers
extern const uint8_t start_marker;
extern const uint8_t end_marker;




// operational states
typedef enum operational_state_t{
	WAIT_FOR_CONNECTION ,
	ACKNOWLEDGE_CONNECTION,
	WAIT_FOR_SENSOR_ID,
	SENSOR_REGISTRATION_COMPLETE,
	RECEIVE_SENSOR_DATA,
	NOT_OPERATIONAL
}OperationalState;



// communication states
/*
typedef enum comm_read_state_t { 
	NO_READ = 0,
	READ_HELLO = 1, 
	READ_ACK = 2,
	READ_SENSOR = 3 
}CommReadState;

typedef enum comm_write_state_t { 
	NO_WRITE = 0,
	SEND_READY_SIGNAL = 1,
	SEND_RESET = 2, 
	SEND_STOP = 3
}CommWriteState;
*/


/*  messages are received byte by byte. As each byte is recevied,
    the message state is changed to reflect which bytes have
    been received, and which byte is expected next.
*/
typedef enum read_write_message_state_t
{ 
	START_MARKER = 0, 							/*< waiting to read/write start marker    */
	DATA_BYTE_ONE = 1,							/*< waiting to read/write data byte one   */
	DATA_BYTE_TWO = 2,							/*< waiting to read/write data byte two   */
	DATA_BYTE_THREE = 3, 						/*< waiting to read/write data byte three */
	END_MARKER = 4 								/*< waiting to read/write end marker      */
} ReadWriteMessageState;





typedef struct comm_state_t{
    
    int fd;													/*< serial file descriptor */

	// state flags
    bool readState;											/*< true when serial data should be read       */
    bool writeState;										/*< true when serial data shoud be transmitted */
    bool readCompletedState;								/*< true when entire message has been received */
    bool writeCompletedState;								/*< true when entire message has been written  */

    // index
    ReadWriteMessageState readIndex;						/*< readBuffer storage location for next byte read */
    ReadWriteMessageState writeIndex;						/*< writeBuffer storage location for next byte written */

	// storage
    uint8_t readBuffer[READ_MESSAGE_LENGTH_BYTES];				/*< stores serial bytes read */
    uint8_t writeBuffer[WRITE_MESSAGE_LENGTH_BYTES];				/*< stores serial write bytes */

    uint8_t readCompletedBuffer[READ_MESSAGE_LENGTH_BYTES+1];	/*< stores message ready for processing. 
    															additional byte needed for string operations */
   

    OperationalState ostate;								/*< present operational state */
   
}CommState;




/*  error conditions help indicate exact sources of failure */
typedef enum error_conditions_t { 
	SUCCESS, SELECT_FAILURE, SELECT_ZERO_COUNT 
	//SERIAL_WRITE_ERROR
} ErrorCondition;



/* ==========     Function Prototypes     ========= */




ErrorCondition check_select_return_value(int selectfds, int errnum, int *zeroCount);

ssize_t read_message(int fd, fd_set readfds, uint8_t *buf);





ssize_t write_message(int fd, fd_set writefds, uint8_t *buf, size_t numBytes);



void process_sensor_data_received(uint16_t theData);

void convert_array_to_hex_string(char* destination, ssize_t dlength, 
								const uint8_t* source, ssize_t slength);



/*void process_read_state_error_message(CommReadState commReadState, 
	const uint8_t *responseData, ssize_t rlength);
*/

#endif