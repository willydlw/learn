#ifndef OPERATIONS_H
#define OPERATIONS_H


#include "communication_state.h"
#include "sensor.h"

#define MIN_NUMBER_COMMAND_LINE_ARGS 	 2


#define SENSOR_INPUT_FILE_NAME_LENGTH 	64


typedef struct debug_stats_t{
	int select_zero_count;				/**<number of times select returns 0  */
    int sensor_id_mismatch_count;		/**<number of times sensor id received in a message
    										does not match the registered id */
    int default_comm_read_state_count;
    int default_comm_write_state_count;
}DebugStats;


typedef struct sensor_comm_operation_t{
	Sensor sensor;
	CommState commState;
}SensorCommOperation;



int initialize_sensor_communication_operations(
	const char* sensorInputFileName, 
	SensorCommOperation *sensorCommArray, int saLength,
	int* totalSensorCount, int* activeSensorCount);

int import_sensor_data(const char* filename, SensorCommOperation *sensorCommArray, 
	int salength, int *totalSensorCount, int *activeSensorCount);


void handle_failed_serial_connections(SensorCommOperation *sensorCommArray,
		int *unopenedList, int numUnopened, int *activeSensorCount);


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


void process_operational_state(SensorCommOperation *sco);


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