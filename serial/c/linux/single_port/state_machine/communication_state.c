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
	
	fprintf(stderr, "\nstart of %s, message state: %s\n", __FUNCTION__, message_state_string[msgState]);

	ssize_t i;
	for(i = 0; i < bytes_read; ++i){

		fprintf(stderr, "start of loop, message state: %s, buf[%ld]: %#x\n", message_state_string[msgState],
					i, buf[i]);

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
				fprintf(stderr, "end marker recognized, complete message processed\n");
				fprintf(stderr, "time to alert program that new sensor data is available\n");

				// calling function here to process the new data received and then
				// set the state to AWAITING_START_MARKER in the case where we are
				// processing the end of one message and have the start of another
				// message in the buffer

				// It is tempting to create a NEW_DATA_RECEIVED state and return
				// that state right here. That would create a problem when there
				// are additional bytes in the buffer. Would have to alter the 
				// code to ensure these additional bytes are handled and not lost.
				// Still working out the best way to implement this. Will depend on
				// how rapidly sensor data must be processed and how much data.
				process_data_received(*sensorData);
				msgState = AWAITING_START_MARKER;
			}
			else { // log error
				fprintf(stderr, "error: %s, state: AWAITING_START_MARKER, buf[%ld]: %#x\n",
						__FUNCTION__, i, buf[i]);
			}
			break;
		default:
			fprintf(stderr, "\nerror: %s, reached default case\n", __FUNCTION__);
			fprintf(stderr, "current message state: %s, processing byte, buf[%ld]: %#x\n",
								message_state_string[msgState], i, buf[i]);
			fprintf(stderr, "setting message state to %s, some data will be lost\n", 
							message_state_string[AWAITING_START_MARKER]);
			fprintf(stderr, "will also cause AWAITING_START_MARKER error until next start marker is received\n");
			msgState = AWAITING_START_MARKER;

		}

	}

	fprintf(stderr, "exiting %s, returning message state: %s\n", __FUNCTION__, message_state_string[msgState]);

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
	fprintf(stderr, "\nentering %s, theData: %u\n", __FUNCTION__, theData);
	fprintf(stderr, "TODO: this is only a stub function, need to determine how to handle"
					" real time data updates\n");
}
