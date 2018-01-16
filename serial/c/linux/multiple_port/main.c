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
* TODO: UPDATE COMMENTS FOR MULTIPLE DEVICE BEHAVIOR
*
*   Communication method: serial
*       Default device path:  /dev/ttyACM0
*       Default baud rate:    9600 baud
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

#include <debuglog.h>

#include "operations.h"
#include "sensor.h"





int main(int argc, char **argv){

    SensorCommOperation sensorCommArray[SENSOR_LIST_LENGTH];

    // statistics
    int totalSensorCount = 0;
    int activeSensorCount = 0;
    int serialPortsOpened;

    if(argc < MIN_NUMBER_COMMAND_LINE_ARGS){
        log_fatal("argc: %d, minimum number command line arguments: %d", 
                    argc, MIN_NUMBER_COMMAND_LINE_ARGS);
        log_fatal("usage: a.out sensorInputFileName");
        return 1;
    }


    // show all messages at console level, 
    // do not write any to a file
    // color on
    log_init(LOG_TRACE, LOG_OFF, 1);


    serialPortsOpened = initialize_sensor_communication_operations(argv[1], 
        sensorCommArray, SENSOR_LIST_LENGTH, &totalSensorCount, &activeSensorCount);


    log_trace("totalSensorCount:    %2d", totalSensorCount);
    log_trace("active sensor count: %2d", activeSensorCount);
    log_trace("serial ports opened: %2d\n", serialPortsOpened);

    
    for(int i = 0; i < totalSensorCount; ++i){
        log_trace("id: %d, name: %s, active: %d, device path: %s, baud rate: %d",
            sensorCommArray[i].sensor.id, sensorCommArray[i].sensor.name,
            sensorCommArray[i].sensor.active, 
            sensorCommArray[i].sensor.devicePath,
            sensorCommArray[i].sensor.baudRate);
    }


	return 0;
}
