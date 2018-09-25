
                 Read Me


*******************************************************
*  Description 
*******************************************************

Objective: 	Illustrates how to run both a client and server
				program. 

		The test_server and test_client programs both link to 
      the libclientserver shared object library: libclientserver.so 
		
		Refer to the clib/clientserver.h documentation for more 
		information on function usage.


*******************************************************
*  Source files
*******************************************************


Name:	test_server.c
	
	Source code for creating a server socket connection that
   	is listening for client connections.
 
	server program echos back any messages received.

Name:	test_client.c
	
	Source code for creating a client socket connection that
   connects to the server socket.
 
	test_client program prompts user to enter a message and
	sends that message to the server.



*******************************************************
*  Circumstances of programs
*******************************************************
Date: 4/3/2015
   The programs compile and link to the library successfully. 

   The programs are developed and tested on Ubuntu 14.04,
   using gcc version 4.8.2
   


*******************************************************
*  How to build and run the programs
*******************************************************

Instructions: 	The Makefile in this directory creates two 
		executables named eserver and eclient

1. Folder client_server/clib/test contains the following files:

	server_test.c
	client_test.c
	Makefile
	readme.txt

Follow steps 2 - 5 to build the executable files eserver and eclient.
They each link to the shared object library libclientserver.so. 


2. Build the executables

    Change to the directory that contains the file by:
    % cd [directory_name] 

    Compile the program and build the files:
    % make

3. First, execute the eserver program, pass one additional command line argument
   in addition to the name of the executable file. 

   Example: ./eserver 8888

   The second argument is the port number connection.

   If the program will not run, you may need to run this command:
   export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH

	To see the path, type  echo $LD_LIBRARY_PATH

	Second, execute the eclient program, pass two additional command line
	arguments in addition to the name of the executable file.

	Example: ./eclient localhost 8888


4. If you want to see a list of an executable's library dependencies
   
   % ldd eserver
	% ldd eclient
  

5. Delete the executable and any object files.

   % make clean





