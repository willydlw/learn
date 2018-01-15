#ifndef SENSOR_DEVICE_H
#define SENSOR_DEVICE_H

#include <stdbool.h>
#include <stdint.h>



#define SENSOR_NAME_LENGTH 	24
#define SENSOR_LIST_LENGTH 	 5
#define MAX_SENSOR_ID		 SENSOR_LIST_LENGTH - 1



extern const uint8_t sensorIdList[SENSOR_LIST_LENGTH]; 

extern const char sensorNameList[SENSOR_LIST_LENGTH][SENSOR_NAME_LENGTH]; 


typedef struct sensor_t{
	uint8_t id;
	char name[SENSOR_NAME_LENGTH];

	int active;
	int fd;
}Sensor;



bool initialize_sensor_array_data(const char* filename, Sensor *sensorArray, int salength,
	int *totalSensorCount, int *activeSensorCount);




#endif
