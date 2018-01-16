#include <string.h>

#include "operations.h"


int initialize_sensor_communication_operations(
	const char* sensorInputFileName, 
	SensorCommOperation *sensorCommArray, int saLength,
	int* totalSensorCount, int* activeSensorCount)
{
	log_trace("function source code in progress");
	import_sensor_data( sensorInputFileName, sensorCommArray, saLength,
							totalSensorCount, activeSensorCount);

	return 1;
}


int import_sensor_data(const char* filename, SensorCommOperation *sensorCommArray, 
	int salength, int *totalSensorCount, int *activeSensorCount)
{
	FILE *ifp = NULL;
	int lineCount = 0;					// number of lines read from file

	// temporary storage, data read from file
	char name[SENSOR_NAME_LENGTH] = {'\0'};	
	char devicePath[SERIAL_DEV_PATH_LENGTH] = {'\0'};
	uint8_t id;
	int onOff;


	ifp = fopen(filename, "r");

	if(ifp == NULL){
		log_fatal("failed to open file %s", filename);
		return false;
	}

	// initialize counts
	*totalSensorCount = 0;
	*activeSensorCount = 0;

	// parse file to extract sensor name, id, active state
	//memset(name, 0, SENSOR_NAME_LENGTH);

	while(*totalSensorCount < salength && 
			fscanf(ifp, "%s%hhu%d%s", name, &id, &onOff, devicePath) == 4){
		
		++lineCount;

		// since id is an unsigned int, it will always be positive
		// no need to compare it to minimum sensor id which is 0
		if(id <= MAX_SENSOR_ID){
			sensorCommArray[id].sensor.id = id;
		}
		else{
			log_warn("input file: %s, line: %d, out of range sensor id %d",
						filename, lineCount, id);
			memset(name, 0, SENSOR_NAME_LENGTH);
			memset(devicePath, 0, SERIAL_DEV_PATH_LENGTH);
			continue;	// back to while test condition
		}

		// populate name, active data members
		strcpy(sensorCommArray[id].sensor.name, name);
		strcpy(sensorCommArray[id].sensor.devicePath, devicePath);
		sensorCommArray[id].sensor.active = onOff;

		// increment counts
		if(onOff == 1){
			*activeSensorCount = *activeSensorCount + 1;
		}

		*totalSensorCount = *totalSensorCount + 1;

		// clear array
		memset(name, 0, SENSOR_NAME_LENGTH);
	}


	fclose(ifp);
	ifp = NULL;

	log_info("totalSensorCount: %d, activeSensorCount: %d", *totalSensorCount, 
				*activeSensorCount);

	return 1;
}

/*
ErrorCondition perform_initialization_operations(const char* sensorDeviceInputFile)
{
	// import sensor device list, populate structs

	// intialize signal handler

	// initialize serial connections

	// set intial states
} */