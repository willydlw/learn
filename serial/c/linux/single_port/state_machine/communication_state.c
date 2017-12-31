#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>


#include "serial.h"
#include "communication_state.h"


// Globals must be initialized in the c file and declared extern in the h file
const char* message_state_string[] = 
		{ "AWAITING_START_MARKER", "AWAITING_SENSOR_ID", "AWAITING_DATA_BYTE_ONE",
		  "AWAITING_DATA_BYTE_TWO", "AWAITING_DATA_BYTE_THREE", 
		  "AWAITING_END_MARKER", "MESSAGE_COMPLETE"};

const char* comm_state_string[] = {"<RDY>", "<ACK>", "<NCK>"};

const char* debug_comm_state_string[] = 
	{"UNUSED", "SEND_READY_QUERY", "WAIT_FOR_ACK", "WAIT_FOR_DATA"};


ErrorCondition confirm_connection(int fd, int max_fd){

	int keepTrying = 1;
	int count = 0;

	fd_set writefds;
	int num_fd_pending;
	struct timeval timeout;

	while(keepTrying){

		FD_ZERO(&writefds);
        FD_SET(fd, &writefds);

        // re-initialize the timeout structure or it will eventually become zero
        // as time is deducted from the data members. timeval struct represents
        // an elapsed time
        timeout.tv_sec = 2;                     // seconds
        timeout.tv_usec = 0;                    // microseconds

        ++count;

        /*
        int select(int nfds, fd_set *readfds, fd_set *writefds,
            fd_set *exceptfds, const struct timespec *timeout);

                nfds nfds is the highest-numbered file descriptor in any of the 
                    three sets, plus 1.

                timeout argument specifies maximum time select should wait 
                before returning. Will return sooner if fd is available

        select returns the number of file descriptors that have a pending condition
            or -1 if there was an error
        */
        num_fd_pending = select(max_fd, NULL, &writefds, NULL, &timeout);
        fprintf(stderr, "\n%s, select returned %d\n", __FUNCTION__, num_fd_pending);

        if(num_fd_pending < 0){
        	int errnum = errno;

        	fprintf(stderr, "select error, function %s, error: %s\n", __FUNCTION__, 
        				strerror(errnum));
        	return SELECT_ERROR;
        }
        else if(num_fd_pending == 0){
        	fprintf(stderr, "cannot write message, select returned %d\n", num_fd_pending);
        }
        else{
        	
            if(FD_ISSET(fd, &writefds)){
            	fprintf(stderr, "calling send_ready_signal\n");

                if( send_ready_signal(fd, MAX_WRITE_TRIES) == SUCCESS ){
                	keepTrying = 0;

                    fprintf(stderr, "send_ready_signal returned SUCCESS\n");
                   
                }
                else{ 
                	fprintf(stderr, "send_ready_signal failed to write, will try again\n");
        			fprintf(stderr, "number times loop has executed: %d\n", count);
                }
            }
            else{
            	fprintf(stderr, "FD_ISSET(fd, &writefds) not true for %d\n", fd);
            }
        }

        fprintf(stderr, "loop count: %d\n", count);

	} // end while

	fprintf(stderr, "total number times keep_trying loop executed: %d\n", count);
	fprintf(stderr, "exiting %s\n", __FUNCTION__);
	getchar();

	// returning 0. If function returns above due to an error, all errno values are
	// greater than 0
	return SUCCESS;
}


ErrorCondition send_ready_signal(int fd, int max_tries){

	ssize_t expected_bytes = (ssize_t)strlen(comm_state_string[SEND_READY_QUERY]);

    ssize_t bytes_written = 0;
    int num_tries = 0;

    bytes_written = serial_write(fd, comm_state_string[SEND_READY_QUERY], 
    								expected_bytes) ;
    ++num_tries;

    // keep sending until all bytes have been transmitted
    while( bytes_written != expected_bytes && num_tries <= max_tries){
    	fprintf(stderr, "error, function %s, did not write all bytes\n", __FUNCTION__);
        fprintf(stderr, "bytes_written: %ld, should have written %ld bytes\n", 
        				bytes_written, expected_bytes);

        bytes_written = serial_write(fd, comm_state_string[SEND_READY_QUERY], 
        								expected_bytes) ;
        ++num_tries;
    }

    if(bytes_written == expected_bytes){
    	return SUCCESS;		// message successfully transmitted
    }
    else{
    	return SERIAL_WRITE_ERROR;		// message not successfully transmitted
    }

}


