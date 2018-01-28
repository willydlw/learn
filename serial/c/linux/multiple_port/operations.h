#ifndef OPERATIONS_H
#define OPERATIONS_H

#include <stdbool.h>

#include "communication_state.h"
#include "sensor.h"


// macro constants
#define MIN_NUMBER_COMMAND_LINE_ARGS 	 2

#define SENSOR_INPUT_FILE_NAME_LENGTH 	64



typedef struct debug_stats_t{

	int totalSensorCount;				/**<total number of sensors listed in input file */
    int activeSensorCount;				/**<total number of sensors listed as active */

	int selectZeroCount;				/**<number of times select returns 0  */
	int selectFailureCount;				/**<number of times select returns failure condition */
    int sensorIdMismatchCount;			/**<number of times sensor id received in a message
    
    										does not match the registered id */

	int serialPortsOpened;				/**<number of serial connections successfully openend */
    
    //int default_comm_read_state_count;
    //int default_comm_write_state_count;

    uint32_t activeSensorList;			/**<bits set to 1 represent an active sensor
    										bit position corresponds to sensor id */
    uint32_t registeredSensorList;		/**<bits set to 1 represent sensor that has
                                            completed the sensor registration state */
}DebugStats;




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
* TODO: write routine to attempt to open active devices that failed
*	    to open serial connection
*
*/
void handle_failed_serial_connections(SensorCommOperation *sensorCommArray,
		int *unopenedList, int numUnopened, int *activeSensorCount);



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


void build_fd_sets(SensorCommOperation *sensorCommArray, int length, int *readCount, 
	int* writeCount, fd_set *readfds, fd_set *writefds);


uint32_t read_fdset(SensorCommOperation *sensorCommArray, int length, fd_set *readfds);


uint32_t write_fdset(SensorCommOperation *sensorCommArray, int length, fd_set *writefds);


ReadWriteMessageState process_received_message_bytes(uint8_t *destination,
	uint8_t *source, ssize_t bytesRead, ReadWriteMessageState readIndex, bool *completedFlag);


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