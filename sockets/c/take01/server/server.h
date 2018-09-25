#ifndef SERVER_H_INCLUDED
#define SERVER_H_INCLUDED

#include <sys/socket.h>

void initialize_server_signal_environment();
int bind_server(const char* port, int backlog);
int server_accept_connection(int listen_fd, int timeout);
void *get_in_addr(struct sockaddr *sa);

#endif // SERVER_H_INCLUDED
