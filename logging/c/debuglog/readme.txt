
                 Read Me


*******************************************************
*  Description 
*******************************************************

Objective: 	Create a shared object library named libdebuglog.so 

Functionality: libdebuglog.so provides debugging logging functions 

    There are 6 logging levels that will produce messages.
    TRACE, DEBUG, INFO, WARN, ERROR, FATAL

    There is one logging level that will suppress all
    messages: OFF

    Messages may be written to stderr and/or a file.
      a) console log level controls which messages are written
         to stderr.

      b) file log level controls which messages are
         written to the output file
		
    There is a color display on state that will
    write the level label with a unique color. This
    only applies to console stderr output. 

		Refer to the debuglog.h for more information on
		the library functions.


*******************************************************
*  Source files
*******************************************************


Name:	debuglog.h
	Contains the definition for the debug logging functions

Name:	debuglog.c
   
	Implements the logging functions.


*******************************************************
*  Circumstances of programs
*******************************************************
Date: 1/9/2017

   The programs compile and build the library successfully. 
   A set of test programs is located in the test directory. 
   
   The programs are developed and tested on Ubuntu 16.04,
   using gcc version 5.4.0
   


*******************************************************
*  How to build and run the program
*******************************************************

Instructions: 	The Makefile in this directory creates a shared object library 
		named libdebuglog.so

1. Folder debuglog contains the following files:

	debuglog.c
	debuglog.h
	Makefile
	readme.txt

	test/console_test.c
	test/file_log_test.c
	test/config_file_test.c
	test/Makefile
	test/readme.txt
	

Follow steps 2 - 5 to build the shared object library files, install the library, configure
the cache, and include it in the searchable library path.

2. Build the library

    Note: The makefile will copy the header file debuglog.h to 
    the directory /usr/local/include/debuglog/.

    Create the directory cdebuglog inside /usr/local/include before running make 
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

5. Set the LD_LIBRARY_PATH (may only need to do this the first time library is built)
   % export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH

6. Restart bash 
   % source ~/.bashrc

  

To uninstall the libary: make uninstall

To clean the files: make clean


*******************************************************
*  TODO
*******************************************************

1. Implement functionality to allow the user to specify
   the output file name and location.

2. Parsing the configuration file is fairly simple.
   Might want to feature that allows the configuration
   file to be XML or JSON.

   Allow user to have comments in configuration file.


3. Add comments for each function definition.


