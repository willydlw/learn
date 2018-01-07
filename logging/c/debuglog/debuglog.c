#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

#include "debuglog.h"

#define MAX_CHARACTERS_PER_WORD 32


/* ==========     Static Global Variables     ========= */

static LogFlags logFlags;


static const  char* log_level_names[] = {	
	"TRACE", "DEBUG", "INFO",  "WARN", "ERROR", "FATAL", "OFF"
};

/* colors reference: http://jafrog.com/2013/11/23/colors-in-terminal.html 
	*
	*	ANSI escape codes:    
	*		ASCII Hex \0x1B		ESC
	*		ASCII Oct \033		ESC
	*		Shell     \e
	*
	*   control sequence introducer ASCII ESC character code and a [
	*	\0x1B[
	*
	*   followed by a list of instructions, separated by ;
	*   [<PREFIX>];[<COLOR>];[<TEXT DECORATION>]
	*
	*   Color codes
	*		Basic 8 colors 					30 .. 37
	*		Basic "high contrast" colors 	90 .. 97
	*		xterm-256 colors 				0 .. 255
	*
	*	If color code is prefixed by 38;5 it is interpreted as one of 256 colors
	*	\e[38;5;91m will color following text purple while \e[91m will indicate bright red
	*
	*	Text decoration
	*		Bold							1
	*		Underscore						4
	*		Background						3
	*
	*	m indicates the end of the control sequence so terminal will know not to interpret 
	*     text after m as a color code
	*
	*   \x1b[0m means reset all attributes
	*

	#define COLOR_RESET		"\0x1b[0m"
	#define COLOR_GRAY		"\0x1b[30m"
	#define COLOR_RED		"\0x1b[31m"
	#define COLOR_GREEN		"\0x1b[32m"
	#define COLOR_YELLOW	"\0x1b[33m"
	#define COLOR_BLUE		"\0x1b[34m"
	#define COLOR_MAGENTA	"\0x1b[35m"
	#define COLOR_CYAN		"\0x1b[36m"
	#define COLOR_WHITE		"\0x1b[37m"
	*/
static const char* log_level_colors[] ={
				"\0x1b[30m", "\0x1b[32m", "\0x1b[37m", "\0x1b[33m", "\0x1b[31m", "\0x1b[34m"
};


void logit(int level, const char *fmt, ...)
{
	time_t currentTime;
	struct tm  *ts;
	

	if(level < logFlags.consoleLevel && level < logFlags.fileLevel){
		return;
	}


	// get current time
	currentTime = time(NULL);
	ts = localtime(&currentTime);

	
	// console logging
	if(level < logFlags.consoleLevel){
		va_list args;
		
		char timeBuffer[16];					// stores formatted time

		// format time into hours:min:sec
		timeBuffer[strftime(timeBuffer, sizeof(timeBuffer), "%H:%M:%S", ts)] = '\0';
 
		if(logFlags.displayColor){
			fprintf(stderr, "%s %s %-5s\x1b[0m \x1b[90m%s:%s:%d:\x1b[0m ", 
				timeBuffer, log_level_colors[logFlags.consoleLevel], 
				log_level_names[logFlags.consoleLevel], __FILE__, __FUNCTION__, __LINE__);
		}
		else{
			fprintf(stderr, "%s %-5s %s:%s:%d: ", 
				timeBuffer, log_level_names[logFlags.consoleLevel],
				__FILE__, __FUNCTION__, __LINE__);
		}

		va_start(args, fmt);
		vfprintf(stderr, fmt, args);
		va_end(args);
		fprintf(stderr, "\n");
	}

	// file logging
	if(level < logFlags.fileLevel && logFlags.fp != NULL){

		va_list args;
		char timeBuffer[32];


		/* format time into "ddd yyyy-mm-dd hh:mm:ss zzz" 
		ddd is day of week like SUN
		zzz is time zone like MST
		*/
		timeBuffer[strftime(timeBuffer, sizeof(timeBuffer), "%a %Y-%m-%d %H:%M:%S %Z", ts)] = '\0';

		fprintf(logFlags.fp, "%s %-5s %s:%s:%d: ", 
				timeBuffer, log_level_names[logFlags.consoleLevel],
				__FILE__, __FUNCTION__, __LINE__);

		va_start(args, fmt);
		vfprintf(logFlags.fp, fmt, args);
		va_end(args);
		fprintf(stderr, "\n");

	}
}


int parse_configuration_file(const char* configFileName)
{
	FILE *fp;

	char word[MAX_CHARACTERS_PER_WORD];
	int ivalue;

	fp = fopen(configFileName, "r");
	if(fp == NULL){
		fprintf(stderr, "%s %s %d, %s failed to open\n", __FILE__, 
				__FUNCTION__, __LINE__, configFileName);
		return 0;
	}

	// read each line of the file
	while(!feof(fp) && fscanf(fp, "%s%d", word, &ivalue) == 2){
		
		if(strcmp(word, "consoleLogLevel") == 0){
			if(ivalue <= LOG_FATAL)
			{
				logFlags.consoleLevel = ivalue;
			}
			else{
				logFlags.consoleLevel = LOG_INFO;
				fprintf(stderr, "%s %s %d, invalid console log level: %d, defaulting to %s\n",
							__FILE__, __FUNCTION__, __LINE__, ivalue, log_level_names[LOG_INFO]);
			}
			
		}
		else if(strcmp(word, "fileLogLevel") == 0){

			if(ivalue <= LOG_FATAL)
			{
				logFlags.fileLevel = ivalue;
			}
			else{
				logFlags.fileLevel = LOG_INFO;
				fprintf(stderr, "%s %s %d, invalid file log level: %d, defaulting to %s\n",
							__FILE__, __FUNCTION__, __LINE__, ivalue, log_level_names[LOG_INFO]);
			}

		}
		else if(strcmp(word, "writeToFile") == 0){
			// non-zero values will be converted to 1
			logFlags.writeToFile = ivalue == 0? 0:1;
		}
		else if(strcmp(word, "displayColor") == 0){
			logFlags.displayColor = ivalue == 0? 0:1;
		}
		else{
			fprintf(stderr, "%s %s %d, ignoring: %s, ignoring: %d\n",
							__FILE__, __FUNCTION__, __LINE__, word, ivalue); 
		}
	}

	fclose(fp);

	return 1;
}


void debuglog_init(const char* logFileName, const char* configFileName,
				int consoleLevel, int fileLevel)
{

	START HERE: REEVALUATE DEFAULT SETTINGS, HOW IS FILE OPENED?


	/** populate struct values */
	logFlags.fileName = logFileName;
	logFlags.fp = NULL;
	logFlags.consoleLevel = consoleLevel;
	logFlags.fileLevel = fileLevel;
	logFlags.writeToFile = 0;

	// parse configuration file if it exists
	if(configFileName != NULL){
		if(parse_configuration_file(configFileName)){
			logit(LOG_INFO, "Configuring logger from %s", configFileName);
		}
		else{
			logit(LOG_INFO, "No configuration file, initializing log values with defaults");
		}
	}

}