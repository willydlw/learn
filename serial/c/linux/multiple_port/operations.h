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
		int *unopenedList, int numUnopened);


void initialize_communication_states(SensorCommOperation *sensorCommArray, 
	int totalSensorCount);


int find_largest_fd(const SensorCommOperation *sensorCommArray, 
		int totalSensorCount);


void build_fd_sets(SensorCommOperation *sensorCommArray, int length, int *readCount, 
	int* writeCount, fd_set *readfds, fd_set *writefds);


uint32_t read_fdset(SensorCommOperation *sensorCommArray, int length, fd_set *readfds);


ReadWriteMessageState process_received_message_bytes(uint8_t *destination,
	uint8_t *source, ssize_t bytesRead, ReadWriteMessageState readIndex, bool *completedFlag);

void process_completed_messages(SensorCommOperation *sensorCommArray, 
		int length, uint32_t completedList);


void close_serial_connections(SensorCommOperation *sensorCommArray, 
		int totalSensorCount);


void log_SensorCommOperation_data(const SensorCommOperation *sensorCommArray, 
		int length);

#endif