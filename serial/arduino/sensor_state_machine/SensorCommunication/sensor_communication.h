#ifndef SENSOR_COMMUNICATION_H
#define SENSOR_COMMUNICATION_H

const uint8_t MAX_RECEIVE_MESSAGE_LENGTH = 16;

// request messages
const char* resetCommand = "<RST>";
const char* stopCommand = "<STP>";


/* When another program opens the serial
    connection, the Arduino program resets.

    This message is broadcast to confirm
    the connection has been made.
*/
const char* helloMessage = "<HLO>";


/* Wait to receive this message after
    hello message has been transmitted
*/
const char* ackMessage = "<ACK>";


/* Wait to receive this message after
    sensor id has been transmitted
*/
const char* readyMessage = "<RDY>";



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


#endif
