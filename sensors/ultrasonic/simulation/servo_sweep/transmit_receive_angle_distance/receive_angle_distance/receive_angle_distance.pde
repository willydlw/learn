import processing.serial.*; 
 
Serial myPort;    // The serial port
PFont myFont;     // The display font
String inString;  // Input string from serial port
int lf = 10;      // ASCII linefeed 

float angle = 0;
float distanceCm = 0;
 
void setup() { 
  size(400,200); 
  // You'll need to make this font with the Create Font Tool 
  myFont = loadFont("AbyssinicaSIL-18.vlw"); 
  textFont(myFont, 18); 
  // List all the available serial ports: 
  printArray(Serial.list()); 
  // I know that the first port in the serial list on my mac 
  // is always my  Keyspan adaptor, so I open Serial.list()[0]. 
  // Open whatever port is the one you're using. 
  myPort = new Serial(this, Serial.list()[0], 9600); 
  myPort.bufferUntil(lf); 
} 
 
void draw() { 
  background(0); 
  text("angle: " + angle, 10,50); 
  text("distance [cm]: " + distanceCm, 10, 80);
} 
 
void serialEvent(Serial p) { 
  inString = myPort.readString();   
    if (inString != null) {
      String[] q = splitTokens(inString, ", ");
      angle = float(q[0]);
      distanceCm = float(trim(q[1]));
      println(inString);
    }
} 