/** Description: 
 *     Serially transmit a byte count that ranges from 0 to 255
 *     Writes binary data, not ASCII characters
*
*   Software: Arduino 1.8.1
*   Date; 12/20/17
*/

unsigned int count = 0;

const byte startMarker = '<';
const byte sensorId = 'Z';
const byte endMarker = '>';

// index 0 start marker
// index 1,2 device id
// index 3, 4 reserved for 2 byte count
// index 5  end marker
byte sendCountBuffer[5];  

void setup(){
  
  // initialize serial and wait for port to open
  Serial.begin(9600);


  sendCountBuffer[0] = startMarker;
  sendCountBuffer[1] = sensorId;

  sendCountBuffer[4] = endMarker;
}

void loop(){
  sendCountBuffer[2] = (byte)(count >> 8);  // msb
  sendCountBuffer[3] = (byte)count;         // lsb
  
  Serial.write(sendCountBuffer, 5);
  
  count++;
  delay(100);  
        
}