ErrorCondition wait_for_acknowledgement(int fd, int max_fd){

	int keepTrying = 1;
	int count = 0;
	ssize_t bytes_read;
	uint8_t buf[16];

	fd_set readfds;
	int num_fd_pending;
	struct timeval timeout;

	MessageState receive_message_state;

	// response data
    char responseData[6];

	while(keepTrying){

		FD_ZERO(&readfds);
        FD_SET(fd, &readfds);

        // re-initialize the timeout structure or it will eventually become zero
        // as time is deducted from the data members. timeval struct represents
        // an elapsed time
        timeout.tv_sec = 2;                     // seconds
        timeout.tv_usec = 0;                    // microseconds

        ++count;

        /*
        int select(int nfds, fd_set *readfds, fd_set *writefds,
            fd_set *exceptfds, const struct timespec *timeout);

                nfds nfds is the highest-numbered file descriptor in any of the 
                    three sets, plus 1.

                timeout argument specifies maximum time select should wait 
                before returning. Will return sooner if fd is available

        select returns the number of file descriptors that have a pending condition
            or -1 if there was an error
        */
        num_fd_pending = select(max_fd, &readfds, NULL, NULL, &timeout);
        fprintf(stderr, "\n%s, select returned %d\n", __FUNCTION__, num_fd_pending);

        if(num_fd_pending < 0){
        	int errnum = errno;

        	fprintf(stderr, "select error, function %s, error: %s\n", __FUNCTION__, 
        				strerror(errnum));
        	return SELECT_ERROR;
        }
        else if(num_fd_pending == 0){
        	fprintf(stderr, "cannot read message, select returned %d\n", num_fd_pending);
        }
        else{
        	
            if(FD_ISSET(fd, &readfds)){

            	fprintf(stderr, "calling serial_read\n");

            	bytes_read = serial_read(fd, buf, 5);  // expecting <ACK>

                // debug
                fprintf(stderr, "serial_read returned bytes_read: %ld\n", bytes_read);
                

                if(bytes_read > 0){

                	// debug, print the bytes
                	for(int i = 0; i < bytes_read; ++i){
                    	fprintf(stderr, "%#x ", buf[i] );
            		}

                	fprintf(stderr, "\n, calling process_received_ack_bytes\n");
                	// end debug

                    receive_message_state = process_received_ack_bytes(receive_message_state, buf, 
                                bytes_read, responseData);

                    fprintf(stderr, "receive_message_state: %s\n", 
                    		message_state_string[receive_message_state]);

                    if(receive_message_state == MESSAGE_COMPLETE){

                    	keepTrying = 0;
                    	
                    	fprintf(stderr, "responseData: %s\n", responseData);

                        if(strcmp(responseData, comm_state_string[1]) == 0){
                            fprintf(stderr, "received ack\n");
                            return SUCCESS;
                        }
	                    else if(strcmp(responseData, comm_state_string[2]) == 0){ 
	                        // received NCK 
	                        fprintf(stderr, "received nck\n");
	                        return NACK_ERROR;
	                
	                    }
	                    else{
	                        // received unexpected response
	                        fprintf( stderr, "error: %s, unexpected response received %s\n", 
	                                    __FUNCTION__, responseData);
	                        return UNEXPECTED_RESPONSE_ERROR;
	                    }
                	}
                	else{
                		fprintf(stderr, "receive_message_state: %s, looping to read the rest\n", 
                			message_state_string[receive_message_state]);
                	}
            	}
            	else{
            		fprintf(stderr, "no bytes_read, error: %s\n", strerror(errno));
            		return SERIAL_READ_ERROR;
            	}
            }
            else {   // for debug only
                fprintf(stderr, "if(FD_ISSET(fd, &readfds)) not true for %d\n", fd);
            }
        }

        fprintf(stderr, "loop count: %d\n", count);

	} // end while

	// will not reach thi
	fprintf(stderr, "total number times keep_trying loop executed: %d\n", count);
	fprintf(stderr, "exiting %s\n", __FUNCTION__);
	getchar();

	return LOOP_COUNT_ERROR;

}


