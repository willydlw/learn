#ifndef OPERATIONS_H
#define OPERATIONS_H


#include "communication_state.h"
#include "sensor.h"

#define MIN_NUMBER_COMMAND_LINE_ARGS 	 2


#define SENSOR_INPUT_FILE_NAME_LENGTH 	64


typedef struct debug_stats_t{
	int read_select_zero_count;			/**<number of times select returns 0 when trying to read */
    int write_select_zero_count;		/**<number of times select returns 0 when trying to write */
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


void close_serial_connections(SensorCommOperation *sensorCommArray, 
		int totalSensorCount);

#endif