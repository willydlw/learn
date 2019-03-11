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

#include "socketClient.h"

namespace mysocket{

    SocketClient::SocketClient(){
        socketfd = -1;
        connected = false;
    }

    SocketClient::SocketClient(const SocketClient& sc){
        socketfd = sc.socketfd;
        connected = sc.connected;
        connectionIPAdrress = sc.connectionIPAdrress;
    }

    SocketClient::~SocketClient(){
        close_socket();
    }

    int SocketClient::connect_client(const char* port, const char* ipAddress)
    {
        struct addrinfo hints, *servinfo, *ptr;

        memset(&hints, 0, sizeof hints);    /// zero out all struct data

        /** AF_UNSPEC is a wildcard that represents Address Family (AF) domain
        *   allows us to connect to IPv4 or IPv6 internet domain */
        hints.ai_family = AF_UNSPEC;

        // SOCK_STREAM provides sequenced, reliable, two-way, connection-based byte streams.
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
        */
        int rv = getaddrinfo(ipAddress, port, &hints, &servinfo);
        if (rv != 0) {
            std::cerr << "error: " << __func__ << ", getaddrinfo: " 
                    << gai_strerror(rv) << std::endl;
            return -1;
        }


        /** iterate through the linked list of results. 
         *  Connect to the first one we can */
        for(ptr = servinfo; ptr != NULL; ptr = ptr->ai_next)
        {
            socketfd = socket(ptr->ai_family, ptr->ai_socktype,ptr->ai_protocol);
            if(socketfd == -1){
                std::cerr << "error: " << __func__ << ", socketfd = " << socketfd 
                    << strerror(errno) << std::endl;
                continue;
            }

            // request connection to server
            rv = connect(socketfd, ptr->ai_addr, ptr->ai_addrlen);

            if( rv != -1){ // successful connection, break out of loop
                break;
            }
            else{
                std::cerr << "error: " << __func__ << ", connect " 
                    << strerror(errno) << std::endl;
                close_socket();
            }
        } // end loop

        if (ptr == NULL)
        {
            std::cerr << "error: " << __func__ << ", failed to connect\n";
            freeaddrinfo(servinfo);     
            return -1;
        }

        freeaddrinfo(servinfo);                 // free address structure memory
    
        connected = true;
        return 0;
    }


    ssize_t SocketClient::receive_data(int connectedFD, void* buf, size_t len){
        ssize_t bytesRead;
        bytesRead = recv(connectedFD, buf, len, 0);
        return bytesRead;
    }

    ssize_t SocketClient::send_data(int connectedFD, void* buf, size_t len){

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
                std::cerr << "warn: " << __func__ << ", bytesSent: " << bytesSent 
                    << std::endl;
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

    void SocketClient::close_socket(){
        if(socketfd != -1){
            close(socketfd);
            socketfd = -1;
            connected = false;
        }
    }

} // end namespace