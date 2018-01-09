/**  Purpose: test logging functions 
		
		using a valid configuration file
		using a non-existent configuration file 

	 	File output is written to debuglog default 
	 	location and default file name.

	 	Tests the following functions:
      		log_config, get_console_level,
      		get_file_level, close_log_file

*/

#include <debuglog.h>
#include <string.h>				// strcmp
#include <unistd.h>				// sleep

static const char* levelNames[] = {
	"TRACE", "DEBUG", "INFO",  "WARN", "ERROR", "FATAL", "OFF"
};



void test_levels(int testNumber, LogLevel consoleLevel, LogLevel fileLevel)
{

	fprintf(stderr, "\ntest %d: console level is %s, file level is %s\n", 
				testNumber, levelNames[consoleLevel], levelNames[fileLevel]);

	if( strcmp(levelNames[fileLevel], "OFF") != 0){
		fprintf(stderr, " %d log level messages should be written to the file\n", LOG_OFF - fileLevel);
	}
	else{
		fprintf(stderr, " no file will be created for the LOG_OFF level,"
						" 0 messages to be written\n");
	}

	
	fprintf(stderr, " %d log level messages should be written to the console\n", 
					LOG_OFF - consoleLevel);

	fprintf(stderr, " color is turned %s\n\n", color_display_state()? "on":"off");

	log_trace("test trace, testNumber: %d", testNumber);
	log_debug("test debug, testNumber: %d", testNumber);
	log_info("test  info, testNumber: %d", testNumber);
	log_warn("test  warn, testNumber: %d", testNumber);
	log_error("test  error, testNumber: %d", testNumber);
	log_fatal("test  fatal, testNumber: %d", testNumber);

	fprintf(stderr, "\n\n");
}



int main(void)
{
	fprintf(stderr, "\n\n=====  Testing Parse Config  =====\n\n");

	LogLevel consoleLevel;
	LogLevel fileLevel;

	char *filename = "configTest.txt";

	fprintf(stderr, "\n***** Test Number 1, file: %s  *****\n\n", filename);
	log_config(filename);

	// tests get_console_level and get_file_level_functions
	consoleLevel = get_console_level();
	fileLevel = get_file_level();

	test_levels(1, consoleLevel, fileLevel);

	close_log_file();
	sleep(1);


	fprintf(stderr, "\n*****  Test with non-existent configuration file  *****\n\n");
	log_config("nonexistent.txt");
	consoleLevel = get_console_level();
	fileLevel = get_file_level();
	test_levels(2, consoleLevel, fileLevel);

	// call close function. No output file should have been opened.
	// verify this only produces a debugging output message
	// and does not cause a run-time error
	close_log_file();

	
	return 0;
}