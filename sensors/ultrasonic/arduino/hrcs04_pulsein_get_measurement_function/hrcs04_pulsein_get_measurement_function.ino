/** Purpose: obtain distance measurements from 
 *  HRC-S04 ultrasonic sensor.
 *  
 *  Uses pulseIn function for timing.
 *  
 *  Write a separate function for getting sonar measurement
 *
 *  Author: willydlw
 *
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
  /* 5. Start timer
     6. Continue timing until echo line falls from high to low
     7. Stop timer
     8. Measure elapsed time
   *
   ** Passing a value of HIGH to pulseIn function causes the 
   *  function to wait for the echo pin to go high and measure
   *  the time until the echo pin goes low. Returns the length
   *  of the pulse in microseconds
   *     ______
   *  ___|    |___
   */
  responseTime = pulseIn(echoPin, HIGH);
}
