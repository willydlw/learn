/** Displays strings received from serial port
    in the draw window
    */

import processing.serial.*; 
 
Serial myPort;    // The serial port
PFont myFont;     // The display font
String inString;  // Input string from serial port
int lf = 10;      // ASCII linefeed 
 
void setup() { 
  size(400,200); 
  // You'll need to make this font with the Create Font Tool 
  myFont = loadFont("DejaVuSans-18.vlw"); 
  textFont(myFont, 18); 
  // List all the available serial ports: 
  printArray(Serial.list()); 
  
  // [0] opens first serial device in list
  // [1] opens second serial device in list
  myPort = new Serial(this, Serial.list()[0], 9600); 
  myPort.bufferUntil(lf); 
} 
 
void draw() { 
  background(0); 
  text("received: " + inString, 10,50); 
} 
 
void serialEvent(Serial p) { 
  inString = p.readString(); 
} 