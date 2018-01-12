
                 Read Me


*******************************************************
*  Description 
*******************************************************

Objective: 	Create programs to test the debuglog library




*******************************************************
*  Source files
*******************************************************


Name: console_test.c
      
      Tests all console logging levels with color 
      display turned on.

      Console output designed to easily confirm each
      level produces the correct messages.

      Tests the following functions:
      log_init, log_trace, log_debug, log_info, log_warn,
      log_error, log_fatal

Name:	file_log_test.c
   
      Tests all file logging levels with color
      display turned off. There is no functionality
      to write colored text to a file.

      The current debuglog library implementation uses 
      a default file name and location to write files.

      As the test program runs, there is console output
      to let the user know the current program execution
      state. 

      Six different files are created, one for each 
      logging level that produces output. 

      Tests the following functions:
        log_init, log_trace, log_debug, log_info, log_warn,
        log_error, log_fatal, close_log_file



Name: parse_config_test.c

      Uses the file configTest.txt to set log flags:
      consoleLevel, fileLevel, colorDisplay

      Uses a non-existent file to test consequences
      of specifying a file that cannot be opened and
      use of default settings.

      Tests the following functions:
          log_config, get_console_level,
          get_file_level, close_log_file


*******************************************************
*  Circumstances of programs
*******************************************************
Date: 1/9/2017

   The programs compile and link to the debuglog 
   library successfully. 
   
   All of the above test program results indicate the
   debuglog functions are working as expected. 
   
   The programs are developed and tested on Ubuntu 16.04,
   using gcc version 5.4.0
   


*******************************************************
*  How to build and run the program
*******************************************************

Instructions: 	
  The Makefile in this directory creates two executable
  files, one for each test.

1. Folder debuglog contains the following files:

	console_test.c
	file_log_test.c
  parse_config_test.c
	Makefile
	readme.txt
	

Follow steps 2 - 5 to build the shared object library files, install the library, configure
the cache, and include it in the searchable library path.

2. Build the executables

    Note: The debuglog library must be installed before
    running these executables. Otherwise, there will be
    a link error, saying libdebuglog.so cannot be found.

    Change to the directory that contains the file by:
    % cd [directory_name] 

    Compile the program and build the so files:
    % make


3. Clean
   % make clean



*******************************************************
*  To Do
*******************************************************

1. Ensure there is a unit test written for every function.




