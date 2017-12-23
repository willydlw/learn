/** Description: 
 *     Serially transmit a byte count that ranges from 0 to 255
 *     Writes binary data, not ASCII characters
*
*   Software: Arduino 1.8.1
*   Date; 12/20/17
*/

unsigned int count = 0;

const byte startMarker = '<';
const byte sensorId[2] = {'L','F'};
const byte endMarker = '>';

// index 0 start marker
// index 1,2 device id
// index 3, 4 reserved for 2 byte count
// index 5  end marker
byte sendCountBuffer[6];  

void setup(){
  
  // initialize serial and wait for port to open
  Serial.begin(9600);


  sendCountBuffer[0] = startMarker;
  sendCountBuffer[1] = sensorId[0];
  sendCountBuffer[2] = sensorId[1];
  sendCountBuffer[5] = endMarker;
}

void loop(){
  sendCountBuffer[3] = (byte)(count >> 8);  // msb
  sendCountBuffer[4] = (byte)count;         // lsb
  
  Serial.write(sendCountBuffer, 6);
  
  count++;
  delay(500);  
        
}

