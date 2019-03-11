/**
* @file echo_server.cpp
*
* @brief This program provides an example of a socket server
*        that accepts multiple client connections and 
*        echos any messages received back to the client.
*
*
*   Usage: ./eserver <port number>
*
*       where 
*           eserver is the executable file name
*           port number is the service port number
*
*   Communication method: TCP sockets
*       
*       
*   Signal handling is implemented for SIGINT and SIGTERM
*       ctrl+c produces the SIGINT
*
*       Either of these signals causes an exit from the main
*       while loop, so that any connections may be closed, and 
*       any end of program data is recorded before the program
*       terminates.       
*
*       signal handling references:
*           http://www.linuxprogrammingblog.com/all-about-linux-signals?page=show
*           http://man7.org/linux/man-pages/man2/select_tut.2.html
*
*
*   Order of operations:
*
*       initialize debug logging level       
*       verify minimum number of command line arguments 
*       initialize server socket
*       register the signal handlers
*       
*       while( no exit request from signal interrupt)
*
*           check for incoming client requests 
*           if connect request
*               connect client
*               add to connected client list
*
*           if read request
*               read message from client
*               echo message back to client
*
*           remove any disconnected clients from list
*      
*       close socket connection
*       
*
* @note Program contains a number of debug logging messages that are written 
*       to the standard error stream and an output file. This allows the user 
*       to watch the program flow to ensure messages are correctly received and
*       indicate all error conditions.
*
*
* @author Diane Williams
* @date 11 Jan 2019
*
*/

#define _POSIX_C_SOURCE 200112L          // pselect
#define  _DEFAULT_SOURCE                 // psignal 

#include <cerrno>
#include <cstdio>
#include <iostream>
#include <signal.h>
#include <string.h>
#include <sys/select.h>
#include <sys/time.h>
#include <vector>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <debuglog/debuglog.h>
#include <mysocket/socketServer.h>



constexpr int MAX_PENDING_CONNECTIONS = 3;
constexpr int BUFFER_SIZE = 256;


using namespace mysocket;


/*============== Global Variable Declarations =============================*/

static volatile sig_atomic_t exitRequest = 0;




/*========================= Function Definitions ==========================*/



/**
* @brief Sets exit request flag to 1 when SIGINT is received
*        or when SIGTERM is raised
*
* @param[in]    sig                       signal passed from operating system 
*
* @param[out]
*       sig_atomic_t    exitRequest      flag set to 1 when signal received
*                                         Defined as global
*
* @return void       
*
*      
* @note
*       Function is not directly invoked. Operating system calls it when
*       the registered signal is received. Signals are software interrupts.
*
*/
static void signal_handler_term(int sig)
{
    /* psignal used for debugging purposes
       debugging console output lets us know this function was
       triggered. Prints the string message and a string for the
       variable sig
    */
    psignal(sig, "signal_handler_term");
    if(sig == SIGINT || sig == SIGTERM){
        exitRequest = 1;
    }
    
}

void init_new_connection(client_info_t *cl)
{
    cl->data = nullptr;
    cl->numBytes = 0;
    cl->writeFlag = false;
}


void free_client_data(client_info_t *client)
{
    free(client->data);
    client->data = nullptr;
    client->numBytes = 0;
    client->writeFlag = false;
}



void build_client_list(std::vector<client_info_t> &clients)
{
    // create a copy of clients
    std::vector<client_info_t> temp(clients);
                             
    int length = int(temp.size());

    // Removes all elements from the vector (which are destroyed), 
    // leaving the container with a size of 0.
    clients.clear();  

    for(int i= 0; i < length; ++i){
        
        if(temp[i].fd != -1){
            log_info("add client with fd: %d to list", temp[i].fd);
            clients.push_back(temp[i]);
        }
        else // temp[i].fd == -1), disconnected client
        {
            log_trace("detected fd -1, this client will be removed, index: %d", i);

            // free data from disconnected clients
            if(clients[i].data != nullptr){
                log_trace("need to free client data");
                free_client_data(&temp[i]);
            }
        }
    }
}




bool transfer_buffer(client_info_t *cl, const void *buffer, size_t len)
{
    if(cl->data != nullptr){
        log_trace("freeing client data before transfer");
        free_client_data(cl);
    }

    cl->data = (uint8_t*)malloc(len*sizeof(uint8_t));
    if(cl->data == nullptr){
        log_error("memory allocation, %d bytes failed",len);
        return false;
    } 

    memcpy((void*)cl->data, buffer, len);
    cl->numBytes = len;
    return true;
}




