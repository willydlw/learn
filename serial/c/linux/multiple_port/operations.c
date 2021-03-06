/**
 * Copyright (c) 2017 willydlw
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See `main.c` for details.
 */


/**@file operations.c
* 
* @brief Functions needed for initializing and processing
*        communication with multiple serial connections.
*
*
* @author willydlw
* @date 30 Jan 2018
* @bugs No known bugs
*
*/

#include <string.h>
#include <signal.h>

#include "operations.h"
#include "serial.h"


/* @brief Initialize sensor communications array
*
*
* \par Process
*
*       [1] read sensor data input file
*       [2] populate sensorCommArray elements with sensor data
*
*       [3] if read sensor data successful, continue
*           else log fatal message, return false
*
*       [4] open serial connection for all active devices
*           
*       [5] if serial connection fails to open
*              add unopened devices to list
*
*       [6] if there are unopened devices
*              call handle failed serial connections function
*
*       [7] initialize communication states for each array element
*       [8] return true
*
*
* @param[in] sensorInputFileName 	input file containing sensor device information
*                                   sensor name, sensor id, active state, serial 
*									device path, and baud rate
*
* @param[out] sensorCommArray       array of sensor communication data structures
*
* @param[out] debugStats            initializes all data members to zero
*
* @return  success - true
*          failure - false
*/
bool initialize_sensor_communication_operations(
	const char* sensorInputFileName, 
	SensorCommOperation *sensorCommArray, int saLength,
	DebugStats *debugStats)
{
	
	// read input data file and populate sensor struct members
	// 		name, id, active, baud rate, device path

	// populates debugStats totalSensorCount, activeSensorCount
	if( import_sensor_data( sensorInputFileName, sensorCommArray, 
											saLength, debugStats) == false)
	{
		log_fatal("import_sensor_data returned 0");
		return false;
	}

	
	// create array to keep track of active devices that have an
	// unopened serial connection
	int unopenedList[debugStats->activeSensorCount];
	int unopenedIndex = 0;

	/* open serial connection for active sensors

	  	loop stop condition is totalSensorCount, not saLength
	  	as it is possible for saLength to be larger than
	  	totalSensorCount. Assumes it is not possible for saLength
	  	to be smaller than totalSensorCount. Depends on
	  	import_sensor_data function to guarantee this.

	  	All devices are listed in the sensorCommArray, whether
	  	active or not. The activeSensorCount may be smaller
	  	than the totalSensorCount. Ideally, it will be equal.

	*/
	for(int i = 0; i < debugStats->totalSensorCount; ++i){

		// only attempt to open a serial connection to active device
		if(sensorCommArray[i].sensor.active){
			sensorCommArray[i].commState.fd = 
				serial_init(sensorCommArray[i].sensor.devicePath, 
					sensorCommArray[i].sensor.baudRate);

			if(sensorCommArray[i].commState.fd != -1){
				++debugStats->serialPortsOpened;
				debugStats->activeSensorList |= (uint32_t)1 << (uint32_t)sensorCommArray[i].sensor.id;
			}
			else{	// add unopened devices to list
				unopenedList[unopenedIndex] = i;
				++unopenedIndex;
			}
		}
		else{	// inactive device
			// set file descriptor to unopened value of -1 
			sensorCommArray[i].commState.fd = -1;
		}
	}

	if(unopenedIndex > 0){
		handle_failed_serial_connections(sensorCommArray, unopenedList, 
		unopenedIndex, &debugStats->activeSensorCount);
	}
	

	// now initialize the communication states 
	initialize_communication_states(sensorCommArray, debugStats->totalSensorCount);

	return true;
}



