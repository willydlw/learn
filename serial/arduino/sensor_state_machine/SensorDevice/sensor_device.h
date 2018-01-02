#ifndef SENSOR_DEVICE_H
#define SENSOR_DEVICE_H

//#include "Arduino.h"
#include <stdint.h>



const int SENSOR_NAME_LENGTH = 16;
const int SENSOR_LIST_LENGTH = 5;


const uint8_t sensorIdList[SENSOR_LIST_LENGTH] = {0x00, 0x01, 0x02, 0x03, 0x04};

const char sensorNameList[SENSOR_LIST_LENGTH][SENSOR_NAME_LENGTH] = 
	{ "unused", "tachometer1", "tachometer2", "tachometer3", "tachometer4"};


struct Sensor{
	uint8_t sensor_id;
	char sensor_name[SENSOR_NAME_LENGTH];
};


enum class OperationalState{
	INITIAL, WAIT, SENSE, RESET, STOP};



void funa(void);


#endif
