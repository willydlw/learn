/** Reference: Beej's Networking Guide
*
**/

#include <poll.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>             /// memset
#include <sys/types.h>

#include <netinet/in.h>
#include <netdb.h>              /// getaddrinf
#include <arpa/inet.h>          /// inet_ntop

#include <unistd.h>             /// close

#include "server.h"

/**
*   Must always call this function at the start of main, before
*   thread creation and creating the server
*
*   Purpose: keeps server from crashing when a client connection
*            is closed.
*
*   When a connected client is terminated, the server crashes. The reason
*   for the crash is that the write() call on the file fails, thus the
*   program receives a SIGPIPE, causing the server to exit.
*
*   From man (2) write:
*   EPIPE fd is connected to a pipe or socket whose reading end is closed.
*   When this happens the writing process will also receive a SIGPIPE signal.
*   (Thus, the write return value is seen only if the program catches, blocks
*   or ignores this signal.)
*
*   Thank you, Skyler Saleh, for cluing me in.
*/
void initialize_server_signal_environment()
{
    signal(SIGPIPE, SIG_IGN);       // ignore sigpipe

}

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

/** Reference: Beej's Networking Guide, pages 28 & 29
*
*   Connects to socketfd
*   Binds
*   Listens'
*
* \param port   - port number
* \param backlog - pending connections queue will hold
*/

int bind_server(const char* port, int backlog)
{
    int socketfd;

    int rv;                 /// return values

    int yes = 1;            /// for setsockopt

    struct addrinfo hints, *serverinfo, *p;

    /** The addrinfo structure used by getaddrinfo() contains the following
       fields:

           struct addrinfo {
               int              ai_flags;
               int              ai_family;
               int              ai_socktype;
               int              ai_protocol;
               socklen_t        ai_addrlen;
               struct sockaddr *ai_addr;
               char            *ai_canonname;
               struct addrinfo *ai_next;
           };
    */


    memset(&hints, 0, sizeof(hints));   /// zero out all struct data

    /** AF_UNSPEC is a wildcard that represents Address Family (AF) domain
    *   allows us to connect to IPv4 or IPv6 internet domain */
    hints.ai_family = AF_UNSPEC;


    /// SOCK_STREAM provides sequenced, reliable, two-way, connection-based byte streams.
    hints.ai_socktype = SOCK_STREAM;

    /** AI_PASSIVE causes the result's IP address to be filled out with INADDR_ANY
    *   (IPv4)or in6addr_any (IPv6); this causes a subsequent call to bind() to
    *   auto-fill the IP address of the struct sockaddr with the address of the
    *   current host. That's excellent for setting up a server when you don't want
    *   to hardcode the address.
    *   Beej's networking guide
    */
    hints.ai_flags = AI_PASSIVE;        /// fill in my IP for me


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

    rv = getaddrinfo(NULL, port, &hints, &serverinfo);

    if(rv != 0)
    {
        fprintf(stderr, "Error: getaddrinfo, %s\n", gai_strerror(rv));
        return -1;
    }

    /** iterate through the linked list of results. Bind to the first one we can */

    for(p = serverinfo; p != NULL; p = p->ai_next)
    {

        /** To create a socket, call the socket function
        *
        *   int socket(int domain, int type, int protocol);
        *
        *   domain argument determines the nature of the communication, including
        *   the address format.
        *
        *   type determines socket type
        *
        *   protocol is usually zero to select the default protocol for the given
        *   domain and socket type
        *
        *
        *   On success, a file descriptor for the new socket is returned.
        *   On error, -1 is returned, and errno is set appropriately.
        *
        */

        socketfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);


        /** avoid address already in use error message. Lets us reuse a port number that has
        *   not yet been cleared by the kernel, without waiting for the kernel to clear it
        *
        *   int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);
        *
        *   On success, zero is returned. On error, -1 is returned, and errno is set appropriately.
        *
        *   example call found in Beej's networking guide, adapted for our use here
        */

        int rv1 = setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));


        /** int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
        *
        *   On success, zero is returned. On error, -1 is returned, and errno is set appropriately.
        */

        int rv2 = bind(socketfd, p->ai_addr, p->ai_addrlen);

        fprintf(stderr, "%s, socketfd: %d, rv1: %d, rv2: %d\n", __FUNCTION__, socketfd, rv1, rv2);


        if(socketfd >= 0 && rv1 == 0 && rv2 == 0)
        {
            break;          /// bound and connected, break out of loop
        }
        else if(socketfd >= 0)
        {
            close(socketfd);    /// close the file descriptor so it may be reused
            socketfd = -1;
        }

    }       /// end for loop

    fprintf(stderr, "out of bind_server while loop\n");

    if(p == NULL)
    {
        fprintf(stderr, "Error: %s, failed to bind", __FUNCTION__);
        return -1;
    }

    freeaddrinfo(serverinfo);          /// free address structure memory

    /** int listen(int sockfd, int backlog);
    *
    *   The backlog argument defines the maximum length to which the queue of pending
    *   connections for sockfd may grow.
    *
    *   On success, zero is returned. On error, -1 is returned, and errno is set appropriately.
    */
    if(listen(socketfd, backlog) != 0)
    {
        perror("Failed to listen");
        if(socketfd >= 0)
            close(socketfd);
        return -1;
    }

    return socketfd;

}

