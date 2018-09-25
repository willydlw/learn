
                 Read Me


*******************************************************
*  Description 
*******************************************************

Program creates a sockets server for interprocess communication.

Objective: Learn how to open a socket and listen for client

Behavior: Creates socket
          Listens for client connection
          Accepts client connection
          Prints message received from client
          Sends message to client
          Program terminates

*******************************************************
*  Source files
*******************************************************

Name:	server.c


*******************************************************
*  Circumstances of programs
*******************************************************
Date: 3/16/2015
   The program compiles and runs successfully.  
   
   The programs are developed and tested on Ubuntu 14.04,
   using gcc version 4.8
   


*******************************************************
*  How to build and run the program
*******************************************************

1. Folder server contains the following files:

	server.c
	readme.txt
	


Follow step 2 to build the program from the command line or step 3
to build the program with the Makefile.

2.  Command Line Build

    Change to the directory that contains the file by:
    % cd [directory_name] 

    Compile the program and build the executable file:
    % gcc -Wall -g server.c -o eserver 

    

3. Run the executable program:
   % ./eserver 8888


	The client and server may be on the same host machine or on different
   host machines.

	Start the server program first. Pass a port number as a command line argument.

   Note: you may use any port number between 2000 and 65535.

	If this port is already in use on that machine, the server will tell you this 
   and exit. If this happens, just choose another port and try again. If the port 
   is available, the server will block until it receives a connection from the 
   client. Don't be alarmed if the server doesn't do anything.

	It's not supposed to do anything until a connection is made.




