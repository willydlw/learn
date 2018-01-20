#include <string.h>

#include "operations.h"
#include "serial.h"


int initialize_sensor_communication_operations(
	const char* sensorInputFileName, 
	SensorCommOperation *sensorCommArray, int saLength,
	int* totalSensorCount, int* activeSensorCount)
{
	int serialPortsOpened = 0;


	log_trace("function source code in progress");
	if( import_sensor_data( sensorInputFileName, sensorCommArray, saLength,
							totalSensorCount, activeSensorCount) == 0)
	{
		log_fatal("import_sensor_data returned 0");
		return -1;
	}


	int unopenedList[*activeSensorCount];
	int unopenedIndex = 0;

	// open serial connection for active sensors
	for(int i = 0; i < *totalSensorCount; ++i){
		if(sensorCommArray[i].sensor.active){
			sensorCommArray[i].commState.fd = 
				serial_init(sensorCommArray[i].sensor.devicePath, 
					sensorCommArray[i].sensor.baudRate);

			if(sensorCommArray[i].commState.fd != -1){
				++serialPortsOpened;
			}
			else{
				// add unopened devices to list
				unopenedList[unopenedIndex] = i;
				++unopenedIndex;
			}

		}
	}

	handle_failed_serial_connections(sensorCommArray, unopenedList, 
		unopenedIndex);

	// now initialize the communication states 
	initialize_communication_states(sensorCommArray, *totalSensorCount);

	return serialPortsOpened;
}




void handle_failed_serial_connections(SensorCommOperation *sensorCommArray,
		int *unopenedList, int numUnopened)
{
	log_warn("TODO: function has not been coded to handle unopened serial connections");
	if(numUnopened == 0){
		log_info("all serial connections for active devices opened");
		return;
	}

	for(int i = 0; i < numUnopened; ++i){
		log_error("failed to open senor: %s, device path %s", 
			sensorCommArray[unopenedList[i]].sensor.name,
			sensorCommArray[unopenedList[i]].sensor.devicePath);
		log_error("finish function by writing code to search dev directory"
					" and try connecting to those tty devices\n");
	}
	
}


// NOTE: do not call this function before establishing serial connection
// tests for file descriptor != -1
void initialize_communication_states(SensorCommOperation *sensorCommArray, 
	int salength)
{
	for(int i = 0; i < salength; ++i){

		if(sensorCommArray[i].sensor.active && 
			sensorCommArray[i].commState.fd != -1){

			sensorCommArray[i].commState.readState = true;
			sensorCommArray[i].commState.ostate = WAIT_FOR_CONNECTION;
		
		}
		else{	// don't want to read if device not operational
			sensorCommArray[i].commState.readState = false;
			sensorCommArray[i].commState.ostate = NOT_OPERATIONAL;
		}

	
	/* write state false because the first operational state waits
	   to receive a message from the connected device
	*/
	sensorCommArray[i].commState.writeState = false;


	sensorCommArray[i].commState.readCompletedState = false;
	sensorCommArray[i].commState.writeCompletedState = false;


	/* Note: setting readIndex,writeIndex to 0 whether sensor is operational 
	   or not. A non-operational sensor should never attempt to read, but 
	   don't want to take a chance of anything going wrong by setting the 
	   index to an out of bounds array index value
	*/
	sensorCommArray[i].commState.readIndex = START_MARKER;
	sensorCommArray[i].commState.writeIndex = START_MARKER;

	/* zero out buffers ??? 

	   Could call memset here to write 0's to all buffer array elements.
	   This is not really a necessary step because the functions that
	   use these buffers, completely fill every element every time,
	   writing over the old data. 
	*/

	}	
}