/*** http://www.gnu.org/software/libc/manual/html_node/Accepting-Connections.html
*
*   A socket that has been established as a server can accept connection requests
*   from multiple clients. The server’s original socket does not become part of
*   the connection; instead, accept makes a new socket which participates in the
*   connection. accept returns the descriptor for this socket. The server’s original
*   socket remains available for listening for further connection requests.
*
*/

/**
*
*   \param listen_fd - file descriptor accepting the connection
*   \param timeout - number of milliseconds function should block
*
*   \returns on success, the connected file descriptor
*            on failure, -1
*/
int server_accept_connection(int listen_fd, int timeout )
{

    struct sockaddr_storage connector_addr;     /// connector's address information

    char s[INET6_ADDRSTRLEN];

    socklen_t sin_size = sizeof connector_addr;

    /** The accept function will wait(block) if there are no connections pending.
    *   We do not want to call this function unless the file descriptor listen_fd
    *   is ready. To ensure the program does not stay here indefinitely, we will
    *   poll the set of read file descriptors for a specified (timeout) period of
    *   time.
    *
    *   If listen_fd is ready to read within the timeout period, accept will be
    *   called. If not, this function will timeout and return without accepting
    *   the connection.
    */


    /** To use the poll function, we need to creat a pollfd object that specifies
    *   the set of file descriptort to be monitored.
    *
    *   The set of file descriptors to be monitored is specified in the
    *   fds argument, which is an array of structures of the following form:

        struct pollfd {
            int   fd;         // file descriptor
            short events;     // requested events
            short revents;    // returned events
        };

        POLLIN  there is data to read

    */

    struct pollfd poll_read = {.fd = listen_fd, .events = POLLIN};



    /** int poll(struct pollfd *fds, nfds_t nfds, int timeout);
    *
    *   waits for one of a set of file descriptors to become ready to perform I/O.
    *
    *   If none of the events requested (and no error) has occurred for any of the
    *   file descriptors, then poll() blocks until one of the events occurs.
    *
    *   The timeout argument specifies the minimum number of milliseconds that
    *   poll() will block.
    *
    */
    int rv = poll(&poll_read, 1, timeout*1000);
    if (rv == -1)
    {
        perror("poll: ");
    }

    if(poll_read.revents & POLLIN)
    {
        /** int accept (int socket, struct sockaddr *addr, socklen_t *length_ptr)
        *
        *   The accept function waits if there are no connections pending, unless
        *   the socket socket has nonblocking mode set.
        *
        *   The addr and length-ptr arguments are used to return information about
        *   the name of the client socket that initiated the connection.
        *
        *   Accepting a connection does not make socket part of the connection. Instead,
        *   it creates a new socket which becomes connected. The normal return value of
        *   accept is the file descriptor for the new socket.
        *
        *   After accept, the original socket socket remains open and unconnected,
        *   and continues listening until you close it. You can accept further connections
        *   with socket by calling accept again.
        */
        int new_fd = accept(listen_fd, (struct sockaddr *)&connector_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept: ");
            return -1;
        }

        fprintf(stderr, "newfd: %d\n", new_fd);

        /** inet_ntop - convert IPv4 and IPv6 addresses from binary to text form
        *
        *    Calling this function to convert address to text form for printing debugging message */
        inet_ntop(connector_addr.ss_family, get_in_addr((struct sockaddr *)&connector_addr), s, sizeof s);
        fprintf(stderr, "server: got connection from %s\n", s);

        return new_fd;
    }

    fprintf(stderr, "%s, poll time out\n", __FUNCTION__);
    return -1;
}




