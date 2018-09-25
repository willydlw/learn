#include <stdio.h>
#include <string.h>             /// memset
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>              /// getaddrinf
#include <arpa/inet.h>          /// inet_ntop

#include <unistd.h>             /// close


#include "client.h"

/** http://beej.us/guide/bgnet/examples/server.c
*
*   get sockaddr, IPv4 or IPv6:
*/
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}


int client_connect_socket(const char* addr, const char* port)
{
    int socketfd;

    struct addrinfo hints, *servinfo, *ptr;

    char s[INET6_ADDRSTRLEN];

    memset(&hints, 0, sizeof hints);    /// zero out all struct data

    /** AF_UNSPEC is a wildcard that represents Address Family (AF) domain
    *   allows us to connect to IPv4 or IPv6 internet domain */
    hints.ai_family = AF_UNSPEC;

    /// SOCK_STREAM provides sequenced, reliable, two-way, connection-based byte streams.
    hints.ai_socktype = SOCK_STREAM;


    /** int getaddrinfo(const char *node, const char *service,
                       const struct addrinfo *hints,
                       struct addrinfo **res);
    *
    *   Input parameters:
    *   The node parameter is the host name to connect to, or an IP address.
    *
    *   For us, the service is our port number
    *
    *   The hints parameter points to a struct addrinfo, that is already filled out
    *   with relevant information.
    *
    *   Output parameter:
    *   res is a pointer to a linked-list of results
    *
    *   The getaddrinfo() function allocates and initializes a linked list of addrinfo
    *   structures, one for each network address that matches node and service, subject
    *   to any restrictions imposed by hints, and returns a pointer to the start of
    *   the list in res.  The items in the linked list are linked by the ai_next field.
    *
    *   http://man7.org/linux/man-pages/man3/getaddrinfo.3.html
    *
    *   getaddrinfo() returns 0 if it succeeds, or a non-zero error code
    *
    */
    int rv = getaddrinfo(addr, port, &hints, &servinfo);
    if (rv != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return -1;
    }


    /** iterate through the linked list of results. Connect to the first one we can */
    for(ptr = servinfo; ptr != NULL; ptr = ptr->ai_next)
    {
        socketfd = socket(ptr->ai_family, ptr->ai_socktype,ptr->ai_protocol);

        /** int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
        *
        *  connects the socket referred to by the file descriptor sockfd to the
        *  address specified by addr.
        *
        *  If the connection or binding succeeds, zero is returned.
        *  On error, -1 is returned, and errno is set appropriately.
        */
        rv = connect(socketfd, ptr->ai_addr, ptr->ai_addrlen);

        /** When the file descriptor is not -1 and connect does not return -1
        *   there is a good connection. Break out ot the oop */
        if( socketfd != -1 && rv != -1)
            break;

        /// close the file descriptor if we have a valid socket fd
        if(socketfd != -1)
        {
            close(socketfd);
            socketfd = -1;
        }
    }

    if (ptr == NULL)
    {
        fprintf(stderr, "Error: %s, client: failed to connect\n", __FUNCTION__);
        return -1;
    }

    inet_ntop(ptr->ai_family, get_in_addr((struct sockaddr *)ptr->ai_addr), s, sizeof(s));

    fprintf(stderr, "client: connecting to %s\n", s);

    freeaddrinfo(servinfo);                 /// free address structure memory

    return socketfd;
}


int recv_data(int socketfd, char* data, int len)
{
    int index = 0;
    while( index < len)
    {
        int bytes_read = recv(socketfd, &data[index], len-index, 0);
        if(bytes_read < 1)
        {
            if(socketfd != -1)
                close(socketfd);
            return -1;
        }

        index += bytes_read;
    }
    return socketfd;
}


int send_data(int socketfd, char* data, int len)
{
    int index = 0;

    fprintf(stderr, "%s, data: %s, len: %d\n", __FUNCTION__, data, len);

    /** loop until all data is sent or send function fails
    *
    *   Upon successful completion, send() shall return the number of bytes sent.
    *   Otherwise, -1 shall be returned and errno set to indicate the error.
    */
    while( index < len)
    {
        /** ssize_t send(int sockfd, const void *buf, size_t len, int flags);
        */
        ssize_t bytes_sent = send(socketfd, &data[index], len-index,0);

        fprintf(stderr, "bytes_sent: %ld\n", bytes_sent);

        if(bytes_sent < 1)  /// send failed
        {
            if(socketfd != -1)
            {
                close(socketfd);
                return -1;
            }
        }

        index += bytes_sent;

    }
    return socketfd;
}
