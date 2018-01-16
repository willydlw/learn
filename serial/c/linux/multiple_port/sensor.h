#ifndef SENSOR_H
#define SENSOR_H

#include <stdbool.h>
#include <stdint.h>


#define SERIAL_DEV_PATH_LENGTH 	32
#define SENSOR_NAME_LENGTH 		24
#define SENSOR_LIST_LENGTH 	 	5
#define MAX_SENSOR_ID		 	SENSOR_LIST_LENGTH - 1



extern const uint8_t sensorIdList[SENSOR_LIST_LENGTH]; 

extern const char sensorNameList[SENSOR_LIST_LENGTH][SENSOR_NAME_LENGTH]; 


typedef struct sensor_t{
	uint8_t id;
	char name[SENSOR_NAME_LENGTH];

	int active;
	
	char devicePath[SERIAL_DEV_PATH_LENGTH];
}Sensor;



bool import_sensor_data(const char* filename, Sensor *sensorArray, int salength,
	int *totalSensorCount, int *activeSensorCount);




#endif
