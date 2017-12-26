#ifndef COMMUNICATION_STATE_H
#define COMMUNICATION_STATE_H


typedef enum comm_state_t { SIGNAL_READY, WAIT_FOR_ACK} CommState;


static const char* comm_state_string[] = {"<RDY>", "<ACK>"};


void send_ready_signal(int fd);




#endif