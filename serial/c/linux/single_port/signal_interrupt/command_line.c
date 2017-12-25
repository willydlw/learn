#include "command_line.h"


#include <stdio.h>              // fprintf
#include <stdlib.h>             // atoi
#include <string.h>             // strcpy, strlen




CommandLineError process_command_line_arguments(int argc, 
                                                         char* argv[], 
                                                         char* serialDevice, 
                                                         int *baudRate,
                                                         int serialDeviceArrayLength)
{
    if(argc > 2){
        if(argc % 2){  // number of command line arguments must be odd
            return parse_command_line(argc, argv, serialDevice, baudRate, serialDeviceArrayLength);
        }
        else{
            fprintf(stderr, "main: error, incorrect number of command line arguments\n");
            fprintf(stderr, "usage: ./eserial -device /dev/ttyS0 -baud 9600\n");
            fprintf(stderr, "program terminating\n");
            return NUM_ARGUMENTS_ERROR;
        }
    }
    else{

        fprintf(stderr, "\n\nargc = %d, using serial device and baud rate defaults\n", argc);
        fprintf(stderr, "to override defaults, use command line arguments\n");
        fprintf(stderr, "usage: ./eserial -device /dev/ttyS0 -baud 9600\n\n");
    }

    return NO_ERROR;
}





CommandLineError parse_command_line(int argc, char* argv[], 
                                             char* serialDevice, int *baudRate,
                                             int serialDeviceArrayLength)
{
    int i = 1;
    
    while(i < argc-1){

        /// strcmp returns 0 when they are equal
        if(!strcmp(argv[i],"--baud") || !strcmp(argv[i],"-baud")){
            *baudRate = atoi(argv[i+1]);
        }
        else if(!strcmp(argv[i],"--device") || !strcmp(argv[i],"-device")){

            // ensure string length copied does not exceed array length
            if((int)strlen(argv[i+1]) <= serialDeviceArrayLength){

                strcpy(serialDevice,argv[i+1]);
            }
            else{

                fprintf(stderr, "error: length of %s is %lu exceeds array memory size of %d\n", 
                            argv[i+1], strlen(argv[i+1]), serialDeviceArrayLength);
                return LENGTH_ERROR;
            }
        }   
        else{
            fprintf(stderr, "warning, ignoring unknown command line argument: %s\n", argv[i]);
        }

        i = i + 2;
    }

    return NO_ERROR;
}