int import_sensor_data(const char* filename, SensorCommOperation *sensorCommArray, 
	int salength, int *totalSensorCount, int *activeSensorCount)
{
	FILE *ifp = NULL;
	int lineCount = 0;					// number of lines read from file

	// temporary storage, data read from file
	char name[SENSOR_NAME_LENGTH] = {'\0'};	
	char devicePath[SERIAL_DEV_PATH_LENGTH] = {'\0'};
	uint8_t id;
	int onOff;
	int baudRate;


	ifp = fopen(filename, "r");

	if(ifp == NULL){
		log_fatal("failed to open file %s", filename);
		return 0;
	}

	// initialize counts
	*totalSensorCount = 0;
	*activeSensorCount = 0;

	// parse file to extract sensor name, id, active state
	//memset(name, 0, SENSOR_NAME_LENGTH);

	while(*totalSensorCount < salength && 
			fscanf(ifp, "%s%hhu%d%s%d", name, &id, &onOff, 
					devicePath, &baudRate) == 5){
		
		++lineCount;

		// since id is an unsigned int, it will always be positive
		// no need to compare it to minimum sensor id which is 0
		if(id <= MAX_SENSOR_ID){
			sensorCommArray[id].sensor.id = id;
		}
		else{
			log_warn("input file: %s, line: %d, out of range sensor id %d",
						filename, lineCount, id);
			memset(name, 0, SENSOR_NAME_LENGTH);
			memset(devicePath, 0, SERIAL_DEV_PATH_LENGTH);
			continue;	// back to while test condition
		}

		// populate data members
		sensorCommArray[id].sensor.active = onOff;
		sensorCommArray[id].sensor.baudRate = baudRate;		
		strcpy(sensorCommArray[id].sensor.name, name);
		strcpy(sensorCommArray[id].sensor.devicePath, devicePath);

		// increment counts
		if(onOff == 1){
			*activeSensorCount = *activeSensorCount + 1;
		}

		*totalSensorCount = *totalSensorCount + 1;

		// clear array
		memset(name, 0, SENSOR_NAME_LENGTH);
	}


	fclose(ifp);
	ifp = NULL;

	log_info("totalSensorCount: %d, activeSensorCount: %d", *totalSensorCount, 
				*activeSensorCount);

	return 1;
}


void build_fd_sets(SensorCommOperation *sensorCommArray, int length, int *readCount, 
	int* writeCount, fd_set *readfds, fd_set *writefds)
{
	FD_ZERO(readfds);
	FD_ZERO(writefds);
	*readCount = 0;
	*writeCount = 0;

	for(int i = 0; i < length; ++i){

		if(sensorCommArray[i].commState.readState){
			FD_SET(sensorCommArray[i].commState.fd, readfds);
			*readCount = *readCount + 1;
		}

		if(sensorCommArray[i].commState.writeState){
			FD_SET(sensorCommArray[i].commState.fd, writefds);
			*writeCount = *writeCount + 1;
		}
	}

}

uint32_t read_fdset(SensorCommOperation *sensorCommArray, int length, fd_set *readfds)
{
	   
    // bits set to one represent sensor with complete message received
    uint32_t completedList = 0;
    ssize_t bytesRead;  


	for(int i = 0; i < length; ++i){

		/* Note: the read_message function restricts the maximum number of bytes
		   read to MESSAGE_LENGTH_BYTES, the length of one entire message.

		   The software then should never read more than one message at a time.
		   However, this does not guarantee that MESSAGE_LENGTH_BYTES will be read. 

		   For example, assume that MESSAGE_LENGTH_BYTES is 5 bytes. Suppose that
		   only the first two message bytes are read. After reading the first two bytes,
		   the message state will be AWAITING_DATA_BYTE_ONE. This is why
		   we maintain a message state.  

		   The next time this function is entered, what happens if 5 bytes are read?
		   The first 3 bytes complete the previous message causing the process message
		   flag to be set and the data in temp buffer to be copied in the
		   message buffer. 

		*/

		bytesRead = read_message(sensorCommArray[i].commState.fd, *readfds, 
									sensorCommArray[i].commState.readBuffer);

        if(bytesRead > 0){

            sensorCommArray[i].commState.readIndex = 
            	process_received_message_bytes(sensorCommArray[i].commState.readCompletedBuffer,
            		sensorCommArray[i].commState.readBuffer,
            		bytesRead, 
            		sensorCommArray[i].commState.readIndex, 
            		&sensorCommArray[i].commState.readCompletedState);

            
        	/* TODO: Decide whether to call the function to process 
        	   the completed message or build a list of those devices
        	   with messages that are ready to process. 

        	   if(sensorCommArray[i].commState.completedFlag){ call appropriate function }

        	   Current solution is to just build a list. Final solution to be determined 
        	   based on real-time requirements and processing required for different
        	   device data and states.
        	*/

        	if(sensorCommArray[i].commState.readCompletedState == true){

        		// use sensor id to set corresponding bit to 1
        		completedList |= (uint32_t)(1 << sensorCommArray[i].sensor.id);
        	}
        } // end if bytes_read > 0
	}  // end for

	return completedList;
}



