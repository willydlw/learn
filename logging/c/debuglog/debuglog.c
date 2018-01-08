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

	reset		"\x1b[0m"
	gray		"\x1b[0;30m"		bold 		"\x1b[1;30m"
	red 		"\x1b[0;31m"		bold 		"\x1b[1;31m"
	green		"\x1b[0;32m"		bold 		"\x1b[0;32m"
	yellow		"\x1b[0;33m"		bold 		"\x1b[1;33m"
	blue		"\x1b[0;34m"
	magenta		"\x1b[0;35m"
	cyan 		"\x1b[0;36m"
	white		"\x1b[0;37m"

	\x1b[31;1m
	*/
static const char* log_level_colors[] ={
				"\x1b[1;35m", "\x1b[1;32;1m", "\x1b[1;37;1m", "\x1b[1;33;1m", "\x1b[1;31;1m", 
				"\x1b[1;34;1m"
};


void logit(int level, const char* file, const char* function, int line, const char *fmt, ...)
{
	time_t currentTime;
	struct tm  *ts;
	

	if(level > logFlags.consoleLevel && level > logFlags.fileLevel){
		return;
	}


	// get current time
	currentTime = time(NULL);
	ts = localtime(&currentTime);

	
	// console logging
	if(level <= logFlags.consoleLevel){
		
		va_list args;
		
		char timeBuffer[16];					// stores formatted time

		// format time into hours:min:sec
		timeBuffer[strftime(timeBuffer, sizeof(timeBuffer), "%H:%M:%S", ts)] = '\0';
 
		if(logFlags.displayColor){
			// display level in color
			// display file:funct:line in gray
			// other text in normal color
			fprintf(stderr, "%s %s%-5s\x1b[0m \x1b[90m%s:%s:%d:\x1b[0m ", 
				timeBuffer, log_level_colors[level], 
				log_level_names[level], file, function, line); 
		}
		else{
			fprintf(stderr, "%s %-5s %s:%s:%d: ", 
				timeBuffer, log_level_names[level],
				file, function, line);
		}

		va_start(args, fmt);
		vfprintf(stderr, fmt, args);
		va_end(args);
		fprintf(stderr, "\n");
	}

	// file logging
	if(level <= logFlags.fileLevel && logFlags.fp != NULL){

		va_list args;
		char timeBuffer[32];


		/* format time into "ddd yyyy-mm-dd hh:mm:ss zzz" 
		ddd is day of week like SUN
		zzz is time zone like MST
		*/
		timeBuffer[strftime(timeBuffer, sizeof(timeBuffer), "%a %Y-%m-%d %H:%M:%S %Z", ts)] = '\0';

		fprintf(logFlags.fp, "%s %-5s %s:%s:%d: ", 
				timeBuffer, log_level_names[level], file, function, line);

		va_start(args, fmt);
		vfprintf(logFlags.fp, fmt, args);
		va_end(args);
		fprintf(logFlags.fp, "\n");

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
			set_console_level(ivalue);
		}
		else if(strcmp(word, "fileLogLevel") == 0){
			set_file_level(ivalue);
		}
		else if(strcmp(word, "displayColor") == 0){
			logFlags.displayColor = ivalue == 0? 0:1;
		}
		else{
			logit(DEFAULT_CONSOLE_LEVEL, __FILE__, __FUNCTION__,
					__LINE__, "ignoring: %s, ignoring: %d\n", word, ivalue); 
		}
	}

	fclose(fp);

	return 1;
}

void log_init(int consoleLogLevel, int fileLogLevel, int colorDisplayOn)
{
	set_console_level(consoleLogLevel);
	set_file_level(fileLogLevel);
	set_display_color(colorDisplayOn);
}


void set_flag_defaults(void)
{
	/** populate struct values */
	logFlags.fp = NULL;
	logFlags.consoleLevel = DEFAULT_CONSOLE_LEVEL;
	logFlags.fileLevel = DEFAULT_FILE_LEVEL;
	logFlags.displayColor = 0;

}


void set_display_color(int onOff)
{
	logFlags.displayColor = onOff == 0? 0:1;
}


void set_console_level(int logLevel)
{
	if(logLevel <= LOG_OFF){
		fprintf(stderr, "logLevel <= LOG_OFF true\n");
		logFlags.consoleLevel = logLevel;
		fprintf(stderr, "logFlags.consoleLevel set to %s\n", log_level_names[logFlags.consoleLevel]);
	}
	else{
		
		logFlags.consoleLevel = DEFAULT_CONSOLE_LEVEL;
		logit(DEFAULT_CONSOLE_LEVEL, __FILE__, __FUNCTION__, __LINE__,
				"invalid console level: %d, using default %s", 
				logLevel, log_level_names[DEFAULT_CONSOLE_LEVEL]);
	}
}


void set_file_level(int logLevel)
{
	if(logLevel <= LOG_OFF){
		logFlags.fileLevel = logLevel;
	}
	else{
		
		logFlags.fileLevel = DEFAULT_FILE_LEVEL;
		logit(LOG_WARN, __FILE__, __FUNCTION__, __LINE__,
				"invalid file level: %d, using default %s", 
				logLevel, log_level_names[DEFAULT_FILE_LEVEL]);
	}

	if(logFlags.fileLevel < LOG_OFF){
		char filename[256];

		// apend date, time to file name
		time_t currentTime = time(NULL);
		struct tm *ts = localtime(&currentTime);
		strftime(filename, sizeof(filename), "/log/logdata_%Y-%m-%d_%H:%M:%S", ts);

		logFlags.fp = fopen(filename, "w");
		if(logFlags.fp == NULL){
			logit(LOG_WARN, __FILE__, __FUNCTION__, __LINE__,
					"log file: %s did not open", filename);
		}
	}
}


void log_config(const char* configFileName)
{

	/** populate struct values */
	set_flag_defaults();

	// parse configuration file if it exists
	if(configFileName != NULL){
		if(parse_configuration_file(configFileName)){
			logit(LOG_INFO, __FILE__, __FUNCTION__, __LINE__,
					"Configuring logger from %s", configFileName);
		}
		else{
			logit(LOG_WARN, __FILE__, __FUNCTION__, __LINE__,
					"No configuration file, initializing log values with defaults");
		}
	}

}