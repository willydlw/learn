/**
 * Copyright (c) 2017 willydlw
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See `debuglog.c` for details.
 */

#ifndef DEBUGLOG_H
#define DEBUGLOG_H


/** needed if this is used in cpp program */


#ifdef __cplusplus
extern "C"{
#endif

	#include <stdio.h>				// FILE*

	/* define macros */
	#define log_trace(...) \
		logit(LOG_TRACE, __FILE__, __func__ ,__LINE__, __VA_ARGS__);

	#define log_debug(...) \
		logit(LOG_DEBUG, __FILE__, __func__ ,__LINE__, __VA_ARGS__);

	#define log_info(...) \
		logit(LOG_INFO, __FILE__, __func__ ,__LINE__, __VA_ARGS__);

	#define log_warn(...) \
		logit(LOG_WARN, __FILE__, __func__ ,__LINE__, __VA_ARGS__);

	#define log_error(...) \
		logit(LOG_ERROR, __FILE__, __func__ ,__LINE__, __VA_ARGS__);

	#define log_fatal(...) \
		logit(LOG_FATAL, __FILE__, __func__ ,__LINE__, __VA_ARGS__);

	#define log_off(...) \
		logit(LOG_OFF, __FILE__, __func__ ,__LINE__, __VA_ARGS__);



	/* logging levels

	LOG_TRACE 0			trace program execution

	LOG_DEBUG 1			info that is diagnostically helpful to others, not 
						just developers

	LOG_INFO  2			generally useful information to log
						service start/stop, configuration assumptions

	LOG_WARN  3			anything that can potentially cause application oddities
						but the application is automatically recovering

	LOG_ERROR 4			errors that are fatal to the operation, but not the service
						or application. Will require attention.

	LOG_FATAL 5			any error that forces a shutdown
	
	LOG_OFF   6 		no logging 

	*/

	typedef enum log_level_t{
		LOG_TRACE, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR, LOG_FATAL, LOG_OFF
	}LogLevel;


	#define DEFAULT_CONSOLE_LEVEL LOG_INFO
	#define DEFAULT_FILE_LEVEL    LOG_OFF


	/* settings */
	typedef struct log_flags_t
	{
		FILE *fp;
		int fileLevel;
		int consoleLevel;
		int displayColor;
		
	}LogFlags;


/**
* NAME : void debuglog_init(const char* logFileName, const char* configFileName,
				int logLevel, int fileLevel);
*
* DESCRIPTION: Function parses configuration file, reads the log level.
*
*              
* INPUTS: 
*   Parameters:
*		const char*		logFileName		  log file name where log information 
*										  is written.
*
*		const char*		configFileName	  configuration file name to be parsed
*										  When NULL, default values are used
*
*		int 			consoleLevel	  console display log level
*		int 			fileLogLevel	  save to file log level
*
*							  
*   Return:
*		void		 	returns nothing
*
*
*      
* NOTES:
*		
*
*/
void log_config(const char* configFileName);


void logit(int level, const char* file, const char* function, int line, const char *fmt, ...);

void log_init(int consoleLogLevel, int fileLogLevel, int colorDisplayOn);

void set_flag_defaults(void);

void set_display_color(int onOff);

void set_file_level(int logLevel);

void set_console_level(int logLevel);


int parse_configuration_file(const char* configFileName);

void close_log_file(void);


LogLevel get_console_level(void);

LogLevel get_file_level(void);

int color_display_state(void);





#ifdef __cplusplus
}
#endif /* __cplusplus */



#endif /* DEBUGLOG_H */