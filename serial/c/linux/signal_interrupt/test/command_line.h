#ifndef  COMMAND_LINE_H
#define COMMAND_LINE_H


typedef enum command_line_error_t { NO_ERROR = 1, 
			   NUM_ARGUMENTS_ERROR = -1,
			   LENGTH_ERROR = -2 } CommandLineError;



CommandLineError process_command_line_arguments(int argc, 
														 char* argv[], 
														 char* serialDevice, 
														 int *baudRate,
														 int serialDeviceArrayLength);

CommandLineError parse_command_line(int argc, char* argv[], 
                                             char* serialDevice, int *baudRate,
                                             int serialDeviceArrayLength);

#endif