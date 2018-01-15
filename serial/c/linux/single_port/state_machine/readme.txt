
                 Read Me


*******************************************************
*  Description 
*******************************************************

Objective: 		Create a program that works in tandem with another
			    device to serially/read transmit simulated sensor
			    data.

Functionality: 	main.c
					
	opens /dev/ttyACM0 at 9600 baud
	
	installs signal handler that allows user to stop program 
	by pressing CTRL+C generating SIGINT

	State Sequence:

	Starts in WAIT_FOR_CONNECTION state. Stays in that
	state until the hello message is received.

	Transitions to SEND_READY_SIGNAL state and transmits
	ready message.

	Transitions to READ_ACK state and waits to receive
	ack message.

	Transitions to READ_SENSOR state and stays in that
	state until CTRL + C is pressed.

	extensive debugging trace information is written to stderr 
	and .log file.

	This allows user to verify sequence of events is as expected, 
	as well as observe that messages are handled correctly when
	not all bytes are received at once.

	Whan a complete sensor data message has been received, 
	the sensor data is extracted and displayed. The expected
	data is a count that starts at 0 and increments by 1 until
	the count rolls over after 2^16 - 1.

					
	When SIGINT (CTRL + C) is received, the program terminates
	after closing connections and writing some summary
	statistics.

					

	Currently, the following defaults are used.
	 	Serial:
	   			Default path: /dev/ttyACM0
	   			Default baud rate: 9600


*******************************************************
*  Source files
*******************************************************


Name:	main.c   

Dependencies: serial.c, serial.h provide serial communication functions
			  communication_state.c, communication_state.h
			  debuglog.h


Name: 	serial.c, serial.h
			implements functions needed for serial communication

Name:	communication_state.c, communication_state.h
			implements functions needed to comnunication states
			for sending, receiving messages



*******************************************************
*  Circumstances of programs
*******************************************************
Date: 1/15/2018

	The program compiles and runs successfully. 
   
	The program was developed and tested on Ubuntu 16.04,
	using gcc version 5.4.0

   	This program was developed to test receiving serial port data from 
	an Arduino Uno R3 and a C program running in a Linux OS.




Program pair

Arduino sensor_state_machine.ino and c/linux/single_port/state_machine/main.c

These two programs are designed to be run together.

   a) Arduino program serially transmits five bytes of binary data per 
      message

   			 Baud rate = 9600
             
             Transmits:
             	start marker
             	sensor id
             	count value as two bytes (msb transmitted first)
             	end marker
            

   b) Linux state_machine/main.c

   			 

Note: This example suffices to establish that the state machine for decoding bytes received
is working.



Next steps:

1) Develop another set of Linux programs that use a state machine to control communication
   with more than one Arduino (multiple serial port connections);

2) Please maintain this program as is, and create a new program for other applications.





*******************************************************
*  How to build and run the program
*******************************************************

Instructions: 	The Makefile in this directory creates an executable named erun

1. Folder receive_bytes contains the following files:

	main.c
	serial.c
	serial.h
	communication_state.c
	communication_state.h
	Makefile
	readme.txt

Follow steps 2 - 5 to build the shared object library files, install the library, configure
the cache, and include it in the searchable library path.

2. Build the executable

    Compile the program and build the object .o and executable files:
    % make

    The debuglog library must be installed or the Makefile will generate link errors
    and will not be able to find debuglog.h.  See learn/logging/c/debuglog/readme.txt
    for instructions on building this library.


3. Run the program
   % ./erun


To clean the files: make clean

