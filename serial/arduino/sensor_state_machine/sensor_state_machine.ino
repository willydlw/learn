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

/*



const uint8_t sensorId = 0x01;



// index 0 start marker
// index 1,2 device id
// index 3, 4 reserved for 2 data bytes
// index 5  end marker
byte sendDataBuffer[5];  


*/





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

 }


void process_received_message(){
  
  if(inString == readyQuery){
    if(operateState == OperationalState::INITIAL){
      Serial.write(ackResponse);
      operateState = OperationalState::WAIT;
    }
    else{
      Serial.write(nackResponse);
    }
  }
}


void loop(){

  if(messageReceived){
    process_received_message();
    messageReceived = false;
  }
  
 /*
  switch(operateState){
  */  
    /* INITIAL state's purpose is to wait for ready 
     *  signal from serial connected program
     *
    case OperationalState::INITIAL:
      if(stateChange){
        state = WAIT_FOR_GO;
        stateChange = false;
      }
    break;
    
    case SEND_COUNT:
      
      if(count < MAX_COUNT){
        Serial.write(count);
        count++;
        delay(100);            // transmit 10 bytes per second
      }
      else{  // count >= MAX_COUNT
        stateChange = true;
        nextState = END;
      }
    break;
    
    case RESET:
      count = 0;
      state = INITIAL;
    break;
    
    case END:
      Serial.end();
    break;  
  }

  */
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
    
    switch(rxmsgState){
      
      case ReceiveMessageState::AWAIT_RECEIPT:
        if(readByte == startMarker){ // no reason to save readByte
          inString = "";            // clear string
          inString = startMarker;  
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
          // should not reach this case, clear input string
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

