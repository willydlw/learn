/**
 * Copyright (c) 2017 willydlw
 *
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the MIT license. See `debuglog.c` for details.
 */


/** @file debuglog.h
 *
 *  @brief Function prototypes for logging library.
 *
 *  This contains the macros, enums, structs, and prototypes
 *  for the logging library.
 * 
 * @par Usage
 * 1. Initialize the logflags data members.
 *    
 *    The library uses a static instance of the LogFlags struct,
 *    named logFlags, declared in the debuglog.c file. Use either
 *    the log_init or log_config functions to initialize the 
 *    data member values.
 *
 *    log_config requires a configuration file.
 *    log_init uses the arguments passed.
 *
 *    consoleLevel logged data is written to stderr
 *    fileLevel logged is written to an output file.
 *
 *
 * 2. Specifying the logging level.
 *
 *    All messages at the level specified and above are
 *    logged.
 *
 *	  Example: 
 *		consoleLevel is LOG_INFO.
 *
 *			All message at a level of LOG_INFO and higher,
 *      	LOG_WARN, LOG_ERROR, and LOG_FATAL are written
 *      	to the console, stderr.
 *
 *       	No LOG_TRACE nor LOG_DEBUG messages will be
 *          written to the console.
 *
 *		fileLevel is LOG_WARN
 *
 *			All message at a level of LOG_WARN and higher,
 *      	LOG_ERROR, and LOG_FATAL are written
 *      	to the output file. 
 *
 *       	No LOG_TRACE, no LOG_DEBUG, nor LOG_INFO messages 
 *			will be written to the output file.
 *
 * 3. Use the macros defined below to log data.
 *		log_trace corresponds to the LOG_TRACE level
 *		log_debug corresponds to the LOG_DEBUG level, etc.
 *
 * 		The file name, function name, line number, and 
 *      logging level are written for each level. You may pass 
 *      a variable number of arguments to each macro. Passing no 
 *      arguments would give only the information listed above.
 *
 *      fprintf is used for logging. You must pass a formatted
 *      string and any variables needed for the formatting 
 *      specifiers used.
 *
 *		Examples: 
 *			log_trace("hello");
 *			log_trace("value: %d", intVariable);
 *			log_debug("total: %.2f, count: %d", fvalue, count);
 *
 *		All messages are terminated with a new line.
 *    
 *    
 *  @author willydlw
 *  @date 10 Jan 2018
 *  @bug No known bugs.
 */

#ifndef DEBUGLOG_H
#define DEBUGLOG_H


/* needed if this is used in cpp program */
#ifdef __cplusplus
extern "C"{
#endif

#include <stdio.h>				// FILE*

/** define logging level macros */
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


/** define default values */
#define DEFAULT_CONSOLE_LEVEL LOG_INFO
#define DEFAULT_FILE_LEVEL    LOG_OFF
#define DEFAULT_COLOR_DISPLAY 0
	

/**
* @brief logging levels
*/
typedef enum log_level_t{

	LOG_TRACE = 0, 	/**< trace program execution */
	LOG_DEBUG = 1,  /**< info that is diagnostically helpful to others, 
						 not just developers */
	LOG_INFO  = 2,  /**< generally useful information to log
						service start/stop, configuration assumptions */
	LOG_WARN  = 3,  /**< anything that can potentially cause application 
						 oddities, but the application is automatically 
						 recovering */
	LOG_ERROR = 4,  /**< errors that are fatal to the operation, but not 
						 the service or application. Will require 
						 attention. */
	LOG_FATAL = 5,  /**< any error that forces a shutdown */
	LOG_OFF   = 6,  /**< no logging */

}LogLevel;



/** 
* @brief Logging flags control level, color
*/
typedef struct log_flags_t
{
	FILE *fp;			/**< output log file pointer */
	int fileLevel;		/**< file logging level */
	int consoleLevel;	/**< console logging level */
	int displayColor;	/**< color on/off for console */
	
}LogFlags;



/** @brief 	Parses configuration file for console log level,
*			file log leve, and color display state.
*
*
* @param[in]	configFileName	  configuration file name to be parsed
*								  When NULL, default values are used
*
* @param[in]	consoleLevel	  console display log level
* @param[in]	fileLogLevel	  save to file log level
*
* @returns 	void		
*
* @note
*	The LogFlags data members are set by passing the following
*   strings and integer values:
*
*	data member 	string 			values
*	
*	fileLevel 		"fileLevel"     integer in range [0,6]
*	consoleLevel 	"consoleLevel" 	integer in range [0,6]
*   displayColor    "displayColor"  0 for off, 1 for on
*		
*	For fileLevel and consoleLevel, the integer values 
*   correspond to the LogLevel enumeration values for 
*   [LOG_TRACE, LOG_OFF]
*
*
*	The following default settings are used if the configuration
*   file cannot be opened or if the file does not contain a
*   valid string for the data members listed above. A LOG_WARN
*   message is written when the default values are used.
*/
void log_config(const char* configFileName);




void log_init(int consoleLogLevel, int fileLogLevel, int colorDisplayOn);

void set_flag_defaults(void);

void set_display_color(int onOff);

void set_file_level(int logLevel);

void set_console_level(int logLevel);


static int parse_configuration_file(const char* configFileName);

void close_log_file(void);


LogLevel get_console_level(void);

LogLevel get_file_level(void);

int color_display_state(void);


void logit(int level, const char* file, const char* function, 
			int line, const char *fmt, ...);



#ifdef __cplusplus
}
#endif /* __cplusplus */



#endif /* DEBUGLOG_H */