/** @brief Logs error message for active devices that failed to open the
*          serial connection. Sets that device's state to inactive.
*
* @param[in/out] sensorCommArray 	array of SensorCommOperation structures
* 									sensors that failed to open serial connection:
*										status is changed from active to inactive
*
* @param[in] unopenedList			array containing sensor id of unopened connections
* @param[in] numUnopened 			number of unopened connections
* @param[in/out] activeSensorCount	number of active sensors is decremented for u
*									unopened connection
*
* @return void
*
* TODO: write routine to attempt to open active devices that failed
*	    to open serial connection
*
*/
void handle_failed_serial_connections(SensorCommOperation *sensorCommArray,
		int *unopenedList, int numUnopened, int *activeSensorCount)
{
	
	// log error messages for unopened connections
	for(int i = 0; i < numUnopened; ++i){

		log_error("failed to open sensor, name: %s, id: %d, device path %s,"
			"setting device to inactive state", 
			sensorCommArray[unopenedList[i]].sensor.name,
			sensorCommArray[unopenedList[i]].sensor.id,
			sensorCommArray[unopenedList[i]].sensor.devicePath);

		// change device's state to inactive
		sensorCommArray[unopenedList[i]].sensor.active = false;
		*activeSensorCount -= 1;   // decrement count
	}
}



/** @brief Initializes the commState data members
*
* @param[out] sensorCommArray 	array of sensor device data structures
* @param[in] salength 			number of elements in sensorCommArray
*
* @return void
*
* @note: do not call this function before establishing serial connection
* tests for file descriptor != -1
*/
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



/* @brief Populates sensor device data from input file
*
*  Input data file should contain
*  		sensor name  	sensor id 	active state  	device path
*
*  in that order, separated by white space.
*
* \par Process
*	[1] open input file 
*	[2] if open fails return false
*	[3] set all debugStats data members to zero
*   [4] while (records read < total sensor count and read does not fail)
*         read name, id, active state, device path,
*   	  if id > MAX_SENSOR_ID
*			log fatal message
*			return false
*		  else
*         	store in sensorCommArray[id] element's sensor object
*         increment total sensor count
*         if active
*           increment active sensor count
*			use id to set corresponding bit in active sensor list
*   [5] close file
*   [6] return true
*
*
*  The sensor id is used to set the corresponding bit position
*  in the activeSensorList.
*  		Example: sensor id 0 is active, bit 0 is set to 1
*				 sensor id 1 is not active, bit 1 is set to 0
*
* @param[in] filename 		sensor data input file name
* @param[in] salength 		number elements in sensorCommArray
* @param[out] sensorCommArray 	array of SensorCommOperation structures
* @param[out] debugStats 	data structure that contains debugging statistics
*
* @return success true  when finished reading file
*         failure false when input file fails to open or 
*						for an invalid sensor id
*
*/
bool import_sensor_data(const char* filename, SensorCommOperation *sensorCommArray, 
	int salength, DebugStats *debugStats)
{
	// File variables
	FILE *ifp = NULL;
	int lineCount = 0;					// number of lines read from file

	// temporary storage for data read from file
	char name[SENSOR_NAME_LENGTH] = {'\0'};	
	char devicePath[SERIAL_DEV_PATH_LENGTH] = {'\0'};
	uint8_t id;
	int active;
	int baudRate;


	ifp = fopen(filename, "r");
	if(ifp == NULL){
		log_fatal("failed to open file %s", filename);
		return false;
	}

	// initialize debug stats 
	debugStats->selectZeroCount = 0;
	debugStats->sensorIdMismatchCount = 0;
	debugStats->serialPortsOpened = 0;
	debugStats->totalSensorCount = 0;
	debugStats->activeSensorCount = 0;
	debugStats->activeSensorList = 0;
	debugStats->registeredSensorList = 0;


	// read file to extract sensor name, id, active state,
	// and device path name
	while( debugStats->totalSensorCount < salength && 
		   fscanf(ifp, "%s%hhu%d%s%d", name, &id, &active, 
							devicePath, &baudRate) == 5)
	{
		
		++lineCount;

		// Becuase id is an unsigned int, it will always be positive
		// no need to compare it to the minimum sensor id which is 0
		if(id <= MAX_SENSOR_ID){
			sensorCommArray[id].sensor.id = id;
		}
		else{
			log_fatal("input file: %s, line: %d, out of range sensor id %d",
						filename, lineCount, id);
			return false;	
		}

		// populate data members
		sensorCommArray[id].sensor.active = active;
		sensorCommArray[id].sensor.baudRate = baudRate;		
		strcpy(sensorCommArray[id].sensor.name, name);
		strcpy(sensorCommArray[id].sensor.devicePath, devicePath);

		// update counts and list
		debugStats->totalSensorCount += 1;

		// increment counts
		if(active == 1){
			debugStats->activeSensorCount += 1;
			debugStats->activeSensorList &= (uint32_t)1U << (uint32_t)id;
		}

		// clear arrays
		memset(name, 0, SENSOR_NAME_LENGTH);
		memset(devicePath, 0, SERIAL_DEV_PATH_LENGTH);
	}

	
	fclose(ifp);	// we are done with the input file, close it
	ifp = NULL;	

	return true;
}