/**   comments are completely out of date, revise
*
*
* NAME : ssize_t process_received_message_bytes(MessageState *msgState, 
*												const uint8_t *buf, 
*												ssize_t bytes_read, 
*												uint8_t *responseData)
*
* DESCRIPTION: Based on the message state, bytes are copied from buf
*              to the appropriate position in responsedData.
*
*			   The message state is updated with each byte transferred.
*
*			   The fucntion returns When the MESSAGE_COMPLETE state is reached,
*			   or when bytes_read have been transferred. t
*
*              
* INPUTS: 
*   Parameters:
*       MessageState* 	msgState       	  pointer to the message state
*	    const uint8_t*  buf 			  buffer containing serial bytes read
*	    ssize_t			bytes_read		  number of bytes in buf
*
* OUTPUTS:
*
*	    MessageState*   msgState          message state is updated as bytes
*                                         are read from buf and stored in 
*										  responseData
*
*		uint8_t*		responseData      bytes extracted from buf
*	    	
*							  
*   Return:
*       type:			ssize_t
*
*		number of bytes that were not transferred from buf to responseData
*
*      
* NOTES:
*		When the message state is AWAITING_START_MARKER and the start marker
*		is not read, an error message is printed. The state does change
*		until the start marker is read.
*
*		If the default case of the message state machine is reached, an
*		error message is printed, and the message state is set to 
*		AWAITING_START_MARKER. Some data may be lost. The default case
*       should never be reached.
*
*/
ReadWriteMessageState process_received_message_bytes(uint8_t *destination,
	uint8_t *source, ssize_t bytesRead, ReadWriteMessageState readIndex, bool *completedFlag)
{
	
	ssize_t i;
	
	/* Why do we process one byte at a time, when we could use memcpy to copy
	   the entire message at once?

	   Is it possible to lose a byte? If not possible, then memcpy may be
	   the faster choice.

	   Example: Assume each complete message contains 5 bytes.

	   read index is zero and we read 3 bytes, without verifying
	   the first byte is the start marker. The three bytes are stored in 
	   index 0, 1, 2. Read index is now 3.

	   Next time 4 bytes are read. The first two bytes are copied into index
	   3 and 4. We mark complete message received. The index 0 byte has not
	   been checked to ensure it was a start marker. Index 4 has not been
	   verified to see if it is the end marker.

	   The message is data byte, data byte, data byte, end marker, start marker

	   Later when the message is checked it will not match any expected, 
	   valid message. The data will be thrown away, but we will lose the
	   start marker of the next message and may throw away every single
	   data message after that.

	   The method below remains in the AWAITING_START_MARKER state until
	   the start marker is received. We only lose the one corrupted message
	   in that case.
	*/

	for(i = 0; i < bytesRead; ++i){

		log_trace("message state readIndex: %25s, source[%ld]: %#4x, (char)source[%ld]: %c\n", 
							debug_read_write_message_state_string[readIndex], i,
							source[i], i, (char)source[i]);


		switch(readIndex){
			case START_MARKER:
				if((char)source[i] == start_marker){
					//memset(receivedBuffer, '\0', (MESSAGE_LENGTH_BYTES+1)*sizeof(uint8_t) );
					destination[readIndex] = source[i];
					++readIndex;						// becomes AWAITING_SENSOR_ID
				}
				else { 
					log_warn("state: AWAITING_START_MARKER, source[%ld]: %#x\n", i, source[i]);
				}
				break;

			case DATA_BYTE_ONE:
			case DATA_BYTE_TWO:
			case DATA_BYTE_THREE:						// all 3 cases require same action
				destination[readIndex] = source[i];
				++readIndex;							
				break;	

			case END_MARKER:

				if((char)source[i] == end_marker){
					destination[readIndex] = source[i];
					destination[readIndex+1] = '\0';     	// null terminate the string
					*completedFlag = true;					// full message received
				}
				else{
					char tempBuf[MESSAGE_LENGTH_BYTES+1] = {'\0'};

					convert_array_to_hex_string(tempBuf, MESSAGE_LENGTH_BYTES+1, destination, readIndex+1);

					log_warn("expecting end marker value %#x, source[%ld]: %#x"
						"discarding %s" , end_marker, i, source[i], tempBuf); 
				}

				readIndex = START_MARKER;

				break;
				

			default:
				log_error("reached default case, readIndex: %d", readIndex);
				
				// set message state to await start of new message
				// throw away the unkown message bytes
				readIndex = START_MARKER;
		}
	}

	return readIndex;
}


