#include <sensor_device.h>
#include <sensor_communication.h>

/** Description: Serially transmits a count that ranges from 0 to 5.
*
*   State machine implementation.
*
*   States:
*
*   INITIAL: purpose is to transmit any garbage characters 
*            in the transmit buffer. Often at startup, there
*            is garbage in the buffer.
*
*   WAIT:    waits to receive the letter r 
*            when 's' is received, transitions to SEND_COUNT state.
*            when 'r' is received, 
*
*   SEND_COUNT: sends a count from 0 to 5, then transitions to END
*
*   RESET:   resets count to zero, next state is INITIAL
*
*   END:     stops serial communication
*/


/*** state  ***/
ReceiveMessageState rxmsgState;
OperationalState operateState;

/*** state change flags  **/
boolean operationalStateChange;
boolean messageReceived;

// note that String is an Arduino data type and does not
// have all of the same functions as the c++ string type.
String inString;

/**** sensor ****/
Sensor sensor; 

/**** timing ****/
const unsigned long WAIT_READY_DELAY = 3000;    // milliseconds
const unsigned long WAIT_STATE_DELAY = 1000;    // milliseconds
const unsigned long SENSE_DELAY = 1000;         // milliseconds

unsigned long startTime;


// index 0 start marker
// index 1,2 device id
// index 3, 4 reserved for 2 data bytes
// index 5  end marker
byte sendDataBuffer[5];  



/*** for this example, the sensor data will be a 
 *   sequential count
 */
unsigned int count;



void setup(){

  Serial.begin(9600);         // enable serial communication

  // set operational state, state flag
  operateState = OperationalState::INITIAL;
  operationalStateChange = false;

  // initialize received message state, state flag
  rxmsgState = ReceiveMessageState::AWAIT_RECEIPT;
  messageReceived = false;

  // reserve enough memory to store message
  inString.reserve(MAX_RECEIVE_MESSAGE_LENGTH);
  inString = "";              // initialize to empty string

  // initialize sensor fields
  sensor.sensor_id = sensorIdList[1];
  strcpy(sensor.sensor_name, sensorNameList[1]);


  // for this example, this is a quick hack to form
  // the sensor data message
  sendDataBuffer[0] = startMarker;
  sendDataBuffer[1] = sensor.sensor_id;
  sendDataBuffer[4] = endMarker;

  count = 0;

  /* Debug
  Serial.print("sensor id: ");
  Serial.println(sensor.sensor_id);
  Serial.print("sensor name: ");
  Serial.println(sensor.sensor_name);
  */
  delay(100);
  Serial.write(helloMessage);
  startTime = millis();

 }


void process_received_message(){
  /*Serial.print("inString: ");
  Serial.print(inString);
  Serial.print(", readyQuery: ");
  Serial.println(readyQuery);
  */
  
  if(inString.compareTo(readyCommand) == 0){
    if(operateState == OperationalState::INITIAL){
      Serial.write(ackResponse);
      operateState = OperationalState::WAIT;
    }
    else{
      Serial.write(nackResponse);
    }
  }
  else if(inString.compareTo(resetCommand) == 0){
    operateState == OperationalState::RESET;
  }
  else if(inString.compareTo(stopCommand) == 0){
    operateState == OperationalState::STOP;
  }

  startTime = millis();
}


void loop(){

  if(messageReceived){
    //Serial.println("message received");
    process_received_message();
    messageReceived = false;
  }
    
 
  switch(operateState){
 
    /* INITIAL state's purpose is to wait for ready 
     *  signal from serial connected program
     */
    case OperationalState::INITIAL:
      // stays here until ready query is received
      // and ack response is sent
      if( (millis() - startTime) > WAIT_READY_DELAY){
        Serial.write(helloMessage);
        startTime = millis();
      }
      
    break;
    
    case OperationalState::WAIT:
      if( (millis() - startTime) > WAIT_STATE_DELAY){
        operateState = OperationalState::SENSE;
        startTime = millis();
      } 
    break;
    
    case OperationalState::SENSE:
      if( (millis() - startTime) > SENSE_DELAY){
        // simulate sensor data with a count
        sendDataBuffer[2] = (byte)(count >> 8);  // msb
        sendDataBuffer[3] = (byte)count;         // lsb
    
        Serial.write(sendDataBuffer, 5);
      
        count++;
        startTime = millis();
      }    
    break;
   
    case OperationalState::RESET:
      count = 0;
      operateState = OperationalState::INITIAL;
      startTime = millis();
    break;

    case OperationalState::STOP:
      Serial.end();
    break;
  }
   
}

/*
  Arduino environment automatically calls this function, right after loop
  finishes execution when serial data has been received. The hidden Arduino
  main executes as shown below.

   int main(void){
    init();
    setup();
    
    for (;;) {
        loop();
        if (serialEventRun) serialEventRun();
    }
        
    return 0;
  }  
*/

void serialEvent(){
  /* There may be multiple bytes available to be read.
   * Our receieved messages are more than one byte, therefore,
   * this function is coded to read an entire message if
   * all of the message bytes from start to end have been received.
   * 
   * In the event, that a message has only been partially received,
   * the receive message state is maintained to ensure proper
   * extrapolation of the data.
   */
  while(Serial.available()){
    
    char readByte = (char)Serial.read();  // read returns int

    //Serial.println(readByte);
    
    switch(rxmsgState){
      
      case ReceiveMessageState::AWAIT_RECEIPT:
        if(readByte == startMarker){ // no reason to save readByte
          inString = "";            // clear string
          rxmsgState = ReceiveMessageState::FORM_STRING;
          inString += startMarker;
          rxmsgState = ReceiveMessageState::FORM_STRING;
        }
        break;
        
      case ReceiveMessageState::FORM_STRING:
      
        // increase string length if message is too long
        // this should not happen, but let's not throw away
        // the message in case we have a programming error
        // related to string length. 
        // The other possibility is that we made an error 
        // when creating the message sent and that it will
        // not match any valid messages.
        
        if(inString.length() >= MAX_RECEIVE_MESSAGE_LENGTH){
          inString.reserve(inString.length() + 1);
        }

        inString += readByte;
        
        if(readByte == endMarker){
          rxmsgState = ReceiveMessageState::RECEIPT_COMPLETE;
          messageReceived = true;
        }
        break;
        
        default:
          // should not reach this case
          // reset to await start state, we lose any information
          // already read
          // inString will be cleared in AWAIT_START
          rxmsgState = ReceiveMessageState::AWAIT_RECEIPT;
      } // end switch

    if(messageReceived){
      // break out of while loop before reading
      // any more data. Reading more data may mean
      // message will be overwritten before it
      // is processed.
      break;
    }
  } // end while
}  // end serialEvent

