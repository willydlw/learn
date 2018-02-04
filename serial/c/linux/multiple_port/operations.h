/**
 * Copyright (c) 2017 willydlw
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See `main.c` for details.
 */


/**@file operations.h
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

#ifndef OPERATIONS_H
#define OPERATIONS_H

#include <stdbool.h>

#include "communication_state.h"
#include "sensor.h"



#define MIN_NUMBER_COMMAND_LINE_ARGS 	 2		/**< minimum number of command line arguments */

#define SENSOR_INPUT_FILE_NAME_LENGTH 	64		/**< maximum string length, sensor input file name */


/**
* @brief Debugging Statistics
*/
typedef struct debug_stats_t{

	int totalSensorCount;				/**< total number of sensors listed in input file */
    int activeSensorCount;				/**< total number of sensors listed as active */

	int selectZeroCount;				/**< number of times select returns 0  */
	int selectFailureCount;				/**< number of times select returns failure condition */

    int sensorIdMismatchCount;			/**< number of times sensor id received in a message
   											 does not match the registered id */

	int serialPortsOpened;				/**< number of serial connections successfully openend */

    uint32_t activeSensorList;			/**< bits set to 1 represent an active sensor
    										 bit position corresponds to sensor id */

    uint32_t registeredSensorList;		/**< bits set to 1 represent sensor that has
                                             completed the sensor registration state */
}DebugStats;



/**
* @brief Sensor Communication Operation
*        defined by sensor object and
*        its communication state.
*/
typedef struct sensor_comm_operation_t{
	Sensor sensor;
	CommState commState;
}SensorCommOperation;




/* @brief Initialize sensor communications array
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
	SensorCommOperation *sensorCommArray, 
	int saLength,
	DebugStats *debugStats);



/* @brief Populates sensor device data from input file
*
*  Input data file should contain
*  		sensor name  	sensor id 	active state  	device path
*
*  in that order, separated by white space.
*
*  This data is stored in the sensorCommArray data members.
*  
*  The debugStats data member totalSensorCount contains the
*  count of the total number of sensors.
*
*  The activeSensorCount is the total number of sensors
*  read as active.
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
bool import_sensor_data(
	const char* filename, 
	SensorCommOperation *sensorCommArray, 
	int salength, 
	DebugStats *debugStats);



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
* @TODO: write routine to attempt to open active devices that failed
*	    to open serial connection
*
*/
void handle_failed_serial_connections(SensorCommOperation *sensorCommArray,
		int *unopenedList, int numUnopened, int *activeSensorCount);



/** @brief Initializes the commState data members
*
*\par State after initialization
*
*	ostate 			active 			WAIT_FOR_CONNECTION
*					inactive        NOT_OPERATIONAL
*
*	readState 		active, true
*					inactive, false 
*
*	writeState              false
*   readCompletedState   	false
*   writeCompletedState     false  
*   readIndex 				START_MARKER
*   writeIndex 				START_MARKER 
*	
*
*
* @param[out] sensorCommArray 	array of sensor device data structures
* @param[in] salength 			number of elements in sensorCommArray
*
* @return void
*
* @note: Do not call this function before establishing serial connection.
*        Function tests for file descriptor != -1
*/
void initialize_communication_states(SensorCommOperation *sensorCommArray, 
	int totalSensorCount);



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
		int totalSensorCount);



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
	int* writeCount, fd_set *readfds, fd_set *writefds);



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
uint32_t read_fdset(SensorCommOperation *sensorCommArray, int length, fd_set *readfds);



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
uint32_t write_fdset(SensorCommOperation *sensorCommArray, int length, fd_set *writefds);



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
	uint8_t *source, ssize_t slength, ReadWriteMessageState readIndex, bool *completedFlag);




void process_operational_state(SensorCommOperation *sco, DebugStats *debugStats);


/* @brief Closes all open file descriptors in the sensor array
*    
*
* @param[in] sensorCommArray		array of sensor communication operation structures
* @param[in] length                 number of elements in sensorCommArray
*
* @return void
*/
void close_serial_connections(SensorCommOperation *sensorCommArray, 
		int length);



/* @brief Logs the state of every data member in the sensorCommOperation object
*         
*		  Log trace level
*
*         Buffer contents are written as a hexadecimal string
*
* @param[in] sensorCommArray		sensor communication operation structure
*
* @return void
*/
void log_SensorCommOperation_data(const SensorCommOperation *sco);

#endif