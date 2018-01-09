/**  Purpose: test file logging functions 

	 Loops through each logging level
	 to ensure messages are written as expected.

	 Console log level is LOG_OFF. 

	 File output is written to debuglog default 
	 location and default file name.

	 This test will create 6 files. No file should
	 be created for the LOG_OFF file level.

	 Tests the following functions:
        log_init, log_trace, log_debug, log_info, log_warn,
        log_error, log_fatal, close_log_file

*/

#include <debuglog.h>
#include <string.h>				// strcmp
#include <unistd.h>				// sleep

static const char* levelNames[] = {
	"TRACE", "DEBUG", "INFO",  "WARN", "ERROR", "FATAL", "OFF"
};



void test_levels(int testNumber, int numMessages)
{
	fprintf(stderr, "test %d: level is %s\n", testNumber, levelNames[testNumber]);

	if( strcmp(levelNames[testNumber], "OFF") != 0){
		fprintf(stderr, " %d log level messages should be written to the file\n", numMessages);
	}
	else{
		fprintf(stderr, "no file will be created for the LOG_OFF level,"
						" 0 messages to be written\n");
	}

	log_trace("test trace, log level: %s, testNumber: %d, numMessages: %d", 
					levelNames[testNumber], testNumber, numMessages);

	log_debug("test debug, log level: %s, testNumber: %d, numMessages: %d", 
					levelNames[testNumber], testNumber, numMessages);

	log_info("test  info, log level: %s, testNumber: %d, numMessages: %d", 
					levelNames[testNumber], testNumber, numMessages);

	log_warn("test  warn, log level: %s, testNumber: %d, numMessages: %d", 
					levelNames[testNumber], testNumber, numMessages);

	log_error("test error, log level: %s, testNumber: %d, numMessages: %d", 
					levelNames[testNumber], testNumber, numMessages);

	log_fatal("test fatal, log level: %s, testNumber: %d, numMessages: %d", 
					levelNames[testNumber], testNumber, numMessages);

	fprintf(stderr, "\n\n");
}



int main(void)
{
	fprintf(stderr, "\n\n=====  Testing Console Log Level, Display Color On  =====\n\n");
	LogLevel level;
	for(level = LOG_TRACE; level <= LOG_OFF; ++level){

		// reset the file logging level each time through the loop
		// color display is off, 0, because color is not used for file logging
		log_init(LOG_OFF, level, 0);

		test_levels(level, (int)LOG_OFF-(int)level);
		/* pause program so that there will be a separate file written for
		   each loop iteration. The default file name has a time stamp.
		   If the program does not pause at least 1 second, then the 
		   file name generated will be the same and the old one will be
		   over written.
		   
		   also close the log file, before continuing through the loop
		 */

		close_log_file();

		sleep(2);
	}
	
	
	return 0;
}