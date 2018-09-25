#ifndef CLIENTSERVER_H_INCLUDED
#define CLIENTSERVER_H_INCLUDED

#include <stdint.h>
#include <sys/socket.h>



/*** server functions ***/

#ifdef __cplusplus
extern "C" void intialize_server_signal_environment(void);
extern "C" int bind_server(const char* port, int backlog);
extern "C" int server_accept_connection(int listen_fd, int timeout);
#else
void initialize_server_signal_environment(void);
int bind_server(const char* port, int backlog);
int server_accept_connection(int listen_fd, int timeout);
#endif



/*** client functions **/
#ifdef __cplusplus

/**
*   addr is hostname
*/
extern "C" int client_connect_socket(const char* addr, const char* port);

/*** client and server functions **/
extern "C" void *get_in_addr(struct sockaddr *sa);
extern "C" int send_data(int socketfd, const uint8_t* data, size_t len);
extern "C" int recv_data(int socketfd, uint8_t* data, size_t len);

#else

/**
*   addr is hostname
*/
int client_connect_socket(const char* addr, const char* port);



/*** client and server functions **/
void *get_in_addr(struct sockaddr *sa);

int send_data(int socketfd, const uint8_t* data, size_t len);
int recv_data(int socketfd, uint8_t* data, size_t len);

#endif



#endif // CLIENT_SERVER_H_INCLUDED
