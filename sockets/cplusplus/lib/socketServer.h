#ifndef SOCKET_SERVER_H
#define SOCKET_SERVER_H

#include <cstdint>
#include <netinet/in.h>                 // struct sockaddr_in

namespace mysocket{

    struct client_info_t{
        int fd;
        struct sockaddr_in address;
        uint8_t *data;
        ssize_t numBytes;
        bool writeFlag;
    };


    class SocketServer{
        public: 

        // constants
        static constexpr int BACKLOG_QUEUE_SIZE = 5;
        static constexpr int RECEIVE_BUFFER_LENGTH = 2048;

        // constructor
        SocketServer();

        // destructor
        ~SocketServer();

        int initialize(const char* port, int maxpending);

        int accept_client_connection(client_info_t *theConnection);

        ssize_t receive_data(int connectedFD, void* buf, size_t len);
        ssize_t send_data(int connectedFD, void* buf, size_t len);

        int get_fd(){return socketfd;}

        // disable copy semantics
        SocketServer(const SocketServer&) = delete;
        SocketServer& operator=(const SocketServer&) = delete;


        void close_socket();


        private:
            int socketfd;
            int servicePort;

            int bind_server();
    };
}


#endif