/**
* @brief Adds file descriptors to sets.
*
* @param[in] sensorCommArray    array of sensor communication structs
* @param[in] length             number of elements in sensorCommArray
* @param[out] readCount         number of sensor comm elements in read state
* @param[out] writeCount        number of sensor comm elements in write state
* @param[out] readfds           read file descriptor set
* @param[out] writefds          write file descriptor set
*
* @return void 
*
*/
void build_fd_sets(const SensorCommOperation *sensorCommArray, int length, int *readCount, 
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



/**
* @brief Reads messages from file descriptors ready to read.
*
* Updates readIndex and readBuffer when bytes are read. 
* Sets receiveCompletedState when a complete message has been received,
* and copies readBuffer to readCompletedBuffer.
*
* @param[in/out] sensorCommArray    array of sensor communication structs
* @param[in] length             number of elements in sensorCommArray
* 
* @param[out] readfds           read file descriptor set
*
* @return completed List - bits set to one indicate complete message received.
*                          Otherwise, bits are set to zero.
*                          Bit position corresponds to sensor id.
*                         
*/
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

		if(sensorCommArray[i].sensor.active != true){
			continue;
		}


		bytesRead = read_message(sensorCommArray[i].commState.fd, *readfds, 
									sensorCommArray[i].commState.readBuffer);

        if(bytesRead > 0){

            sensorCommArray[i].commState.readIndex = 
            	process_received_message_bytes(sensorCommArray[i].commState.readCompletedBuffer,
            		sensorCommArray[i].commState.readBuffer,
            		bytesRead, 
            		sensorCommArray[i].commState.readIndex, 
            		&sensorCommArray[i].commState.readCompletedState);

        	if(sensorCommArray[i].commState.readCompletedState == true){

        		// use sensor id to set corresponding bit to 1
        		completedList |= (uint32_t)1 << (uint32_t)sensorCommArray[i].sensor.id;
        	}
        } // end if bytes_read > 0
	}  // end for

	return completedList;
}




/**
* @brief Writes messages from file descriptors ready to write.
*
* Updates writeIndex when bytes are written.
* Sets writeCompletedState to true when a complete message has been 
* written. readIndex is reset to zero when complete message has 
* been written.
*
* @param[in/out] sensorCommArray    array of sensor communication structs
* @param[in] length             number of elements in sensorCommArray
* 
* @param[out] writefds           write file descriptor set
*
* @return completed list - bits set to one indicate complete message written.
*                          Otherwise, bits are set to zero.
*                          Bit position corresponds to sensor id.
*                         
*/
uint32_t write_fdset(SensorCommOperation *sensorCommArray, int length, fd_set *writefds)
{
	   
    // bits set to one represent sensor with complete message received
    uint32_t completedList = 0;

    size_t bytesToWrite; 
    ssize_t bytesWritten;


    for(int i = 0; i < length; ++i){

    	if(sensorCommArray[i].sensor.active != true){
			continue;
		}

    	bytesToWrite = WRITE_MESSAGE_LENGTH_BYTES - sensorCommArray[i].commState.writeIndex;

    	log_trace("sensor id: %d, bytesToWrite: %d", 
    		sensorCommArray[i].sensor.id, bytesToWrite)

    	
    	bytesWritten = write_message(sensorCommArray[i].commState.fd, 
    								*writefds, 
    								// pass the address of the first byte that should be transmitted
									&sensorCommArray[i].commState.writeBuffer[sensorCommArray[i].commState.writeIndex],
									bytesToWrite);

    	log_trace("bytesWritten: %d", bytesWritten);

    	// update the write index
    	// Important when the bytesWritten differ from the bytesToWrite
    	sensorCommArray[i].commState.writeIndex += bytesWritten;


    	/* Example: writeIndex is 0
    				WRITE_MESSAGE_LENGTH_BYTES is 5
    				writeBuffer length is always same as WRITE_MESSAGE_LENGTH_BYTES
    				If all 5 bytes are written, then writeIndex + bytesWritten = 5
    				5 is beyond the array boundary, means the entire message was written
		*/
    	if(sensorCommArray[i].commState.writeIndex >= WRITE_MESSAGE_LENGTH_BYTES){

    		sensorCommArray[i].commState.writeIndex = 0;      // reset to start
     		sensorCommArray[i].commState.writeCompletedState = true;

    		// use sensor id to set corresponding bit to 1
        	completedList |= (uint32_t)1 << (uint32_t)sensorCommArray[i].sensor.id;

    	}
    }

    return completedList;
}



