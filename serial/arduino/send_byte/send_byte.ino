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
*            when 'r' is received, transitions to SEND_COUNT state.
*
*   SEND_COUNT: sends a count from 0 to 5, then transitions to END
*
*   RESET:   not yet implemented
*
*   END:     stops serial communication
*/
enum STATE { INITIAL = 1, WAIT = 2, SEND_COUNT = 3, RESET = 4, END = 5}; 

byte count;

enum STATE state;

void setup(){
  count = 0;
  Serial.begin(57600);
  state = INITIAL;
  delay(1000);
}

void loop(){
  switch(state){
    /* INITIAL state's purpose is to clean out garbage that seems 
       to be left in the serial buffer */
    case INITIAL:
      Serial.write('i');
      Serial.flush();
      state = WAIT;
    break;
    
    // waits to receive signal r for ready before proceeding to NORMAL state
    case WAIT:
      if(Serial.available() > 0){
        char readByte = Serial.read();
        if( readByte = 'r'){
          Serial.print('r');    // echo back to sender
          state = SEND_COUNT;
          delay(500);
        }
      }
      break;
      
    case SEND_COUNT:
      Serial.write(count);
      count++;
      if(count > 5){
        state = END;
      }
      delay(1000);
    break;
    
    case RESET:
    break;
    
    case END:
      Serial.end();
    break;  
  }
}
