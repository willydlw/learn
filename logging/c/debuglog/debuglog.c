#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>

#include "debuglog.h"

#define MAX_CHARACTERS_PER_WORD 32


/* ==========     Static Global Variables     ========= */

static LogFlags logFlags;


const  char* log_level_string[] = 
			{	"LOG_NONE", "LOG_INFO", "LOG_DEBUG",  
				"LOG_WARN", "LOG_ERROR", "LOG_FATAL"};


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
							__FILE__, __FUNCTION__, __LINE__, ivalue, log_level_string[LOG_INFO]);
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
							__FILE__, __FUNCTION__, __LINE__, ivalue, log_level_string[LOG_INFO]);
			}

		}
		else if(strcmp(word, "writeToFile") == 0){
			// non-zero values will be converted to 1
			logFlags.writeToFile = ivalue == 0? 0:1;
		}
		else if(strcmp(word, "displayColor") == 0){
			logFlags.displayColor = ivalue == 0? 0:1;
		}
		else if(strcmp(word, "recordTimeStamp") == 0){
			logFlags.recordTimeStamp = ivalue == 0? 0:1;
		}
		else{
			fprintf(stderr, "%s %s %d, ignoring: %s, ignoring: %d\n",
							__FILE__, __FUNCTION__, __LINE__, word, ivalue); 
		}
	}

	return 1;
}


void debuglog_init(const char* logFileName, const char* configFileName,
				int consoleLevel, int fileLevel)
{
	/** populate struct values */
	logFlags.fileName = logFileName;
	logFlags.consoleLevel = consoleLevel;
	logFlags.fileLevel = fileLevel;
	logFlags.writeToFile = 0;
	logFlags.recordTimeStamp = 1;

	// parse configuration file if it exists
	if(configFileName != NULL){
		if(parse_configuration_file(configFileName)){
			debuglog_log(0, LOG_INFO, "Configuring logger from %s", configFileName);
		}
		else{
			debuglog_log(0, LOG_INFO, "No configuration file, initializing log values with defaults");
		}
	}

}