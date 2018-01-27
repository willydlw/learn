/* 
* Copyright (c) 2017 willydlw
*
* Permission is hereby granted, free of charge, to any person obtaining a 
* copy of this software and associated documentation files (the "Software"), 
* to deal in the Software without restriction, including without limitation 
* the rights to use, copy, modify, merge, publish, distribute, sublicense, 
* and/or sell copies of the Software, and to permit persons to whom the 
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included 
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS 
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN 
* THE SOFTWARE.
*
*/


/**
* @file main.c
*
* @brief This program is meant as a generic framework for serial communication
*        with multiple serial connections. This program's purpose is to receive 
*        data from the other connected devices and prepare it for further 
*        processing.
*
*   Usage: ./erun sensorList.txt
*
*       where 
*           erun is the executable file name
*           sensorList.txt is input file name. May be any name.
*
*   Communication method: serial
*       
*       
*   Signal handling is implemented for SIGINT and SIGTERM
*       ctrl+c produces the SIGINT
*
*       Either of these signals causes an exit from the main
*       while loop, so that any connections may be closed, and 
*       any end of program data is recorded before the program
*       terminates.       
*
*       signal handling references:
*           http://www.linuxprogrammingblog.com/all-about-linux-signals?page=show
*           http://man7.org/linux/man-pages/man2/select_tut.2.html
*
*
*   Order of operations:
*
*       verify minimum number of command line arguments were passed to main

*       initialize debug logging struct
*       initialize serial communication
*       register the signal handlers
*       set intial state to WAIT_FOR_CONNECTION
*
*
*       while( no exit request from signal interrupt)
*
*           if in a read state, read available data
*           else if in a write state, write data
*
*           state WAIT_FOR_CONNECTION
*               remain in this state until hello message received
*               transition to SEND_READY_SIGNAL state 
*
*           state SEND_READY_SIGNAL
*               write ready message
*               transition to READ_ACK state
*
*           state READ_ACK
*               remain in this state until acknowledge message received
*               transition to READ_SENSOR state
*
*           state READ_SENSOR
*               read received message
*               upon receiving a complete message
*                   validate sensor id
*                   extra sensor data
*                   process sensor data
*               remain in READ_SENSOR state until a reset or stop condition
*            
*           state SEND_RESET
*           state SEND_STOP
*               neither of these states has been programmed yet
*               they currently serve as a place holder and will
*               be implemented in the future if needed.
*               
*
*       write summary statistics
*       close serial connection
*       
*
* @note Program contains a number of debug logging messages that are written 
*       to the standard error stream and an output file. This allows the user 
*       to watch the program flow to ensure messages are correctly received and
*       indicate all error conditions.
*
*
* @author willydlw
* @date 14 Jan 2018
* @bugs No known bugs
*
*
*/


#define _POSIX_C_SOURCE 200112L          // pselect

#include <errno.h>

#include <debuglog.h>

#include "operations.h"
#include "sensor.h"





