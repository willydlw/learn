#include <stdio.h>
#include <string.h>


#include "serial.h"
#include "communication_state.h"

void send_ready_signal(int fd){

	const static ssize_t expected_bytes = (ssize_t)strlen(comm_state_string[SIGNAL_READY]);

    ssize_t bytes_written = 0;

    bytes_written = serial_write(fd, comm_state_string[SIGNAL_READY], 
    								expected_bytes) ;

    // keep sending until all bytes have been transmitted
    while( bytes_written!= expected_bytes){
        perror("send_ready_signal: not all bytes written ");
        fprintf(stderr, "bytes_written: %ld\n", bytes_written);
        bytes_written = serial_write(fd, comm_state_string[SIGNAL_READY], 
        								expected_bytes) ;
    }

}


MessageState process_received_bytes(MessageState msgState, const uint8_t *buf, ssize_t bytes_read,
										uint16_t *sensorData)
{

	ssize_t i;
	for(i = 0; i < bytes_read; ++i){

		switch(msgState){
		case AWAITING_START_MARKER:
			if(buf[i] == start_marker){
				msgState = AWAITING_SENSOR_ID;
			}
			else { // log error
				fprintf(stderr, "error: %s, state: AWAITING_START_MARKER, buf[%ld]: %#x\n",
						__FUNCTION__, i, buf[i]);
			}
			break;
		case AWAITING_SENSOR_ID:
			if(valid_sensor_id(buf[i])){
				msgState = AWAITING_DATA_BYTE_ONE;
			}
			else { // log error
				fprintf(stderr, "error: %s, state: AWAITING_SENSOR_ID, invalid id: buf[%ld]: %#x\n",
						__FUNCTION__, i, buf[i]);
			}
			break;

		case AWAITING_DATA_BYTE_ONE:
			// assumes MSB received first
			*sensorData = (uint16_t)(((uint16_t)buf[i]) << 8);
			msgState = AWAITING_DATA_BYTE_TWO;
			break;

		case AWAITING_DATA_BYTE_TWO:
			*sensorData = (uint16_t) (*sensorData | (uint16_t)buf[i]);
			msgState = AWAITING_END_MARKER;
			break;

		case AWAITING_END_MARKER:
			if(buf[i] == end_marker){

				process_data_received(*sensorData);
				msgState = AWAITING_START_MARKER;
			}
			else { // log error
				fprintf(stderr, "error: %s, state: AWAITING_START_MARKER, buf[%ld]: %#x\n",
						__FUNCTION__, i, buf[i]);
			}
			break;
		default:
			fprintf(stderr, "error: %s, reached default case\n", __FUNCTION__);

		}

	}

	return msgState;
}
	


bool valid_sensor_id(uint8_t id){
	// @TODO: need a scheme for quickly validating sensor ids
	//        once multiple sensors are handled
	if(id == sensor_id[0])
		return true;
	else 
		return false;
}


void process_data_received(uint16_t theData)
{
	fprintf(stderr, "process_data_received stub function needs to be completed\n");
	fprintf(stderr, "theData: %u\n", theData);
}
