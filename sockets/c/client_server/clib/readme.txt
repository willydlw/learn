
                 Read Me


*******************************************************
*  Description 
*******************************************************

Objective: 	Create a shared object library named libclientserver.so 

Functionality: 	libclientserver.so provides client server sockets functions 
		
		Functions are used to bind a server, connect a client socket
		send and receive data.
		


*******************************************************
*  Source files
*******************************************************


Name:	clientserver.h
	Contains the client server function prototypes.

Name:	clientserver.c
   
	Implements the client server functions.



*******************************************************
*  Circumstances of programs
*******************************************************
Date: 4/3/2015

   The programs compile and build the library successfully. 
   
   The programs are developed and tested on Ubuntu 14.04,
	64 bit, using gcc version 4.8.2
   


*******************************************************
*  How to build and run the program
*******************************************************

Instructions: 	The Makefile in this directory creates a shared object library 
		named libclientserver.so

1. Folder client_server/clib contains the following files:

	clientserver.c
	clientserver.h
	Makefile
	readme.txt

Follow steps 2 - 5 to build the shared object library files, install the library, configure
the cache, and include it in the searchable library path.

2. Build the library

    Note: The makefile will copy the header file clientserver.h to the directory 
	 /usr/local/include/clientserver/

    Create the directory client_server inside /usr/local/include before running make 
    the first time. If this directory does not exist, the files will not be copied.

    Change to the directory that contains the file by:
    % cd [directory_name] 

    Compile the program and build the so files:
    % make


3. Install the library:
   % sudo make install

4. Configure
   % sudo ldconfig -n -v /usr/local/lib

   ldconfig creates the necessary links and cache to the most recent shared libraries found 
   in the directories specified on the command line, in the file /etc/ld.so.conf, and in 
   the trusted directories (/lib and /usr/lib).

5. Set the LD_LIBRARY_PATH
   % export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH

   


To uninstall the libary: make uninstall

To clean the files: make clean





