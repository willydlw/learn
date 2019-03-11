#include <cstdio>               // perror
#include <cstring>              // memset
#include <unistd.h>             // close

#include <iostream>
#include <cerrno>

#include <arpa/inet.h>          // inet_ntop
#include <netdb.h>              

#include <sys/types.h>
#include <sys/socket.h>

#include <debuglog/debuglog.h>

#include "socketServer.h"



namespace mysocket{

    SocketServer::SocketServer(){
        socketfd = -1;
        servicePort = 0;
    }

    SocketServer::~SocketServer(){
        close_socket();
    }

    /*
    *   portNumber
    * 
    *   addressFamily:
    *       AF_INET, AF_UNIX
    * 
    *   communication style:
    *       SOCK_STREAM, SOCK_DGRAM, SOCK_RAW
    */
    int SocketServer::initialize(const char* port, int backlog)
    {
        int rv;                 /// return values
        int yes = 1;            /// for setsockopt
        struct addrinfo hints, *serverinfo, *p;

        servicePort = 0;

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
        *   current host. 
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
            std::cerr << "error: " << __func__ << ", getaddrinfo, " << gai_strerror(rv)
                           << std::endl;
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
            *   On success, a file descriptor for the new socket is returned.
            *   On error, -1 is returned, and errno is set appropriately.
            *
            */
            socketfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
            if(socketfd == -1){
                std::cerr << "error: " << __func__ << ", socketfd = " << socketfd 
                    << strerror(errno) << std::endl;
                continue;
            }


            /** SO_REUSEADDR: avoid address already in use error message. 
             *  Allows reuse of a port number that has not yet been cleared 
             *  by the kernel, without waiting for the kernel to clear it.
             * 
             *  If the server connection has been shut down and restarted
             *  using the same IP address and TCP port number, the bind call
             *  will fail if there were connections open when the server died.
             *  The connections will hopd the TCP port in TIME_WAIT state for
             *  30 - 120 seconds.
             * 
             *  The decision here is to get the server up and running again
             *  to avoid missing incoming connections, suffering the possibility
             *  of some packet loss.
             * 
             *  It is likely a server restart is an unusual situation, requiring
             *  recovery as quickly as possible.
             *
             */

            rv = setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
            if(rv != 0){
                std::cerr << "error: " << __func__ << ", setsockopt, " 
                        << strerror(errno) << std::endl;
                close_socket();
                continue;
            }


            // bind the socket to address
            rv = bind(socketfd, p->ai_addr, p->ai_addrlen);
            if(rv == 0){
                break;
            }
            else{
                std::cerr << "error: " << __func__ << ", bind " 
                    << strerror(errno) << std::endl;
                close_socket();
            }
        }       /// end for loop

        freeaddrinfo(serverinfo);          /// free address structure memory

        if(p == NULL)
        {
            std::cerr << "error: " << __func__ << ", initialize server failure\n";
            return -1;
        }

        /** int listen(int sockfd, int backlog);
        *   The backlog argument defines the maximum length to which the queue of pending
        *   connections for sockfd may grow.
        *
        *   On success, zero is returned. On error, -1 is returned, and errno is set appropriately.
        */
        if(listen(socketfd, backlog) != 0)
        {
            std::cerr << "error: " << __func__ << ", listen failed " 
                << strerror(errno) << std::endl;
            close_socket();
            return -1;
        }

        servicePort = atoi(port);
        return 0;
    }

    
    int SocketServer::accept_client_connection(client_info_t *theConnection){

        // initialize
        theConnection->fd = -1;
        memset((void*)&theConnection->address, 0, sizeof(theConnection->address));

        socklen_t addressLength = sizeof(theConnection->address);

        /** int accept (int socket, struct sockaddr *addr, socklen_t *length_ptr)
        *
        *   The accept function waits if there are no connections pending, unless
        *   the socket has nonblocking mode set.
        *
        *   The addr and length_ptr arguments are used to return information about
        *   the name of the client socket that initiated the connection.
        *
        *   Accepting a connection does not make socket part of the connection. Instead,
        *   it creates a new socket which becomes connected. The normal return value of
        *   accept is the file descriptor for the new socket.
        *
        *   After accept, the original socket remains open and unconnected,
        *   and continues listening until you close it. You can accept further connections
        *   with a socket by calling accept again.
        */
        theConnection->fd = accept(socketfd, (struct sockaddr *)&theConnection->address, &addressLength);
        if (theConnection->fd == -1) {
            perror("accept: ");
            return -1;
        }
        else return 0;
    }

    ssize_t SocketServer::receive_data(int connectedFD, void* buf, size_t len){
        ssize_t bytesRead;
        bytesRead = recv(connectedFD, buf, len, 0);
        return bytesRead;
    }

    ssize_t SocketServer::send_data(int connectedFD, void* buf, size_t len){

        ssize_t totalBytesSent = 0;
        ssize_t bytesSent;
        ssize_t bytesRemaining = len - totalBytesSent;

        while(bytesRemaining > 0){

            /* MSG_NOSIGNAL - don't generate a SIGPIPE signal if the peer
            *  on a stream-oriented socket has closed the connection.
            */
            bytesSent = send(connectedFD, buf, bytesRemaining, MSG_NOSIGNAL);

            if(bytesSent > 0){
                totalBytesSent += bytesSent;
            }
            else if(bytesSent == 0){
                //std::cerr << "warn: " << __func__ << ", bytesSent: " << bytesSent 
                //    << std::endl;
            }
            else{
                std::cerr << "warn: " << __func__ << ", bytesSent: " << bytesSent
                    << ", errno: " << strerror(errno) << std::endl;
                break;
            }

            bytesRemaining = len - totalBytesSent;
            log_trace("bytesSent %ld, bytesRemaining %ld", bytesSent, bytesRemaining);
        }
        return totalBytesSent;
    }

    void SocketServer::close_socket(){
        if(socketfd != -1){
            close(socketfd);
            socketfd = -1;
        }
    }

} // end namespace