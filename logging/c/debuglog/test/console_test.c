/**  Purpose: test console logging functions 

	 Loops through each logging level
	 to ensure messages are displayed as expected.

	 File log level is LOG_OFF. Generates no file output.

	 Color display is turned on.

	 Tests the following functions:
      log_init, log_trace, log_debug, log_info, log_warn,
      log_error, log_fatal


*/

#include <debuglog.h>

static const char* levelNames[] = {
	"TRACE", "DEBUG", "INFO",  "WARN", "ERROR", "FATAL", "OFF"
};



void test_levels(int testNumber, int numMessages)
{
	fprintf(stderr, "test %d: level is %s\n", testNumber, levelNames[testNumber]);
	fprintf(stderr, "         %d log level messages should be displayed below\n", numMessages);

	log_trace("test trace %d, float %f, char %c, string %s", 1, 1.23, 'A', "Wookie");
	log_debug("test debug %d, float %f, char %c, string %s", 2, 4.5, 'B', "Jedi");
	log_info("test info with no extra arguments");
	log_warn("test warn %d", 22);
	log_error("test error %d", 33);
	log_fatal("test fatal %f", 99.876);

	fprintf(stderr, "\n\n");
}



int main(void)
{
	fprintf(stderr, "\n\n=====  Testing Console Log Level, Display Color On  =====\n\n");
	LogLevel level;

	for(level = LOG_TRACE; level <= LOG_OFF; ++level){
		// 1 turns on color display, 0 turns off color display
		log_init(level, LOG_OFF, 1);
		test_levels(level, (int)LOG_OFF-(int)level);
	}
	
	
	return 0;
}