/**
* @brief Advances readIndex as bytes are copied from source to destination. Sets
*        completed flag when end marker is read.
*
*
* For all of the bytes: zero to salength-1, each byte is copied from the source
* to the destination array. The destination array location is based on the 
* readIndex value. The readIndex value is set to the next appropriate message
* state (index location).
*
* When the message state is AWAITING_START_MARKER and the start marker
* is not read, an error message is printed. The state does change
* until the start marker is read.
*
* When the message state is END_MARKER and
*   - the end marker is read: the data byte is copied from source to 
*     destination, followed a null-termination character added to
*     the destination array. The completedFlag is set to true.
*
*   - the end marker is not read: An error message is logged. The
*     destination data will be discarded.
*
*   readIndex is set to AWAITING_START_MARKER 
*
* If the default case of the message state machine is reached, an
* error message is printed, and the message state is set to 
* AWAITING_START_MARKER. Some data may be lost. The default case
* should never be reached.
*
*
* @param[in]  source				  array containing data bytes to be copied
* @param[in]  slength                 number of elements in source array
*
* @param[in]  readIndex               index location where data will be copied
*                                     into destination array. Also represents
*                                     the message state.
*
* @param[out] destination             array to which source data is copied
* @param[out] completedFlag           set to true when the END_MARKER has
*                                     been recevied and copied into destination
*
* @return     readIndex value is returned
*/

