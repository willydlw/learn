#ifndef COMMUNICATION_STATE_H
#define COMMUNICATION_STATE_H

#include <stdbool.h>


typedef enum comm_state_t { SEND_READY_QUERY, WAIT_FOR_ACK, WAIT_FOR_DATA} CommState;


static const char* comm_state_string[] = {"<RDY>", "<ACK>", "<NCK>"};


static const uint8_t start_marker = '<';
static const uint8_t end_marker = '>';

static const uint8_t sensor_id[1] = {'1'};




typedef enum message_state_t { AWAITING_START_MARKER, AWAITING_SENSOR_ID, AWAITING_DATA_BYTE_ONE,
								AWAITING_DATA_BYTE_TWO, AWAITING_DATA_BYTE_THREE,
								AWAITING_END_MARKER, MESSAGE_COMPLETE} MessageState;


static const char* message_state_string[] = 
		{ "AWAITING_START_MARKER", "AWAITING_SENSOR_ID", "AWAITING_DATA_BYTE_ONE",
		  "AWAITING_DATA_BYTE_TWO", "AWAITING_DATA_BYTE_THREE", "AWAITING_END_MARKER", "MESSAGE_COMPLETE"};




int send_ready_signal(int fd, int max_tries);


MessageState process_received_ack_bytes(MessageState msgState, const uint8_t *buf, ssize_t bytes_read,
										char *responseData);


MessageState process_received_data_bytes(MessageState msgState, const uint8_t *buf, ssize_t bytes_read,
										uint16_t *sensorData);


bool valid_sensor_id(uint8_t id);


void process_data_received(uint16_t theData);


#endif