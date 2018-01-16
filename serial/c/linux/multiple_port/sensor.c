
#include <string.h>				// memset, strcpy
#include <debuglog.h>

#include "sensor.h"



const uint8_t sensorIdList[SENSOR_LIST_LENGTH] = {0x00, 0x01, 0x02, 0x03, 0x04};


const char sensorNameList[SENSOR_LIST_LENGTH][SENSOR_NAME_LENGTH] = 
	{ "sensor0", "sensor1", "sensor2", "sensor3", "sensor4"};


