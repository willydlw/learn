#ifndef DEBUGLOG_H
#define DEBUGLOG_H


/** needed if this is used in cpp program */


#ifdef __cplusplus
extern "C"{
#endif

	/* define macros */
	#define log_all(...) \
		logit(LOG_ALL, __VA_ARGS__);

	#define log_debug(...) \
		logit(LOG_DEBUG, __VA_ARGS__);

	#define log_info(LEVEL, ...) \
		logit(LOG_INFO, __VA_ARGS__);

	#define log_warn(LEVEL, ...) \
		logit(LOG_WARN, __VA_ARGS__);

	#define log_error(LEVEL, ...) \
		logit(LOG_ERROR, __VA_ARGS__);

	#define log_fatal(LEVEL, ...) \
		logit(LOG_FATAL, __VA_ARGS__);

	#define log_off(LEVEL, ...) \
		logit(LOG_OFF, __VA_ARGS__);



	#define MAX_MESSAGE_SIZE 256

	/* levels */
	#define LOG_TRACE 0			// trace program execution

	#define LOG_DEBUG 1			// info that is diagnostically helpful to others, not 
								// just developers

	#define LOG_INFO  2			// generally useful information to log
								//	service start/stop, configuration assumptions

	#define LOG_WARN  3			// anything that can potentially cause application oddities
								//  but the application is automatically recovering

	#define LOG_ERROR 4			// errors that are fatal to the operation, but not the service
								//  or application. Will require attention.

	#define LOG_FATAL 5			// any error that forces a shutdown
	
	#define LOG_OFF   6 		// no logging 



	



	/* settings */
	typedef struct log_flags_t
	{
		const char* fileName;
		FILE *fp;
		int fileLevel;
		int consoleLevel;
		int writeToFile;
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
void debuglog_init(const char* logFileName, const char* configFileName,
				int consoleLevel, int fileLevel);

int parse_configuration_file(const char* configFileName);



#ifdef __cplusplus
}
#endif /* __cplusplus */



#endif /* DEBUGLOG_H */