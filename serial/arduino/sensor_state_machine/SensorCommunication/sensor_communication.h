#ifndef SENSOR_COMMUNICATION_H
#define SENSOR_COMMUNICATION_H

const uint8_t MAX_RECEIVE_MESSAGE_LENGTH = 16;

// request messages
const char* readyCommand = "<RDY>";
const char* resetCommand = "<RST>";
const char* stopCommand = "<STP>";

// response messages
const char* ackResponse = "<ACK>";
const char* nackResponse = "<NCK>";


/** When another program opens the serial
    connection, the Arduino program resets.

    This message is broadcast to confirm
    the connection has been made.
**/
const char* helloMessage = "<HLO>";



// message characteristics
const char startMarker = '<';
const char endMarker = '>';

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
