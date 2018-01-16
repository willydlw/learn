#include <stdint.h>			// uint8_t
#include <stdio.h>			// FILE, fscanf 
#include <debuglog.h>


#include "../sensor_device.h"

/* argv[1] is name of sensor initialization file
*/




int main(int argc, char **argv)
{
	

	int totalSensorCount = 0;
	int activeSensorCount = 0;
	

	Sensor sensorArray[SENSOR_LIST_LENGTH];

	// console logging: show all messages
	// file logging: do not log
	// color on 
	log_init(LOG_TRACE, LOG_OFF, 1);

	if(argc < 2){
		log_fatal("usage: a.out sensorList.txt\n");
		return 1;
	}


	if( import_sensor_data(argv[1], sensorArray, SENSOR_LIST_LENGTH,
				&totalSensorCount, &activeSensorCount) == false){
		log_fatal("initialize_sensor_array_data failed, program terminating");
		return 1;
	}
	

	return 0;
}