void process_operational_state(SensorCommOperation *sco)
{

	log_trace("entered function, operational state: %s", debug_operational_state_string[sco->commState.ostate]);
	switch(sco->commState.ostate)
	{
		case WAIT_FOR_CONNECTION:

			// processing the message we read, need to reset readCompletedState
			// to indicate it had been processed
			sco->commState.readCompletedState = false;
			
			// verify expected message was received before transitioning to next state
			if( strcmp((char*)sco->commState.readCompletedBuffer, helloMessage) == 0){
				sco->commState.ostate = ACKNOWLEDGE_CONNECTION;
				sco->commState.readState = false;
				sco->commState.writeState = true;
				// load acknowledge message into write buffer
				strcpy(sco->commState.writeBuffer, ackResponse);
			}
			else{
				// log error, do not change state
				char tempBuf[MESSAGE_LENGTH_BYTES+1];
				convert_array_to_hex_string(tempBuf, MESSAGE_LENGTH_BYTES+1, 
						sco->commState.readCompletedBuffer, 
						(ssize_t)strlen((char*)sco->commState.readCompletedBuffer));

				log_error("operational state: %s, expected: %s, received: %s",
					debug_operational_state_string[sco->commState.ostate],
					helloMessageHEX, tempBuf);
			}
			break;

		case ACKNOWLEDGE_CONNECTION:
			if(sco->commState.writeCompletedState == true){
				// after transmitting ack response, the next operational
				// state is waiting to receive the sensor id from the device
				sco->commState.ostate = WAIT_FOR_SENSOR_ID;
				sco->commState.writeState = false;
				sco->commState.readState = true;
				sco->commState.writeCompletedState = false;	
			}
			else WHAT?
			break;

		case WAIT_FOR_SENSOR_ID:
		log_fatal("WAIT_FOR_SENSOR_ID not programmed");
		break;
		case SENSOR_REGISTRATION_COMPLETE:
		log_fatal("SENSOR_REGISTRATION_COMPLETE not programmed");
		break;
		case RECEIVE_SENSOR_DATA:
		log_fatal("RECEIVE_SENSOR_DATA not programmed");
		break;
		case NOT_OPERATIONAL:
			log_fatal("NOT_OPERATIONAL not programmed");
		break;
	}

}



void process_completed_messages(SensorCommOperation *sensorCommArray, 
		int length, uint32_t completedList)
{

	for(int i = 0; i < length; ++i){

		log_trace("completedList: %#x, (i << %d): %#x", completedList, i, 1 << i);

		// array index location is the same as the sensor id
		// bit mask is 1 << i
		if( (completedList & (uint32_t)(1 << i)) ){

			// for debug only, remove when debugging completed
			char debugBuf[MESSAGE_LENGTH_BYTES+1];
			convert_array_to_hex_string(debugBuf, MESSAGE_LENGTH_BYTES+1, 
				sensorCommArray[i].commState.readCompletedBuffer,
				MESSAGE_LENGTH_BYTES+1);
				
			log_trace("sensor %d received Buffer: %s", i, debugBuf);
			// end debug

			
			process_operational_state(&sensorCommArray[i]);

			
		}
	}

}




