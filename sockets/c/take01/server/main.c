/**
*
* Filename: main.c
*
* 1. Start the server first.
* 2. Server will be bound to the port passed as command line argument argv[1]
*
*
*   References: http://haifux.org/lectures/171/multi.c
                http://www.gnu.org/software/libc/manual/html_node/Server-Example.html
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>             // close

#include "server.h"


#define BACKLOG_QUEUE_SIZE 5
#define READ_BUFFER_LENGTH 256

int main(int argc, char ** argv)
{
    /** local variable declarations **/

    /// socket file descriptors
    int server_socketfd, newsocketfd;

    /// file descriptor sets
    fd_set master_fd_set, read_fd_set;

    /// maximum file descriptor number
    int fdmax;

    /// read buffer
    char read_buffer[READ_BUFFER_LENGTH];
    ssize_t bytes_read;

    int i;


    if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }


    initialize_server_signal_environment();

    /// server network port number is argv[1]
    if( (server_socketfd = bind_server(argv[1], BACKLOG_QUEUE_SIZE)) > 0)
    {
        fprintf(stderr, "server initialized, listening on port number %d\n", atoi(argv[1]));
    }
    else
    {
        fprintf(stderr, "error: server failed to intialize, program terminating\n");
        return -1;
    }

    /** Initialize the set of active sockets. Initially, there is only
    *   one active socket open, listening to receive new incoming connections. **/
    FD_ZERO(&master_fd_set);
    FD_ZERO(&read_fd_set);
    FD_SET(server_socketfd, &master_fd_set);

    fdmax = server_socketfd;

    /** Program now enters an infinite while loop. There is nothing to do until
    *   a client makes a connection.
    */
    while(1)
    {

        fprintf(stderr, "while loop waiting on select, waiting for a client request\n");
        read_fd_set = master_fd_set;


        /** int select(int nfds, fd_set *readfds, fd_set *writefds,
        *               fd_set *exceptfds, struct timeval *timeout);
        *
        *   select() systems call waits until any of the file decriptors
        *   specified in the erad, write, and exception sets given to it
        *   are ready to give data, send data, or is in an exception state.
        *
        *   The function call will wait for a given time before returning.
        *   Passing the value NULL for timeout causes this function call to
        *   block (not timeout).
        */
        if( select(fdmax+1, &read_fd_set, NULL, NULL, (struct timeval *)NULL) == -1)
        {
            perror("select ");
            break;
        }

        /* Service all the sockets with input pending. */
        for (i = 0; i <= fdmax; ++i)
        {
            /** when the server socket is ready for reading, this means a new connection
            *   request has arrived */

            if (FD_ISSET (i, &read_fd_set)){
                if (i == server_socketfd) {   /// connection request on server socket
                    fprintf(stderr, "i: %d, server_socketfd: %d\n", i, server_socketfd);
                    fprintf(stderr, "calling server_accept_connection\n");
                    newsocketfd = server_accept_connection(server_socketfd, 1000);  /// timeout 1 second
                    if(newsocketfd != -1) {
                        FD_SET(newsocketfd, &master_fd_set); /// add new connection to active set
                        if(newsocketfd > fdmax){
                            fdmax = newsocketfd;        /// keep track of max file descriptor value
                        }
                    }
                    else {
                        fprintf(stderr, "error, did not accept new client\n");
                    }
                }
                else {   /// data arriving on already connected socket

                    fprintf(stderr, "data arrived on already connected socket, i: %d\n", i);

                    /** ssize_t recv(int sockfd, void *buf, size_t len, int flags); */
                    bytes_read = recv(i, read_buffer, sizeof(read_buffer), 0);

                    fprintf(stderr, "server main, bytes read: %ld\n", bytes_read);
                    if(bytes_read < 0){
                        perror("recv: ");
                        break;
                    }
                    else if(bytes_read == 0){    /// client closed connection
                        fprintf(stderr, "socket %i hung up\n", i);
                        close(i);
                        FD_CLR(i, &master_fd_set);
                    }
                    else{
                        int j;
                        for(j = 0; j < bytes_read; ++j)
                        {
                            fprintf(stderr, "%c", read_buffer[j]);
                        }
                        fprintf(stderr, "\n\n");

                        /// ssize_t send(int sockfd, const void *buf, size_t len, int flags);
                        send(i, "server got message", 18, 0);
                    }
                }
            }
            //else if(FD_ISSET, &write_fd_set){}
        } /// end for

    }

    if(server_socketfd != -1){
        close(server_socketfd);
    }

    return 0;

}