int main(int argc, char **argv){

    // Data structure that contains sensor and communication
    // state information for each device
    SensorCommOperation sensorCommArray[SENSOR_LIST_LENGTH];

    // Debugging and operational statistics
    DebugStats debugStats = {0};
    

    // variables needed to monitor multiple file descriptors
    fd_set readfds;                     // read file descriptor set
    fd_set writefds;                    // write file descriptor set

    int maxfd;                          // largest file descriptor value             
            
    int selectReturn;                   // number read/write file descriptors pending
    int readCount;                      // number of sensors in a read state
    int writeCount;                     // number of sensors in a write state
   
    struct timespec timeout;            // timeout for pselect

    ErrorCondition errorCondition;      // select and serial error conditions

    // bits set to one represent sensor with complete message received/transmitted
    uint32_t completedList = 0;  

    // loop variable
    int i;

    
    // Verify the minimum number of arguments were passed to main
    if(argc < MIN_NUMBER_COMMAND_LINE_ARGS){
        log_fatal("argc: %d, minimum number command line arguments: %d", 
                    argc, MIN_NUMBER_COMMAND_LINE_ARGS);
        log_fatal("usage: a.out sensorInputFileName");
        return 1;
    }


    // show all messages at console level, 
    // write DEBUG and higher level to a file
    // color on at console level
    log_init(LOG_TRACE, LOG_DEBUG, 1);


    if( initialize_sensor_communication_operations(argv[1], 
            sensorCommArray, SENSOR_LIST_LENGTH, &debugStats) == false)
    {
        log_fatal("initialize_sensor_communication_operations failed");
        return 1;
    }

    
    if(debugStats.serialPortsOpened < 1){
        log_fatal("no serial ports opened, program terminating");
        return 1;
    }

    
    // log successful initialization
    log_info("SUCCESS, initialized sensor communication data members");
    log_info("totalSensorCount:    %4d", debugStats.totalSensorCount);
    log_info("active sensor count: %4d", debugStats.activeSensorCount);
    log_info("serial ports opened: %4d\n", debugStats.serialPortsOpened);
    log_info("active sensor list:  %#4x", debugStats.activeSensorList);

      

    // find the file descriptor with the largest value
    // Note that we are only finding this value once, with the
    // assumption that the serial connection is not closed
    // and reopened with a different fd or that another sensor
    // is not added at a later time with another fd that might
    // be larger.

    // If either of the above happens at some other point
    // in the program, must ensure that this value of
    // maxfd is still the largest numeric value.
    // Currently, there is no code that closes and reopens a serial port
    maxfd = find_largest_fd(sensorCommArray, debugStats.totalSensorCount);

    // pselect requires an argument that is 1 more than
    // the largest file descriptor value
    maxfd = maxfd + 1;   

    // pselect does not change the timeout argument, so we only need to
    // initialize it here. If the code is changed to use select instead
    // of pselect, then this initialization should be moved inside the
    // while loop, as select updates the timeout argument, deducting
    // elapsed time from it. 
    // reference: http://man7.org/linux/man-pages/man2/select.2.html 
    timeout.tv_sec = 2;                     // seconds
    timeout.tv_nsec = 0;                    // nanoseconds   


    // infinite while loop that will eventually be changed to
    // while(!exit_request) cannot do it right now as the
    // signal handler has not yet been integrated into this program
    while(1){

        // adds all open file descriptor to the appropriate read/write set
        build_fd_sets(sensorCommArray, debugStats.totalSensorCount, &readCount, &writeCount,
                    &readfds, &writefds);

        
        //selectReturn = pselect(maxfd, &readfds, &writefds, NULL, &timeout, &empty_mask);
        selectReturn = pselect(maxfd, &readfds, &writefds, NULL, &timeout, NULL);

        /** TODO: Observe select return count, is it ever larger than 1 ?
                  It appears it will not be as there is only one hardware UART 

            Logging statement below is for debugging behavior only. Remove when
            behavior is confirmed.
        */
        if(selectReturn < (readCount + writeCount)) {
            log_debug("readCount: %d, writeCount: %d, selectReturn %d", 
                    readCount, writeCount, selectReturn);   
        }
        

        ADD CODE BACK FOR SIGNAL HANDLER 

        /*if(exit_request){
        log_info("received exit request");
        break;
        } */


        UPDATE PROGRAM TO TOTAL SENSOR FAILURE COUNTS

        SHOULD PROGRAM TERMINATE AFTER A CERTAIN NUMBER OF FAILURES 
        OR SEND FLAG TO ALERT ANOTHER PROGRAM ??


        errorCondition = check_select_return_value(selectReturn, errno, &debugStats.selectZeroCount);

        if(errorCondition != SUCCESS){
            // return to while(test condition)
            continue;
        }


        // read only when a device has a read state
        if(readCount > 0){

            completedList = read_fdset(sensorCommArray, totalSensorCount, &readfds);

            // process each completed task in the array
            for(i = 0; i < totalSensorCount; ++i){
                

                // array index location is the same as the sensor id
                // bit mask is 1 << i
                if( (completedList & (uint32_t)(1 << i)) ){

                    log_trace("sensor id: %d in completedList: %#x, (1 << %d): %#x", 
                        sensorCommArray[i].sensor.id, completedList, i, 1 << i);

                    process_operational_state(&sensorCommArray[i]);
                    
                }
            }
        }

 
        if(writeCount > 0){

            completedList = write_fdset(sensorCommArray, totalSensorCount, &writefds);

            // process each completed task in the array
            for(i = 0; i < totalSensorCount; ++i){
                
                // array index location is the same as the sensor id
                // bit mask is 1 << i
                if( (completedList & (uint32_t)(1 << i)) ){

                    log_trace("sensor id: %d in completedList: %#x, (1 << %d): %#x", 
                        sensorCommArray[i].sensor.id, completedList, i, 1 << i);

                    process_operational_state(&sensorCommArray[i]);
                    
                }
            }
        }

    } // end while(1)
      
    

    close_serial_connections(sensorCommArray, totalSensorCount);


	return 0;
}
