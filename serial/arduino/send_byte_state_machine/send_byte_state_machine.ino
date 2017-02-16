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
enum STATE { INITIAL = 1, WAIT = 2, SEND_COUNT = 3, RESET = 4, END = 5}; 


const unsigned int MAX_COUNT = 25;
byte count;
int waitCount = 0;

enum STATE state, nextState;
bool stateChange;

void setup(){
  count = 0;
  stateChange = false;
  state = INITIAL;
  nextState = INITIAL;
  Serial.begin(9600);         // enable serial communication
  delay(1000);
}

void loop(){
  
  if(stateChange){
    state = nextState;
    stateChange = false;
  }
  
  switch(state){
    /* INITIAL state's purpose is to clean out garbage that seems 
       to be left in the serial buffer */
    case INITIAL:
      Serial.write('i');
      Serial.flush();
      stateChange = true;
      nextState = WAIT;
    break;
    
    // waits for a state change to occur, does nothing
    case WAIT:
      ;             // do nothing
      break;
      
    case SEND_COUNT:
      
      if(count < MAX_COUNT){
        Serial.write(count);
        count++;
        delay(100);            // transmit 1 byte per second
      }
      else{  // count >= MAX_COUNT
        stateChange = true;
        nextState = END;
      }
    break;
    
    case RESET:
      count = 0;
      stateChange = true;
      nextState = INITIAL;
    break;
    
    case END:
      Serial.end();
    break;  
  }
}


void serialEvent(){
  if(Serial.available() > 0){
    int readByte = Serial.read();
    switch(readByte){
      case 'r':
        if(state != RESET){
          nextState = RESET;
          stateChange = true;
        }
        break;
      case 's':
        if(state != SEND_COUNT){
          nextState = SEND_COUNT;
          stateChange = true;
        }
    }
  }
}

