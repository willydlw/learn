/** Purpose: test serial transmission of two numbers.
 *  
 *  Simulates transmitting servo angle and proximity sensor distance data.
 *  
 *  Goal is to build a program that controls a servo motor
 *  rotating an ultrasonic proximity sensor. 
 *  
 *  Author: Diane Williams
 *  Date: 3/1/17
 *  
 *  Test enviroment:
 *    Software: Arduino version 1.8.1
 *    Hardware: Arduino Uno
 *    Operating System: Ubuntu 16.04
 */


// Global Constant Declarations

// Servo motor angular range
const int MINIMUM_ANGLE_DEGREES = 0;
const int MAXIMUM_ANGLE_DEGREES = 180;

int deltaThetaDegrees = 5;

// 
int servoAngleDegrees;        // units, degrees

float distanceCm = 10;        // units, cm

void setup() {
  
  Serial.begin(9600);       // enable Serial hardware
  // intialize servo motor position
  servoAngleDegrees = 0;
  delay(2000);
}

void transmitDistanceData(void){
  // Encoding scheme: angle,distance
  Serial.print(servoAngleDegrees);
  Serial.print(",");
  Serial.println(distanceCm);
  
}

// simulate servo rotation
void rotateServo(){
  // Change rotation direction at limits
  if(servoAngleDegrees >= MAXIMUM_ANGLE_DEGREES ||servoAngleDegrees <= MINIMUM_ANGLE_DEGREES)
  {
    deltaThetaDegrees = -deltaThetaDegrees;
  }

  servoAngleDegrees += deltaThetaDegrees;
  
}

void getDistanceMeasurement(){
  // simulates a distance measurement
  distanceCm = distanceCm + 1;  
}

void loop() {
  
  getDistanceMeasurement();
  transmitDistanceData();
  rotateServo();
  delay(500);

}
