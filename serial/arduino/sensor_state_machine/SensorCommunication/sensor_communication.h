#ifndef SENSOR_COMMUNICATION_H
#define SENSOR_COMMUNICATION_H

const uint8_t MAX_RECEIVE_MESSAGE_LENGTH = 16;

// request messages
const char* readyQuery = "<RDY>";

// response messages
const char* ackResponse = "<ACK>";
const char* nackResponse = "<NCK>";



// message characteristics
const uint8_t startMarker = '<';
const uint8_t endMarker = '>';

struct SensorMessage{
	uint8_t start;
	uint8_t id;
	uint8_t data[2];
	uint8_t end;
};


enum class ReceiveMessageState{ 	

				AWAIT_RECEIPT, 
				FORM_STRING, 
				RECEIPT_COMPLETE
};


enum class SENSOR_COMM_STATE{  

				INITIAL = 1, 
				WAIT = 2, 
				SEND_COUNT = 3, 
				RESET = 4, 
				END = 5
};

#endif