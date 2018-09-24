
                 Read Me


*******************************************************
*  Description 
*******************************************************

Objective: 		Create Unix Domain Socket server, client examples 

Functionality: 	server.c program 
					uses a default path: /tmp/srv_socket
					
					creates server socket endpoint for local communication
						file: /tmp/srv_socket
						AF_UNIX specifies local communication
						SOCK_STREAM provides sequenced, reliable, two-way, connection-based
						socket_fd is the file descriptor

					calls unlink function to delete the name /tmp/srv_socket from the file
						system before trying to bind to that file. bind will fail if the file
						already exists.
						
					binds to the file /tmp/srv_socket
					listens for a client to connect

					while(1)
						accept client connection
						read bytes from the client until end of file signaled
						writes bytes read to standard error stream
						upon EOF, closes connection to client, but loop keeps running

					use ctrl+C to terminate the program

	   			
Functionality: 	client.c program 
					uses a default path: /tmp/srv_socket
					
					creates client socket endpoint for local communication
						file: /tmp/srv_socket
						AF_UNIX specifies local communication
						SOCK_STREAM provides sequenced, reliable, two-way, connection-based
						socket_fd is the file descriptor

						
					connects to server socket

					while( bytes read from standard input > 0)
						write bytes read to socket file descriptor (the server)
						terminate upon write error

					use ctrl+C to terminate the program

*******************************************************
*  Source files
*******************************************************


Name:	server.c   

Dependencies: file srv_socket must exist or unlink error will terminate the program

To create the file: touch /tmp/srv_socket

To see if the file exists: ls -al /tmp/srv_socket


Name: 	client.c  
			



*******************************************************
*  Circumstances of programs
*******************************************************
Date: 9/24/2018

	The programs compile and run successfully. 
   
	The programs were developed and tested on Ubuntu 18.04,
	using gcc version 7.3.0

   	These programs were developed to learn the most simple forms of UNIX
   	domain socket server and client programs.





*******************************************************
*  How to build and run the program
*******************************************************

Instructions: 	The Makefile in this directory creates two executables: runServer, runClient

1. Folder receive_bytes contains the following files:

	server.c
	client.c
	Makefile
	readme.txt


Follow steps 2 - 5 to build the shared object library files, install the library, configure
the cache, and include it in the searchable library path.

2. Build the executables

    Compile the program and build the object .o and executable files:
    % make


3. Run the server program first
   % ./runServer

   If you get an unlink error, make sure the file /tmp/srv_socket exists.


4. Run the client program

	% ./runClient

	type input and press the enter key. The server program will show this as output.

5. ctrl + c to stop both programs



   
To clean the files: make clean

