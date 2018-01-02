#ifndef COMMUNICATION_STATE_H
#define COMMUNICATION_STATE_H

#include <stdbool.h>

#define NUM_MESSAGE_STATES 7

#define MAX_WRITE_TRIES 3


// Globals must be initialized in the c file and declared extern in the h file
extern const char* debug_comm_state_string[];

extern const char* comm_state_string[];

extern const char* message_state_string[];



typedef enum error_conditions_t { 
	SUCCESS, SELECT_ERROR, SERIAL_WRITE_ERROR, SERIAL_READ_ERROR,
	CONNECTION_ERROR, NACK_ERROR, UNEXPECTED_RESPONSE_ERROR,
	LOOP_COUNT_ERROR, ACKNOWLEDGEMENT_ERROR } ErrorCondition;


typedef enum comm_state_t { 

	WAIT_FOR_CONNECTION = 1,
	SEND_READY_SIGNAL = 2,
	READ_SENSOR = 3, 
	SEND_RESET = 4, 
	SEND_STOP = 5
}CommState;



typedef enum message_state_t { AWAITING_START_MARKER, AWAITING_SENSOR_ID, AWAITING_DATA_BYTE_ONE,
								AWAITING_DATA_BYTE_TWO, AWAITING_DATA_BYTE_THREE,
								AWAITING_END_MARKER, MESSAGE_COMPLETE} MessageState;





ErrorCondition confirm_connection(int fd, int max_fd);


ErrorCondition send_ready_signal(int fd, int max_tries);

ErrorCondition wait_for_acknowledgement(int fd, int max_fd);


MessageState process_received_ack_bytes(MessageState msgState, const uint8_t *buf, ssize_t bytes_read,
										char *responseData);


MessageState process_received_data_bytes(MessageState msgState, const uint8_t *buf, ssize_t bytes_read,
										uint16_t *sensorData);


bool valid_sensor_id(uint8_t id);


void process_data_received(uint16_t theData);


#endif