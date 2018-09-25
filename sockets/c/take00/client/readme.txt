
                 Read Me


*******************************************************
*  Description 
*******************************************************

Program creates a sockets server for interprocess communication.

Objective: Learn how to open a socket and connect to a server

Behavior:  creates socket
           connects to server
           prompts user to enter message to send to server
           disconnects from server
           program terminates


*******************************************************
*  Source files
*******************************************************

Name:	client.c


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

1. Folder client contains the following files:

	client.c
	readme.txt
	


Follow step 2 to build the program from the command line or step 3
to build the program with the Makefile.

2.  Command Line Build

    Change to the directory that contains the file by:
    % cd [directory_name] 

    Compile the program and build the executable file:
    % gcc -Wall -g client.c -o eclient

    

3. Run the executable program:
   % ./eclient localhost 8888

   Note: you may use any port number between 2000 and 65535.

	To run the client you need to pass in two arguments, the name of the 
   host on which the server is running and the port number on which the 
   server is listening for connections.

   The client will prompt you to enter a message. If everything works correctly, 
   the server will display your message on stdout, send an acknowledgement 
   message to the client and terminate.

   The client will print the acknowledgement message from the server and then terminate.

   You can simulate this on a single machine by running the server in one window 
   and the client in another. In this case, you can use the keyword localhost as 
   the first argument to the client.




