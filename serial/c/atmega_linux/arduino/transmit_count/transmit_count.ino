/** Writes a count to the serial port */

unsigned char serialDataCount;

void setup() {
  
  Serial.begin(9600);
  serialDataCount = 0;
}

void loop() {
  
  /** write binary data
  *
  * do not use the print function as it sends ASCII */
  //Serial.write(serialDataCount & 0xFF);
  //Serial.write(serialDataCount >> 8);
  Serial.write(serialDataCount);

  // when this is an unsigned char, it will run from 0 to 255
  // and then restart at 0 again
  serialDataCount++;

  // delay controls how often signal is transmitted
  delay(1000);

}
