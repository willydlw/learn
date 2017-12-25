
                 Read Me


*******************************************************
*  Description 
*******************************************************

Objective: 		Create a program that utilizes a signal IO interrupt to
            	indicate when serial data has been received. 

Functionality: 	main.c provides C language functions for signal IO interrupt 
				handler, specifically for a Linux OS.

				signalIO interrupt signals arrival of serial data

	   			Serial:
	   				Default path: /dev/ttyACM0
	   				Default baud rate: 9600

	   			 	May override default values with command line arguments

	   			 	usage: ./eserial -device /dev/ttyS0 -baud 9600


	   			 Program runs in a loop until it reads 25 bytes, then displays
	   			 the bytes received, and terminates.

*******************************************************
*  Source files
*******************************************************


Name:	main.c   

		Implements signal handler 
		Sets up serial communication
		Runs in a loop until 25 bytes (MAX_DATA_VALUES) have been read.
		Writes data to stderr before terminating.


Dependencies: command.c, command.h provide command line parsing functions

		libcserial.so provides C language functions for serial communication



*******************************************************
*  Circumstances of programs
*******************************************************
Date: 12/21/2017

	The program compiles and runs successfully. 
   
	The program was developed and tested on Ubuntu 16.04,
	using gcc version 5.4.0

   	This program was developed to test receiving serial port data from 
	an Arduino Uno R3 and a c or cpp program running in a Linux OS.




Program pair

Arduino send_byte.ino and Linux signal_interrupt/test/receive_bytes/main.c

These two programs are designed to be run together.

   a) Arduino program serially transmits one byte of binary data
             byte is a count ranging from 0 to 255
             Baud rate - 9600

   b) Linux receive_bytes/main.c

   			 

Note: There are no states in the Arduino program. Once it starts running, it keeps
transmitting the byte count, whether or not the program receiving the data is running.

You will notice that as you run the C program, the byte count received will not always
start at zero. Sometimes the first and second byte will differ greatly. I suspect that
when the C program terminates, there may be a value sitting in the UART buffer waiting to
be read, and that this value is the first read the next time the program runs.

This example suffices to establish that the signal io interrupt handler is working,
and that serial bytes are successfully read.



Next steps:

1) Develop another set of Arduino and Linux programs that use a state machine to control 
   Arduino serial transmission.

2) Please maintain this program as is, and create a new program for specific read applications.

Reference for asynchronous input, serial programming:

http://tldp.org/HOWTO/Serial-Programming-HOWTO/x115.html



*******************************************************
*  How to build and run the program
*******************************************************

Instructions: 	The Makefile in this directory creates an executable named erun

1. Folder receive_bytes contains the following files:

	main.c
	Makefile
	readme.txt

Follow steps 2 - 5 to build the shared object library files, install the library, configure
the cache, and include it in the searchable library path.

2. Build the executable

    Note: The makefile will copy the header file cserial.h to the directory 
    /usr/local/include/cserial/.

    Create the directory cserial inside /usr/local/include before running make 
    the first time. If this directory does not exist, the files will not be copied.

    Change to the directory that contains the file by:
    % cd [directory_name] 

    Compile the program and build the so files:
    % make

    The libcserial.so must be built before running make. Otherwise, there will be
    linking errors. The files needed to build this library are in the serialib directory.


3. Run the program
   % ./erun



   
To clean the files: make clean