ReadWriteMessageState process_received_message_bytes(uint8_t *destination,
	uint8_t *source, ssize_t slength, ReadWriteMessageState readIndex, bool *completedFlag)
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

	for(i = 0; i < slength; ++i){

		log_trace("message state readIndex: %s, source[%ld]: %#4x, (char)source[%ld]: %c\n", 
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
					log_warn("state: %s, source[%ld]: %#x\n", 
						debug_read_write_message_state_string[readIndex], i, source[i]);
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

					char hexmsg[3*readIndex+1];

					convert_array_to_hex_string(hexmsg, 3*readIndex+1, destination, readIndex);

					log_warn("expecting end marker value %#x, received: %#x"
						"discarding %s" , end_marker, source[i], hexmsg); 
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


void process_operational_state(SensorCommOperation *sco, DebugStats *debugStats)
{

	// for debug only, remove when debugging completed
	if(sco->commState.readState == true){
		char hexmsg[3*READ_MESSAGE_LENGTH_BYTES+1];
			convert_array_to_hex_string(hexmsg, 3*READ_MESSAGE_LENGTH_BYTES+1, 
				sco->commState.readCompletedBuffer,
				READ_MESSAGE_LENGTH_BYTES);
				
			log_trace("sensor id: %d, message received: %s\n", sco->sensor.id, hexmsg);

	}

	if(sco->commState.writeState == true){
		log_trace("sensor id: %d, completed writing message\n", sco->sensor.id);
	}
	// end debug


	switch(sco->commState.ostate)
	{
		case WAIT_FOR_CONNECTION:
			// checking readCompletedState is redundant
			// should not be here unless that were true
		    // after sufficient testing, may want to remove
		    // this check to speed execution
			if(sco->commState.readCompletedState == true){

				// processing the message we read, need to reset readCompletedState
				// to indicate it had been processed
				sco->commState.readCompletedState = false;
				
				// verify expected message was received before transitioning to next state
				if( strcmp((char*)sco->commState.readCompletedBuffer, helloMessage) == 0){

					log_info("SUCCESS, sensor id: %d, helloMessage recognized\n",
						sco->sensor.id);
					sco->commState.ostate = ACKNOWLEDGE_CONNECTION;
					sco->commState.readState = false;
					sco->commState.writeState = true;
					// load acknowledge message into write buffer
					strcpy((char*)sco->commState.writeBuffer, ackResponse);
				}
				else{
					// log error, do not change state
					char hexmsg[3*READ_MESSAGE_LENGTH_BYTES+1];
					convert_array_to_hex_string(hexmsg, 3*READ_MESSAGE_LENGTH_BYTES+1, 
							sco->commState.readCompletedBuffer, 
							READ_MESSAGE_LENGTH_BYTES);

					log_error("operational state: %s, expected: %s, received: %s",
						debug_operational_state_string[sco->commState.ostate],
						helloMessageHEX, hexmsg);
				}
			}
			else{
				log_error("operational state: %s, readCompletedState is false, "
					"when it should have been true, data contents logged below", 
					debug_operational_state_string[sco->commState.ostate]);

				log_SensorCommOperation_data(sco);
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
			else{

				log_debug("operational state: %s, writeCompletedState is false, data contents below",
					debug_operational_state_string[sco->commState.ostate]);
				log_SensorCommOperation_data(sco);
			}
			break;

		case WAIT_FOR_SENSOR_ID:
			if(sco->commState.readCompletedState == true){

				sco->commState.readCompletedState = false;

				// is this the id message
				if(sco->commState.readCompletedBuffer[2] == 'i' &&
					sco->commState.readCompletedBuffer[3] == 'd'){

					// extract the sensor id from the message received
					// data bytes one and two
					uint8_t sid;
					
					// sensor id contained in data byte one
					sid = sco->commState.readCompletedBuffer[DATA_BYTE_ONE];
					
					// verify the sensor id sent matches the one that was registered
					// in the initialization process
					if(sco->sensor.id == sid){
						log_trace("SUCCESS, sensor id match, id: %d", sid);
						sco->commState.ostate = SENSOR_REGISTRATION_COMPLETE;
						sco->commState.readState = false;
						sco->commState.writeState = true;
						strcpy((char*)sco->commState.writeBuffer, readyResponse);
					}
					else{
						log_fatal("operational state: %s, sensor id mismatch, expected: %d, received: %d",
									debug_operational_state_string[sco->commState.ostate], 
									sco->sensor.id, sid);
						raise(SIGTERM);
					}
				}
				else{   // die not receive id characters in message

					char hexmsg[3*READ_MESSAGE_LENGTH_BYTES+1];
					convert_array_to_hex_string(hexmsg, 3*READ_MESSAGE_LENGTH_BYTES+1, 
							sco->commState.readCompletedBuffer, 
							READ_MESSAGE_LENGTH_BYTES);

					log_warn("current state: %s, expected id message <%did>, received: %s\n",
						debug_operational_state_string[sco->commState.ostate],
						sco->sensor.id, hexmsg);
				}
			}
			else{

				log_error("operational state: %s, readCompletedState is false, "
					"when it should have been true, data contents logged below", 
					debug_operational_state_string[sco->commState.ostate]);

				log_SensorCommOperation_data(sco);
			}

			break;

		case SENSOR_REGISTRATION_COMPLETE:
		{
			static int stateEntryCount = 0;

			++stateEntryCount;

			if(sco->commState.writeCompletedState == true){

				log_trace("success, sensor %d registration complete", sco->sensor.id);

				// after transmitting ack response, the next operational
				// state is waiting to receive the sensor id from the device
				sco->commState.ostate = RECEIVE_SENSOR_DATA;
				sco->commState.writeState = false;
				sco->commState.readState = true;
				sco->commState.writeCompletedState = false;	
				debugStats->registeredSensorList |= (uint32_t)1 << (uint32_t)sco->sensor.id;
			}
			else{

				log_debug("operational state: %s, writeCompletedState is false, data contents below",
					debug_operational_state_string[sco->commState.ostate]);
				log_SensorCommOperation_data(sco);
			}

			if(stateEntryCount > debugStats->activeSensorCount){
				log_warn("stateEntryCount: %d, activeSensorCount: %d, "
					"expected all sensors to be registered", stateEntryCount, debugStats->activeSensorCount);
				log_warn("sensors with bit values of 1 are not registered, %#x", 
					debugStats->registeredSensorList ^ debugStats->activeSensorList);

				if(stateEntryCount > (2*debugStats->activeSensorCount) ){
					log_fatal("program terminating due to sensor registration failure");
					raise(SIGTERM);
				}
			}

		}

			
			break;

		case RECEIVE_SENSOR_DATA:

			// checking readCompletedState is redundant
			// should not be here unless that were true
		    // after sufficient testing, may want to remove
		    // this check to speed execution
			if(sco->commState.readCompletedState == true){

				// processing the message we read, need to reset readCompletedState
				// to indicate it had been processed
				sco->commState.readCompletedState = false;
				
				// remain in the same state
				// sco->commState.ostate = RECEIVE_SENSOR_DATA;
				
				// readState remains true, writeState is false
				sco->commState.readState = true;
				sco->commState.writeState = false;


				// extract the sensor id from the message received
				// data bytes one and two
				uint16_t sensorData;

				sensorData = (uint16_t)(((uint16_t)sco->commState.readCompletedBuffer[DATA_BYTE_TWO] << 8U) |
                                        ((uint16_t)sco->commState.readCompletedBuffer[DATA_BYTE_THREE] & 0xFF) );

				log_info("sensorData: %u", sensorData);

			}
			else{
				log_error("operational state: %s, readCompletedState is false, "
					"when it should have been true, data contents logged below", 
					debug_operational_state_string[sco->commState.ostate]);

				log_SensorCommOperation_data(sco);
			}
			break;

		case NOT_OPERATIONAL:

			log_fatal("NOT_OPERATIONAL not programmed, logging data contents below");
			log_SensorCommOperation_data(sco);

		break;
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
	// smallest possible fd is -1 for unopened ports
	int maxfd = -1;
	

	for(int i = 0; i < length; ++i){
		if( sensorCommArray[i].sensor.active == true &&
			sensorCommArray[i].commState.fd > maxfd){
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
void log_SensorCommOperation_data(const SensorCommOperation *sco)
{
	char hexReadBuf[3*READ_MESSAGE_LENGTH_BYTES+1];
	char hexReadCompletedBuf[3*READ_MESSAGE_LENGTH_BYTES+1];
	char hexWriteBuf[3*WRITE_MESSAGE_LENGTH_BYTES+1];

	convert_array_to_hex_string(hexReadBuf, 3*READ_MESSAGE_LENGTH_BYTES+1, 
		sco->commState.readBuffer, READ_MESSAGE_LENGTH_BYTES);

	convert_array_to_hex_string(hexReadCompletedBuf, 3*READ_MESSAGE_LENGTH_BYTES+1, 
		sco->commState.readCompletedBuffer, READ_MESSAGE_LENGTH_BYTES);

	convert_array_to_hex_string(hexWriteBuf, 3*WRITE_MESSAGE_LENGTH_BYTES+1, 
		sco->commState.writeBuffer, WRITE_MESSAGE_LENGTH_BYTES);


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

        sco->sensor.id, 
        sco->sensor.name,
        sco->sensor.active, 
        sco->sensor.devicePath,
        sco->sensor.baudRate,
        debug_operational_state_string[sco->commState.ostate],
        sco->commState.fd,
        debug_read_write_message_state_string[sco->commState.readIndex],
        sco->commState.readState,
        sco->commState.readCompletedState,
        debug_read_write_message_state_string[sco->commState.writeIndex],
        sco->commState.writeState,
        hexReadBuf,
        hexReadCompletedBuf,
        hexWriteBuf);

}