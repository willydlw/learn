#include <string.h>

#include "operations.h"
#include "serial.h"


int initialize_sensor_communication_operations(
	const char* sensorInputFileName, 
	SensorCommOperation *sensorCommArray, int saLength,
	int* totalSensorCount, int* activeSensorCount)
{
	int serialPortsOpened = 0;


	log_trace("function source code in progress");
	if( import_sensor_data( sensorInputFileName, sensorCommArray, saLength,
							totalSensorCount, activeSensorCount) == 0)
	{
		log_fatal("import_sensor_data returned 0");
		return -1;
	}


	int unopenedList[*activeSensorCount];
	int unopenedIndex = 0;

	// open serial connection for active sensors
	for(int i = 0; i < *totalSensorCount; ++i){
		if(sensorCommArray[i].sensor.active){
			sensorCommArray[i].sensor.fd = 
				serial_init(sensorCommArray[i].sensor.devicePath, 
					sensorCommArray[i].sensor.baudRate);

			if(sensorCommArray[i].sensor.fd != -1){
				++serialPortsOpened;
			}
			else{
				// add unopened devices to list
				unopenedList[unopenedIndex] = i;
				++unopenedIndex;
			}

		}
	}

	handle_failed_serial_connections(sensorCommArray, unopenedList, 
		unopenedIndex);

	// now initialize the communication states 
	initialize_communication_states(sensorCommArray, *totalSensorCount);

	return serialPortsOpened;
}




void handle_failed_serial_connections(SensorCommOperation *sensorCommArray,
		int *unopenedList, int numUnopened)
{
	log_warn("TODO: function has not been coded to handle unopened serial connections");
	if(numUnopened == 0){
		log_info("all serial connections for active devices opened");
		return;
	}

	for(int i = 0; i < numUnopened; ++i){
		log_error("failed to open senor: %s, device path %s", 
			sensorCommArray[unopenedList[i]].sensor.name,
			sensorCommArray[unopenedList[i]].sensor.devicePath);
		log_error("finish function by writing code to search dev directory"
					" and try connecting to those tty devices\n");
	}
	
}



void initialize_communication_states(SensorCommOperation *sensorCommArray, 
	int salength)
{
	for(int i = 0; i < salength; ++i){
		if(sensorCommArray[i].sensor.active && 
			sensorCommArray[i].sensor.fd != -1){

			sensorCommArray[i].commState.ostate = WAIT_FOR_CONNECTION;
			sensorCommArray[i].commState.messageState = AWAITING_START_MARKER;
			sensorCommArray[i].commState.readState = true;
			sensorCommArray[i].commState.writeState = false;
		}
		else{
			sensorCommArray[i].commState.ostate = NOT_OPERATIONAL;
			sensorCommArray[i].commState.messageState = NOT_COMMUNICATING;
			sensorCommArray[i].commState.readState = false;
			sensorCommArray[i].commState.writeState = false;
		}
	}
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
	int baudRate;


	ifp = fopen(filename, "r");

	if(ifp == NULL){
		log_fatal("failed to open file %s", filename);
		return 0;
	}

	// initialize counts
	*totalSensorCount = 0;
	*activeSensorCount = 0;

	// parse file to extract sensor name, id, active state
	//memset(name, 0, SENSOR_NAME_LENGTH);

	while(*totalSensorCount < salength && 
			fscanf(ifp, "%s%hhu%d%s%d", name, &id, &onOff, 
					devicePath, &baudRate) == 5){
		
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

		// populate data members
		sensorCommArray[id].sensor.active = onOff;
		sensorCommArray[id].sensor.baudRate = baudRate;		
		strcpy(sensorCommArray[id].sensor.name, name);
		strcpy(sensorCommArray[id].sensor.devicePath, devicePath);

		// initialize to invalid file descriptor
		// this field will be populated in another function
		sensorCommArray[id].sensor.fd = -1;		


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


int find_largest_fd(const SensorCommOperation *sensorCommArray, 
		int totalSensorCount)
{
	int maxfd = -1;
	for(int i = 0; i < totalSensorCount; ++i){
		if(sensorCommArray[i].sensor.fd > maxfd){
			maxfd = sensorCommArray[i].sensor.fd;
		}
	}
	return maxfd;
}


void close_serial_connections(SensorCommOperation *sensorCommArray, 
		int totalSensorCount)
{
	for(int i = 0; i < totalSensorCount; ++i){
		if(sensorCommArray[i].sensor.fd != -1){
			serial_close(sensorCommArray[i].sensor.fd);
		}
	}
}




void log_SensorCommOperation_data(const SensorCommOperation *sensorCommArray, int length)
{
	for(int i = 0; i < length; ++i){
        log_trace("id: %d, name: %s, active: %d\n"
        	"device path: %s, baud rate: %d, fd: %d\n"
        	"ostate: %s, messageState: %s\n"
        	"readState: %d, writeState: %d",
            sensorCommArray[i].sensor.id, sensorCommArray[i].sensor.name,
            sensorCommArray[i].sensor.active, 
            sensorCommArray[i].sensor.devicePath,
            sensorCommArray[i].sensor.baudRate,
            debug_operational_state_string[sensorCommArray[i].commState.ostate],
            debug_message_state_string[sensorCommArray[i].commState.messageState],
            sensorCommArray[i].commState.readState,
            sensorCommArray[i].commState.writeState);
    }
}