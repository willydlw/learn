

#============================
def sendToArduino(sendStr):
	ser.write(sendStr)

#============================
def recvFromArduino():
	global startMarker, endMarker

	ck = ""
	x = "z"  # set to value that is not start or end marker
	byteCount = -1

	# wait for start character
	while ord(x) != startMarker:
		x = ser.read()

	# save data until end marker is found
	while ord(x) != endMarker:
		if ord(x) != startMarker:
			ck = ck + x
			byteCount +=1
		x = ser.read()

	return(ck)



#============================
def waitForArduinoReady():

	# wait until the Arduino sends ready message
	# allows time for Arduino to reset when serial connection
	# is made. Ensures bytes left Arduino's serial transmit buffer
	# are cleared out

	global startMarker, endMarker

	msg = ""
	while msg.find("Arduino ready") == -1:
		# inWaiting returns number of bytes in the serial buffer
		while ser.inWaiting() == 0:
			pass
		msg = recvFromArduino()

		print msg
		print 

#======================================


#======================================

def runTest(fo, numLoops):

  n = 0
  while n < numLoops:

  	while ser.inWaiting() == 0:
  		pass
	dataRecvd = recvFromArduino()
  	print "Reply Received  " + dataRecvd
	n += 1

	print "==========="

	fo.write(str(dataRecvd) + "\n")

    



#==============================
#  DEMO
#==============================

import serial
import time 

print 
print 

# open port at 9600, 8, N, 1, no timeout
ser = serial.Serial('/dev/ttyACM0', 57600) # open serial port

print(ser.name)


# open file for writing
fileObject = open("data.csv", "w")

if not(fileObject.closed):
	print "file " + fileObject.name + ", open"



startMarker = 60        # <
endMarker = 62          # >

waitForArduinoReady()
ser.write("<ack>") 
msg = recvFromArduino();
print("numDataSets message: " + msg)
ser.write("<ack>")



runTest(fileObject, int(msg))


# write data in python 2.7
# ser.write('5')
# in python 3.3, strings are unicode by default
# must be converted to bytes before sending to Arduino
# ser.write(b'5')  required for python 3, optional for 2.7


ser.close()
fileObject.close()