MessageState process_received_ack_bytes(MessageState msgState, const uint8_t *buf, ssize_t bytes_read,
										char *responseData)
{
	fprintf(stderr, "\nstart of %s, message state: %s\n", __FUNCTION__, message_state_string[msgState]);

	ssize_t i;
	for(i = 0; i < bytes_read; ++i){
		switch(msgState){
			case AWAITING_START_MARKER:
				if((char)buf[i] == start_marker){
					responseData[0] = buf[i];
					msgState = AWAITING_SENSOR_ID;
				}
				else { // log error
					fprintf(stderr, "error: %s, state: AWAITING_START_MARKER, buf[%ld]: %#x\n",
							__FUNCTION__, i, buf[i]);
				}
				break;
			case AWAITING_DATA_BYTE_ONE:
				responseData[1] = buf[1];
				msgState = AWAITING_DATA_BYTE_TWO;
				break;
			case AWAITING_DATA_BYTE_TWO:
				responseData[2] = buf[2];
				msgState = AWAITING_DATA_BYTE_THREE;
				break;
			case AWAITING_DATA_BYTE_THREE:
				responseData[3] = buf[3];
				msgState = AWAITING_END_MARKER;
				break;
			case AWAITING_END_MARKER:
				responseData[4] = buf[4];
				return MESSAGE_COMPLETE;
			default:
				fprintf(stderr, "error: %s, reached default case, msgState: %#x,"
						" message_state_string: %s\n", __FUNCTION__, msgState, 
						msgState < NUM_MESSAGE_STATES? message_state_string[msgState]:"unknown");
		}
	}
	return msgState;
}


MessageState process_received_data_bytes(MessageState msgState, const uint8_t *buf, ssize_t bytes_read,
										uint16_t *sensorData)
{
	
	fprintf(stderr, "\nstart of %s, message state: %s\n", __FUNCTION__, message_state_string[msgState]);

	ssize_t i;
	for(i = 0; i < bytes_read; ++i){

		fprintf(stderr, "start of loop, message state: %s, buf[%ld]: %#x\n", message_state_string[msgState],
					i, buf[i]);

		switch(msgState){
		case AWAITING_START_MARKER:
			if(buf[i] == start_marker){
				msgState = AWAITING_SENSOR_ID;
			}
			else { // log error
				fprintf(stderr, "error: %s, state: AWAITING_START_MARKER, buf[%ld]: %#x\n",
						__FUNCTION__, i, buf[i]);
			}
			break;
		case AWAITING_SENSOR_ID:
			if(valid_sensor_id(buf[i])){
				msgState = AWAITING_DATA_BYTE_ONE;
			}
			else { // log error
				fprintf(stderr, "error: %s, state: AWAITING_SENSOR_ID, invalid id: buf[%ld]: %#x\n",
						__FUNCTION__, i, buf[i]);
			}
			break;

		case AWAITING_DATA_BYTE_ONE:
			// assumes MSB received first
			*sensorData = (uint16_t)(((uint16_t)buf[i]) << 8);
			msgState = AWAITING_DATA_BYTE_TWO;
			break;

		case AWAITING_DATA_BYTE_TWO:
			*sensorData = (uint16_t) (*sensorData | (uint16_t)buf[i]);
			msgState = AWAITING_END_MARKER;
			break;

		case AWAITING_END_MARKER:
			if(buf[i] == end_marker){
				fprintf(stderr, "end marker recognized, complete message processed\n");
				fprintf(stderr, "time to alert program that new sensor data is available\n");

				// calling function here to process the new data received and then
				// set the state to AWAITING_START_MARKER in the case where we are
				// processing the end of one message and have the start of another
				// message in the buffer

				// It is tempting to create a NEW_DATA_RECEIVED state and return
				// that state right here. That would create a problem when there
				// are additional bytes in the buffer. Would have to alter the 
				// code to ensure these additional bytes are handled and not lost.
				// Still working out the best way to implement this. Will depend on
				// how rapidly sensor data must be processed and how much data.
				process_data_received(*sensorData);
				msgState = AWAITING_START_MARKER;
			}
			else { // log error
				fprintf(stderr, "error: %s, state: AWAITING_START_MARKER, buf[%ld]: %#x\n",
						__FUNCTION__, i, buf[i]);
			}
			break;
		default:
			fprintf(stderr, "\nerror: %s, reached default case\n", __FUNCTION__);
			fprintf(stderr, "current message state: %s, processing byte, buf[%ld]: %#x\n",
								message_state_string[msgState], i, buf[i]);
			fprintf(stderr, "setting message state to %s, some data will be lost\n", 
							message_state_string[AWAITING_START_MARKER]);
			fprintf(stderr, "will also cause AWAITING_START_MARKER error until next start marker is received\n");
			msgState = AWAITING_START_MARKER;

		}

	}

	fprintf(stderr, "exiting %s, returning message state: %s\n", __FUNCTION__, message_state_string[msgState]);

	return msgState;
}
	


bool valid_sensor_id(uint8_t id){
	// @TODO: need a scheme for quickly validating sensor ids
	//        once multiple sensors are handled
	if(id == sensor_id[0])
		return true;
	else 
		return false;
}


void process_data_received(uint16_t theData)
{
	fprintf(stderr, "\nentering %s, theData: %u\n", __FUNCTION__, theData);
	fprintf(stderr, "TODO: this is only a stub function, need to determine how to handle"
					" real time data updates\n");
}
