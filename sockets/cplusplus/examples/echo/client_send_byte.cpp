/* Description: 
*   client object connects to socket server
*   
*   while loop condition true
*       client sends one byte to server
*       client receives byte from server
*
*  Command line arguments:
*   argv[1]   ip address
*   argv[2]  port number
*
*/
#include <cstdio>
#include <cstdlib>              // atoi
#include <cstring>
#include <chrono>
#include <thread>

#include <mysocket/socketClient.h>

constexpr int BUFFER_SIZE = 16;



const char* defaultIPAddress = "localhost";
const char* defaultPort = "8080";



int main(int argc, char **argv){

    uint8_t count = 0;
    ssize_t bytesWritten, bytesReceived;
    uint8_t readByte;

   
    // ensure minimum number command line arguments
    if(argc < 3){
        fprintf(stderr, "usage: %s <ip address> <port number> ", argv[0]);
        return 1;
    }
    
    // populate ip address and port number
    char *serverIp = argv[1];
    char *port = argv[2];
    
    // create client object and connect to server
    SocketClient client;
    if( client.connect_client(port, serverIp) < 0){
        fprintf(stderr, "fatal error: client failed to connect to server\n");
        return 1;
    }


    // loop a few times to demonstrate byte sent from client is echoed by server
    while(count < 5){
        
        // send the count value to the server
        fprintf(stderr, "client sending: %d", count);
        
        bytesWritten = client.send_data(client.get_fd(), &count, 1);
        if(bytesWritten < 1){
            fprintf(stderr,"error, bytesWritten: %ld < 1\n", bytesWritten);
        }

        // expect the server to echo back the byte just sent
        bytesReceived = client.receive_data(client.get_fd(), &readByte, 1);
        
        if(bytesReceived == 1){
            fprintf(stderr, "client received: %d\n", int(readByte));
        }
        else{
            fprintf(stderr, "error, bytesReceived: %ld\n", bytesReceived);
            break;
        }

        fprintf(stderr,"\n");

        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        ++count;
    }
    return 0;
}