/* @brief Searches sensor comm array to find the file descriptor with
*         the largest value.
*    
*
* @param[in] sensorCommArray		array of sensor communication operation structures
* @param[in] length                 number of elements in sensorCommArray
*
* @return largest file descriptor value
*/
int find_largest_fd(const SensorCommOperation *sensorCommArray, 
		int length)
{
	
	int maxfd = sensorCommArray[0].commState.fd;

	for(int i = 1; i < length; ++i){
		if(sensorCommArray[i].commState.fd > maxfd){
			maxfd = sensorCommArray[i].commState.fd;
		}
	}
	return maxfd;
}


/* @brief Closes all open file descriptors in the sensor array
*    
*
* @param[in] sensorCommArray		array of sensor communication operation structures
* @param[in] length                 number of elements in sensorCommArray
*
* @return void
*/
void close_serial_connections(SensorCommOperation *sensorCommArray, 
		int length)
{
	for(int i = 0; i < length; ++i){
		if(sensorCommArray[i].commState.fd != -1){
			serial_close(sensorCommArray[i].commState.fd);
		}
	}
}



/* @brief Logs the state of every data member in the sensorCommArray.
*         
*		  Log trace level
*
*         Buffer contents are written as a hexadecimal string
*
* @param[in] sensorCommArray		array of sensor communication operation structures
* @param[in] length                 number of elements in sensorCommArray
*
* @return void
*/
void log_SensorCommOperation_data(const SensorCommOperation *sensorCommArray, int length)
{
	for(int i = 0; i < length; ++i){

		char tempReadBuf[MESSAGE_LENGTH_BYTES+1];
		char tempReadCompletedBuf[MESSAGE_LENGTH_BYTES+1];
		char tempWriteBuf[MESSAGE_LENGTH_BYTES+1];

		convert_array_to_hex_string(tempReadBuf, MESSAGE_LENGTH_BYTES+1, 
			sensorCommArray[i].commState.readBuffer, MESSAGE_LENGTH_BYTES);

		convert_array_to_hex_string(tempReadCompletedBuf, MESSAGE_LENGTH_BYTES+1, 
			sensorCommArray[i].commState.readCompletedBuffer, MESSAGE_LENGTH_BYTES);

		convert_array_to_hex_string(tempWriteBuf, MESSAGE_LENGTH_BYTES+1, 
			sensorCommArray[i].commState.writeBuffer, MESSAGE_LENGTH_BYTES);


        log_trace("\n\tsensor - \n"
        	"\tid: %d, name: %s, active: %d\n"
        	"\tdevice path: %s, baud rate: %d\n"
        	"\tstate -\n"
        	"\tostate: %s, fd: %d\n"
        	"\treadIndex: %s, readState: %d, readCompletedState: %d \n"
        	"\twriteIndex: %s, writeState: %d\n"
        	"note: printing entire contents of buffer, may contain garbage\n"
        	"\tdepending on operational state, as well as read/write Index\n"
        	"\treadBuffer: %s\n"
        	"\treadCompletedBuffer: %s\n"
        	"\twriteBuffer: %s\n",

            sensorCommArray[i].sensor.id, 
            sensorCommArray[i].sensor.name,
            sensorCommArray[i].sensor.active, 
            sensorCommArray[i].sensor.devicePath,
            sensorCommArray[i].sensor.baudRate,
            debug_operational_state_string[sensorCommArray[i].commState.ostate],
            sensorCommArray[i].commState.fd,
            debug_read_write_message_state_string[sensorCommArray[i].commState.readIndex],
            sensorCommArray[i].commState.readState,
            sensorCommArray[i].commState.readCompletedState,
            debug_read_write_message_state_string[sensorCommArray[i].commState.writeIndex],
            sensorCommArray[i].commState.writeState,
            tempReadBuf,
            tempReadCompletedBuf,
            tempWriteBuf);

    }
}