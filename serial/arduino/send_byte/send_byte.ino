/** Description: 
 *     Serially transmit a byte count that ranges from 0 to 255
 *     Writes binary data, not ASCII characters
*
*   Software: Arduino 1.8.1
*   Date; 12/20/17
*/

byte count = 0;

void setup(){
  
  // initialize serial and wait for port to open
  Serial.begin(9600); 
}

void loop(){
  // writes binary data to the serial port
  Serial.write(count);
  count++;
  delay(250);  
        
}

