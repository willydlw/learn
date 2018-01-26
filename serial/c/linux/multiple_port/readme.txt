
                 Read Me


*******************************************************
*  Description 
*******************************************************

Objective: 		Create a program that reads serial data. 

Functionality: 	main.c program 
					opens /dev/ttyACM0 at 9600 baud
					loops 100 times
						calls serial_read, 5 bytes at a time
						counts number of times < 5 bytes read
					write results to file

					loops 100 times
						calls serial_read_until endmarker
						counts number of times read fails
						counts number of times read times out

					write results to file 

					program terminates


	   			Serial:
	   				Default path: /dev/ttyACM0
	   				Default baud rate: 9600


*******************************************************
*  Source files
*******************************************************


Name:	main.c   

Dependencies: serial.c, serial.h provide serial communication functions


Name: 	serial.c  
			implements functions defined in serial.h



*******************************************************
*  Circumstances of programs
*******************************************************
Date: 12/24/2017

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






*******************************************************
*  ToDo
*******************************************************

1. Add functionality to read debug log levels from a
   file (pass file name to main) or pass the log levels
   as agruments to main. Right now the levels are 
   hard coded in main.

2. Handle failed serial connections - currently the function
   handle_failed_serial_connections is called when the connection
   to an active devices fails. An error message is logged and
   the device is set to inactive. There is no recovery 
   alternative written into the code.

   One reason for the failure may be that the device is listed
   as active in the input file, but it is not actually active.

   Another reason for the failure may be that the serial device
   path is incorrect. We can try to recover from that error
   by reading the active devices from /dev/tty* and try 
   connecting to one of those. Later, during the sensor 
   registration state, if there is a sensor id mismatch,
   we will have connected to the incorrect device.

   Hoping that recovery code will not be needed as the 
   Linux udev rules will provide a symbolic link to the
   device allowing us to assign the desired device path
   name to the same device every time. Then the device path
   name in the input file should correspond to the correct
   device and result in no failure to open due to the
   wrong name.

   Other reasons for failure are likely hardware related,
   with no opportunity for software recovery.
