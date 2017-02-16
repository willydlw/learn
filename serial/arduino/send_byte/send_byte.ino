/** Description: Serially transmits a byte count that ranges from 0 to 255
*
*/

byte count;

void setup(){
  count = 0;
  Serial.begin(9600);         // enable serial communication
  delay(100);
}

void loop(){

  Serial.println(count);
  count++;
  delay(250);  
        
}

