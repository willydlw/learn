
                 Read Me


*******************************************************
*  Description 
*******************************************************

Objective: 		Create a program that reads serial data. 

Functionality: 	main.c program 
					opens /dev/ttyACM0 at 9600 baud
					infinite while loop
						reads serial data
						displays data read

					any error messages are sent to stderr


	   			Serial:
	   				Default path: /dev/ttyACM0
	   				Default baud rate: 9600


*******************************************************
*  Source files
*******************************************************


Name:	main.c   

		Sets up serial communication
		Runs in a loop until 25 bytes (MAX_DATA_VALUES) have been read.
		Writes data to stderr before terminating.


Dependencies: serial.c, serial.h provide serial communication functions


Name: 	serial.c  
			implements functions defined in serial.h



*******************************************************
*  Circumstances of programs
*******************************************************
Date: 12/2142017

	The program compiles and runs successfully. 
   
	The program was developed and tested on Ubuntu 16.04,
	using gcc version 5.4.0

   	This program was developed to test receiving serial port data from 
	an Arduino Uno R3 and a c or cpp program running in a Linux OS.




Program pair

Arduino send_bytes.ino and c/linux/simple_serial/main.c

These two programs are designed to be run together.

   a) Arduino program serially transmits five bytes of binary data
   			 Baud rate = 9600
             
             Transmits:
             	start marker
             	sensor id
             	count value as two bytes (msb transmitted first)
             	end marker
            

   b) Linux simple_serial/main.c

   			 

Note: There are no states in the Arduino program. Once it starts running, it keeps
transmitting the byte count sequence, whether or not the program receiving the data 
is running.

You will notice that as you run the C program, the byte count received will not always
start at zero. 

This example suffices to establish that the state machine for decoding bytes received
is working.



Next steps:

1) Develop another set of Arduino and Linux programs that use a state machine to control 
   Arduino serial transmission.  Example: Arduino and Linux program establish communication
   with an acknowledgment. Arduino then starts transmitting after acknowledge sequence.

   Linux program is written to interact with one serial device. When the ack sequence is 
   successfully tested, the next step will be creating a Linux program that handles 
   multiple sensors.

2) Please maintain this program as is, and create a new program for specific read applications.





*******************************************************
*  How to build and run the program
*******************************************************

Instructions: 	The Makefile in this directory creates an executable named erun

1. Folder receive_bytes contains the following files:

	main.c
	serial.c
	serial.h
	Makefile
	readme.txt

Follow steps 2 - 5 to build the shared object library files, install the library, configure
the cache, and include it in the searchable library path.

2. Build the executable

    Compile the program and build the object .o and executable files:
    % make


3. Run the program
   % ./erun



   
To clean the files: make clean

