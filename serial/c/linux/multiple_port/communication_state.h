/**
 * Copyright (c) 2017 willydlw
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See `main.c` for details.
 */


/**@file communication_state.h
* 
* @brief Communication State
*
*
* @author willydlw
* @date 30 Jan 2018
* @bugs No known bugs
*
*/

#ifndef COMMUNICATION_STATE_H
#define COMMUNICATION_STATE_H

#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>						// ssize_t
#include <sys/select.h>					// fd_set

#include <debuglog.h>



#define READ_MESSAGE_LENGTH_BYTES  5	/**< every message received will be this length */
#define WRITE_MESSAGE_LENGTH_BYTES 5	/**< every message transmitted will be this length */



/* ==========     External Global Variables     ========== */


// external globals must be initialized in the c file 
// and declared extern in the h file


/**
* @brief String names of operational states
*        for enum OperationalState.
*
*        Enum value is the index location of the corresponding
*        string name.
* 
*        Used for logging messages.
*/
extern const char* debug_operational_state_string[];



/**
* @brief String names of read write message states
*        for enum ReadWriteState.
*
*        Enum value is the index location of the corresponding
*        string name.
* 
*        Used for logging messages.
*/
extern const char* debug_read_write_message_state_string[];



/**
* @brief String names of error conditions
*        for enum ErrorCondition.
*
*        Enum value is the index location of the corresponding
*        string name.
* 
*        Used for logging messages.
*/
extern const char* debug_error_condition_string[];



/**
* @brief Messages transmitted to connected device
*/
extern const char* ackResponse;					/**< acknowledge message transitted after hello received */
extern const char* readyResponse;				/**< ready message transmitted after sensor registration */
extern const char* resetCommand;				/**< reset command message */
extern const char* stopCommand;					/**< stop command message */



/**
* @brief Messages received from connected device
*/
extern const char* helloMessage;				/**< confirms connection */
extern const char* helloMessageHEX;				/**< hexadecimal string of helloMessage character values
													 used for logging */



/**
* @brief message markers
*/
extern const uint8_t start_marker;				/**< indicates start of message */
extern const uint8_t end_marker;				/**< indicates end of message */



/**
* @brief Operational states
*/
typedef enum operational_state_t{
	WAIT_FOR_CONNECTION ,					/**< waits for initial message from other connected device */
	ACKNOWLEDGE_CONNECTION,					/**< send message acknowledging connection message received */
	WAIT_FOR_SENSOR_ID,						/**< waits to received connected device id message */
	SENSOR_REGISTRATION_COMPLETE,			/**< acknowledges device id received */
	RECEIVE_SENSOR_DATA,					/**< receives connected device data */
	NOT_OPERATIONAL							/**< no connection established, device not operational */
}OperationalState;



/**
* @brief Indicates message bytes that have been
*        received or transmitted.
*
*        Messages are received/transmitted byte by 
*        byte. As each byte is received/transmitted,
*        the message state is changed to reflect which 
*        bytes which byte is next.
*
*        Acts as read/write index in read/write buffers.
*/
typedef enum read_write_message_state_t
{ 
	START_MARKER = 0, 							/*< waiting to read/write start marker    */
	DATA_BYTE_ONE = 1,							/*< waiting to read/write data byte one   */
	DATA_BYTE_TWO = 2,							/*< waiting to read/write data byte two   */
	DATA_BYTE_THREE = 3, 						/*< waiting to read/write data byte three */
	END_MARKER = 4 								/*< waiting to read/write end marker      */
} ReadWriteMessageState;



/**
* @brief Communications State
*
*/
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
   

    OperationalState ostate;									/*< present operational state */
   
}CommState;




/**
* @brief error conditions help indicate pselect
*        return state
*/
typedef enum error_conditions_t { 
	SUCCESS, SELECT_FAILURE, SELECT_ZERO_COUNT 
} ErrorCondition;




/* ==========     Function Prototypes     ========= */



/** @brief Checks value of selectfds and returns error state.
*          In the case of a select failure error, a LOG_ERROR
*          level message is generated, including the string describing errnum.
*
* @param[in] selectfds      pselect return value, number of file descriptors 
* @param[in] errnum         errno generated by an error condition
*	    
* @return
*   SUCCESS                 when selectfds > 0
*   SELECT_ZERO_COUNT       when selectfds equals zero
*   SELECT_FAILURE 	        when selectfds < 0
*
*/
ErrorCondition check_select_return_value(int selectfds, int errnum);



/** @brief Reads data bytes from the serial file descriptor fd.
*          A maximum of MESSAGE_LENGTH_BYTES are read and stored in buf.
*
*
* @param[in]    fd		    file descriptor
* @param[in]    readfds	    read file descriptor set
*	    
* @param[out] buf           buffer which stores bytes read
*	    								  
* @return bytes read
*      
*/
ssize_t read_message(int fd, fd_set readfds, uint8_t *buf);



/** @brief Transmits a message
*              
*
* @param[in] fd		  		  file descriptor
* @param[in] writefds		  write file descriptor set
* @param[in] buf              buffer containing bytes to transmit
* @param[in] numBytes         number of bytes to transmit
*    								  
* @return number of bytes transmitted
*
*/
ssize_t write_message(int fd, fd_set writefds, uint8_t *buf, size_t numBytes);



/** @brief Stub function that is called to log the device data
*          which has been extracted from the received message.
*
* @param[in] theData    device data received
*
* @return void
*
* @note When integrating these functions into an application
*       this function should be modified to properly handle
*       received data.
*
*/   
void process_sensor_data_received(uint16_t theData);



/** @brief Converts numbers to equivalent hex string
*
*   Example: 254 is decimal value stored in source array
*            "FE" is character string, hex representation of 254
*
* @param[in] source     array containing numbers to be converted
* @param[in] slength    number of elements in source array
* @param[in] dlength    number of elements in destination array
*
* @param[out] destination   array containing string representation
*
* @note: dlength must be 3*salength + 1
* 
*/
void convert_array_to_hex_string(char* destination, ssize_t dlength, 
								const uint8_t* source, ssize_t slength);

#endif