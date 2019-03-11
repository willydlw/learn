#ifndef SOCKET_CLIENT_H
#define SOCKET_CLIENT_H

#include <cstdint>
#include <netinet/in.h>                 // struct sockaddr_in

namespace mysocket{
    class SocketClient{
        public: 

        // constants
        static constexpr int RECEIVE_BUFFER_LENGTH = 2048;

        SocketClient();
        SocketClient(const SocketClient&);
        ~SocketClient();

        // returns 0 upon success, -1 upon failure
        int connect_client(const char* port, const char* ipAddress);

        ssize_t receive_data(int connectedFD, void* buf, size_t len);
        ssize_t send_data(int connectedFD, void* buf, size_t len);

        int get_fd(){return socketfd;}
        bool get_connect_state(){return connected;}
        std::string get_connection_ip_address(){return connectionIPAdrress;}

        // disable copy semantics
        
        SocketClient& operator=(const SocketClient&) = delete;


        void close_socket();


        private:
            int socketfd;
            bool connected;
            std::string connectionIPAdrress;

    };
}


#endif