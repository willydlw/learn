#include <string.h>

/** ultrasonic sensor **/
const int triggerPin = A5;
const int echoPin = A4;

const int asize = 32;
const int numSonarDataSets = 3;

unsigned long elapsedTime;
unsigned long distance[asize];

/** transmission **/
const char startMarker = '<';
const char endMarker = '>';
const byte inputBufferSize = 32;

byte bytesReceived = 0;
boolean readInProgress = false;
boolean newInputData = false;
char inputDataBuffer[inputBufferSize] = {0};

int sendCount = 0;


void setup() {
  // enable UART at 57600 baud, 8N1
  Serial.begin(57600);

  // set pin mode
  pinMode(echoPin, INPUT);
  pinMode(triggerPin, OUTPUT);
  delay(2000);
  
  Serial.println("<Arduino ready>");
  while( wait_for_ack() == false);
  Serial.print(startMarker);
  Serial.print(numSonarDataSets);
  Serial.print(endMarker);
  while( wait_for_ack() == false);
  delay(1000);

}

void loop() {

  if(sendCount < numSonarDataSets){
    // get distance measurements
    for(int i = 0; i < asize; ++i){
      distance[i] = ultrasonic_distance_pulsein(asize, 58UL);
      delay(50);
    }
  

    // transmit distance measurements
    send_distance_measurements();
    delay(100);
    ++sendCount;
  }

}

void get_data_from_serial(void){
  
  if(Serial.available() > 0){
      char data = Serial.read();

      // end marker will be transmitted while readInProgress is true
      // check for endMarker state before handling normal readInProgress
      if(data == endMarker){
        readInProgress = false;
        newInputData = true;
        inputDataBuffer[bytesReceived] = 0; // null terminate
        //Serial.print("debug, get_data_from_serial, inputDataBuffer: ");
        //Serial.println(inputDataBuffer);
      }
      else if(readInProgress){
        // don't overflow buffer, excess characters are discarded
        if(bytesReceived < inputBufferSize-1){
          inputDataBuffer[bytesReceived] = data;
          ++bytesReceived;
        }
      }

      // possible to have a new start marker during read in progress
      // check for this condition after handling readInProgress, not as
      // and else if condition
      if(data == startMarker){
        bytesReceived = 0;
        readInProgress = true;
      }
    }
  
}

boolean wait_for_ack(void){
  boolean ack = false;
  while(ack == false){
    get_data_from_serial();
    if(newInputData){
      newInputData = false;
      if(strcmp(inputDataBuffer, "ack") == 0)
      {
        return true;
      } 
      else
      {
        return false;
      }
    }
  }
  return ack;
}

unsigned long ultrasonic_distance_pulsein(int numMeasurements, int divisor){

  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  digitalWrite(triggerPin, HIGH);
  delayMicroseconds(10);
  elapsedTime = pulseIn(echoPin, HIGH);
  return (elapsedTime / divisor);
  
}

void send_distance_measurements(void){
  int i;

  // transmit number of measurements
  Serial.print(startMarker);
  Serial.print("Num Measurements,");
  Serial.print(asize);
  Serial.print(",");

  for(i = 0; i < asize; ++i){
    Serial.print(distance[i]);
    Serial.print(",");
  }

  // send the last distance measurement with no following comma
  Serial.print(endMarker);
  Serial.println("");
  Serial.flush();
}

