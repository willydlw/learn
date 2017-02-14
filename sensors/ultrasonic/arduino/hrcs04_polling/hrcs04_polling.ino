/** Purpose: obtain distance measurements from 
 *  HRC-S04 ultrasonic sensor.
 *  
 *  Uses polling method for timing.
 *
 *  Author: willydlw
 *  Date: 2/14/17
 *
 *  Testing environment
 *    Arduino 1.8.1, Windows 8.1,  Mega 2560
 *    Arduino 1.8.1, Ubuntu 16.04, Mega 2560
 *    
 */

// Global variable declarations
const int triggerPin = 9;
const int echoPin = 8;
unsigned long responseTime;      // units, microseconds
unsigned int distanceCm;         // units, centimeters
 
void setup() {
  pinMode(triggerPin, OUTPUT);
  Serial.begin(9600);
  delay(2000);
}


void loop() {

  get_ultrasonic_measurement();
  
  // Calculate distance to object
  distanceCm = (unsigned int)responseTime / 58UL;

  // Write distance to serial port
  Serial.print("cm: ");
  Serial.println(distanceCm);

  // wait 1 second before repeating the loop
  delay(1000);

}

void get_ultrasonic_measurement(void){
  
  // 1. Set trigger line low for a brief interval, 2 microseconds
  digitalWrite(triggerPin, LOW);
  delayMicroseconds(2);
  // 2. Change the trigger line from low to high
  digitalWrite(triggerPin, HIGH);
  // 3. Hold the trigger line high for at least 10 microseconds
  delayMicroseconds(10);
  // 4. Pull trigger line low
  digitalWrite(triggerPin, LOW);
 
  // 5. while echo pin is low do nothing
  while(echoPin == LOW) ;
  
  // 6. Start timer
  unsigned long startTime = micros();

  // 7. Continue timing until echo line falls from high to low
  //    while echo pin is high do nothing
  while( echoPin == HIGH) ;

  // 8 Stop timer
  unsigned long stopTime = micros();

  // 9. Measure elapsed time
  responseTime = stopTime - startTime;
}
