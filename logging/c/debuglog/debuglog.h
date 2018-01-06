#ifndef DEBUGLOG_H
#define DEBUGLOG_H


/** needed if this is used in cpp program */


#ifdef __cplusplus
extern "C"{
#endif

	extern const char* log_level_string[];

	/* levels */
	typedef enum log_level_t{
		LOG_NONE, LOG_INFO, LOG_DEBUG,  LOG_WARN, LOG_ERROR, LOG_FATAL
	} LogLevel;



	/* flags */
	typedef struct log_flags_t
	{
		const char* fileName;
		int fileLevel;
		int consoleLevel;
		int writeToFile;
		int recordTimeStamp;
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