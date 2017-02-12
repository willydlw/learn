
                 Read Me


*******************************************************
*  Description 
*******************************************************

Objective: 	Create a shared object library named libcserial.so 

Functionality: 	libcserial.so provides C language functions for serial communication
		

*******************************************************
*  Source files
*******************************************************


Name:	cserial.h
	Contains the serial function declarations.

Name:	cserial.c
   
	Implements the serial functions.


*******************************************************
*  Circumstances of programs
*******************************************************
Date: 2/11/2017
   The programs compile and build the library successfully. The programs are not
   completely developed for all needed serial read/write functionality. The programs are also
   not fully tested, nor has a set of test programs yet been created. 
   
   The programs are developed and tested on Ubuntu 16.04,
   using gcc version 5.4.0
   


*******************************************************
*  How to build and run the program
*******************************************************

Instructions: 	The Makefile in this directory creates a shared object library 
		named libcserial.so

1. Folder serial contains the following files:

	cserial.c
	cserial.h
	Makefile
	readme.txt

Follow steps 2 - 5 to build the shared object library files, install the library, configure
the cache, and include it in the searchable library path.

2. Build the library

    Note: The makefile will copy the header file cserial.h to the directory 
    /usr/local/include/cserial/.

    Create the directory cserial inside /usr/local/include before running make 
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





