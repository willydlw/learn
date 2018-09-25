#ifndef CLIENT_H_INCLUDED
#define CLIENT_H_INCLUDED

#include <sys/socket.h>

void *get_in_addr(struct sockaddr *sa);

/**
*   addr is hostname
*/
int client_connect_socket(const char* addr, const char* port);
int send_data(int socketfd, char* data, int len);
int recv_data(int socketfd, char* data, int len);

#endif // CLIENT_H_INCLUDED