int main(int argc, char **argv){

    /** Declarations **/
    SocketServer server;
    bool serverWrite;

    // client connections
    std::vector<struct client_info_t> clients; 
    std::vector<int> deletionList;
    int length;
    bool deleteClient;

    // file descriptor handling
    fd_set readfds;                     // read file descriptor set
    fd_set writefds;                    // write file descriptor set

    int maxfd;                          // largest file descriptor value             
            
    int selectReturn;                   // number read/write file descriptors pending
    struct timespec timeout;            // timeout for pselect

    ssize_t bytesRead;
    ssize_t bytesSent;
    uint8_t readBuffer[BUFFER_SIZE];

    // signal handling 
    sigset_t sigmask;
    sigset_t empty_mask;
    
    struct sigaction saterm;            // SIGTERM raised by this program or another
    struct sigaction saint;             // SIGINT caused by ctrl + c
    
    // miscellaneous
    int i;
    
    /* Initialize logging levels
    *  LOG_TRACE - show all messages at console level, 
    *  LOG_DEBUG - write DEBUG and higher level to a file
    *  1 - color on at console level
    */
    log_init(LOG_INFO, LOG_OFF, 1);

    // Verify the minimum number of arguments were passed to main
    if(argc < 2){
        log_fatal("usage: %s port", argv[0]);
        return 1;
    }

    if(server.initialize(argv[1], MAX_PENDING_CONNECTIONS) != 0){
        log_fatal("server initialize failure");
        return 1;
    }

    // pselect does not change the timeout argument, so we only need to
    // initialize it here. If the code is changed to use select instead
    // of pselect, then this initialization should be moved inside the
    // while loop, as select updates the timeout argument, deducting
    // elapsed time from it. 
    // reference: http://man7.org/linux/man-pages/man2/select.2.html 
    timeout.tv_sec = 5;                     // seconds
    timeout.tv_nsec = 0;                    // nanoseconds   


    // register the SIGTERM signal handler function
    memset(&saterm, 0, sizeof(saterm));
    saterm.sa_handler = signal_handler_term;

    /*  The sigaction() system call is used to change the action 
        taken by a process on receipt of a specific signal.
    */
    if(sigaction(SIGTERM, &saterm, NULL) < 0){
        log_fatal("sigaction saterm, errno: %s", strerror(errno));
        return 1;
    }
    

    // register the SIGINT signal handler function
    memset(&saint, 0, sizeof(saint));
    saint.sa_handler = signal_handler_term;
    if(sigaction(SIGINT, &saint, NULL) < 0){
        log_fatal("sigaction saint, errno: %s", strerror(errno));
        return 1;
    }

    // signal mask initialization
    sigemptyset(&sigmask);
    sigemptyset(&empty_mask);

    sigaddset(&sigmask, SIGTERM);
    sigaddset(&sigmask, SIGINT);

    // set as blocking so that pselect can receive event
    if(sigprocmask(SIG_BLOCK, &sigmask, NULL) < 0){
        log_fatal("sigprocmask, errno: %s", strerror(errno));
        return 1;
    }


    // SIGINT (ctrl + c) or SIGTERM causes loop exit
    while(exitRequest == 0){

        // clear file descriptor sets
        FD_ZERO(&readfds);
        FD_ZERO(&writefds); 

        serverWrite = false;          
        
        // add the server socket to the read set
        FD_SET(server.get_fd(), &readfds);
        //FD_SET(server.get_fd(), &writefds);
        maxfd = server.get_fd();

        // add connected client sockets to set
        length = int(clients.size());
        log_trace("number of clients: %d", length);
        for(i = 0; i < length; ++i){
            log_trace("adding client %d, fd: %d to readfds",i, clients[i].fd);
            // add all clients to read set 
            FD_SET(clients[i].fd, &readfds);
            FD_SET(clients[i].fd, &writefds);

            // select function requires largest file descriptor number
            if(clients[i].fd > maxfd){
                maxfd = clients[i].fd;
            }
        } 


        // pselect requires an argument that is 1 more than
        // the largest file descriptor value
        selectReturn = pselect(maxfd+1, &readfds, &writefds, NULL, &timeout, &empty_mask); 

        log_trace("back from pselect, selectReturn: %d", selectReturn);
        
        // signal may have occurred during pselect call
        if(exitRequest == 1){
            log_info("received exit request");
            break;
        } 

        if(selectReturn < 0){
            log_fatal("select error, selectReturn: %d", selectReturn);
            break;
        }
        else if(selectReturn == 0){
            // nothing to read or write
            log_debug("nothing to read or write, selectReturn: %d", selectReturn);
            continue;
        }

        /* A read request on the server socket file descriptor must 
        *  be an incoming connection request
        */
        if(FD_ISSET(server.get_fd(), &readfds)){

            /* Note that it is possible this client has previously connected
            *  and is already in the connectedfd list here. Disconnected
            *  clients are removed from the list as they are discovered, later
            *  in this loop.
            */
            client_info_t newClient;
            // newClient fd and address are set by accept function
            if( server.accept_client_connection(&newClient) != -1){
                log_info("new connection, fd: %d, ip: %s, port: %d", newClient.fd,
                            inet_ntoa(newClient.address.sin_addr), 
                            ntohs(newClient.address.sin_port));

                init_new_connection(&newClient);
                clients.push_back(newClient);
            }
            else{
                log_warn("connection request failed");
            }
        }

        // read messages from connected clients
        // not updating the length as the new connection add above
        // will not an entry in readfds 
        for(i = 0; i < length; ++i){
            if(FD_ISSET(clients[i].fd, &readfds)){

                memset(readBuffer, 0, BUFFER_SIZE);
                
                bytesRead = server.receive_data(clients[i].fd, readBuffer, BUFFER_SIZE);
                
                log_info("bytesRead: %d", bytesRead);
                log_info("client fd: %d", clients[i].fd);

                if(bytesRead > 0){
                    // this is an echo server that sends data back to client
                    // serverWrite flag set to true so that the data may be sent
                    // later
                    serverWrite = true;

                    // transfer data to client buffer for further processing
                    if(transfer_buffer(&clients[i], readBuffer, bytesRead)){
                        log_trace("buffer transfer completed");
                        // set write flag so that the data will be echoed to client
                        clients[i].writeFlag = true;
                    }
                    else{
                        log_warn("client fd: %d, received data lost due to memory"
                            " allocation error", clients[i].fd);
                    }
                }
                else if(bytesRead == 0){ // disconnected
                    log_info("Disconnected from ip %s, port %d",  
                          inet_ntoa(clients[i].address.sin_addr) , 
                          ntohs(clients[i].address.sin_port));
                    
                    deleteClient = true;

                    // free any data 
                    if(clients[i].data != nullptr){
                        log_warn("freeing data buffer");
                        free(clients[i].data);
                        clients[i].data = nullptr;
                        clients[i].numBytes = 0;
                    }   
                         
                    //Close the socket and mark as 0 in list for reuse  
                    close( clients[i].fd );
                    clients[i].fd = -1; 
                }
                else{
                    log_info("server did not echo bytes received");
                }
            }
        }

        // server echos data to client
        if(serverWrite)
        {
            log_trace("serverWrite is true");
            for(i = 0; i < length; ++i){

                if(FD_ISSET(clients[i].fd, &writefds)){
                    log_trace("client[%d].fd: %d is writeable", i, clients[i].fd);

                    // echo back if there is data
                    if(clients[i].writeFlag){

                        log_trace("client fd: %d, writeFlag set", clients[i].fd);
                        bytesSent = server.send_data(clients[i].fd, clients[i].data, clients[i].numBytes);
                        if(bytesSent > 0){
                            if(bytesSent != clients[i].numBytes){
                                log_warn("bytesSent: %d != numBytes: %d", bytesSent, clients[i].numBytes);
                                log_warn("freeing client data, even though all bytes were not sent");
                            }
                            else{
                                log_trace("all bytes sent to client")
                            }

                            free_client_data(&clients[i]);
                        }
                        else{
                            log_trace("no bytes sent to client[%d].fd: %d", i, clients[i].fd);
                            log_trace("will try again next time through loop");
                        }
                    }
                }
            } // end for
        } // end if serverwrite
        
        // rebuild client list to handle disconnects
        if(deleteClient){
            build_client_list(clients);
            deleteClient = false;
        }
            
    } // end while

    log_trace("exited while loop");

    // close connected client socket and free memory
     length = int(clients.size());
     for(i = 0; i < length; ++i){
         if(clients[i].data != nullptr){
            free_client_data(&clients[i]);
         }
         if(clients[i].fd != -1){
             close(clients[i].fd);
         }
     }

    /* Note the socket server class destructor takes care 
    *  of closing the socket. Thus, a call to close_socket 
    *  is not technically required.
    */
    server.close_socket();


    return 0;
}
