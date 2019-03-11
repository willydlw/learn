/* Purpose: 
*   Send/receive fixed length byte arrays
*   Verify data received matches data sent
*
*  Command line arguments:
*   argv[1]  ip address
*   argv[2]  port number
*   argv[3]  array size
*   argv[4]  loop iterations
*                           
*
Description: 
*
*   verify correct number command line arguments
*   assign command line arguments to variables
*
*   client object connects to socket server
*   
*   while loop condition true
*       client fills byte array with random values
*       client sends array to server
*       client receives bytes from server
*
*       compare bytes received to sent
*           log any mismatch errors
*
*
*/
#include <cstdlib>              // atoi, rand
#include <ctime>                // time
#include <iostream>
#include <chrono>
#include <thread>

#include <debuglog/debuglog.h>

#include <mysocket/socketClient.h>

constexpr int BUFFER_SIZE = 16;

using namespace mysocket;


void fillArray(uint8_t *array, int len)
{
    for(int i = 0; i < len; i++){
        array[i] = uint8_t(rand()%256);
    }
}

int compareBytes(const uint8_t *a, const uint8_t *b, int len)
{
    int mismatchCount = 0;
    for(int j = 0; j < len; ++j){
        if(a[j] != b[j]){
            ++mismatchCount;
        }
    }
    return mismatchCount;
}


int main(int argc, char **argv){

    int loopCount = 0;
    int currentIndex;

    ssize_t bytesWritten, bytesReceived;
    ssize_t bytesRemaining;

    int bytesWrittenError = 0;
    int mismatchByteCount = 0;

    log_init(LOG_DEBUG, LOG_OFF, 1);

    // verify min required command line arguments
    if(argc < 5){
        log_error("usage: %s <ip address> <port number> <array size> <loop iterations> ", argv[0]);
        return 1;
    }

    char *serverIp = argv[1];
    char *port = argv[2];
    int arraySize = atoi(argv[3]);
    int loopIterations = atoi((argv[4]));

    // create arrays now that array size is known
    // C++ does not allow variable length arrays, must dynamically allocate
    // relying on C++ to throw an exception if it cannot allocate
    uint8_t *writeBuffer = new uint8_t[arraySize];
    uint8_t *readBuffer = new uint8_t[arraySize];
    uint8_t *dataReceived = new uint8_t[arraySize];

    // generate different random values each time program runs
    srand(unsigned(time(NULL)));
    
    // create client object and connect to server
    SocketClient client;
    if( client.connect_client(port, serverIp) < 0){
        log_fatal("client failed to connect to server");
        return 1;
    }

    log_info("arraySize: %d", arraySize);
    
   
    while(loopCount < loopIterations){
        
        // update user on progress
        log_trace("loopCount: %d", loopCount);
    
        // fill array with random byte values
        fillArray(writeBuffer, arraySize);
        
        // transmit to server
        bytesWritten = client.send_data(client.get_fd(), writeBuffer, arraySize);
        
        // verify all bytes were sent
        if(bytesWritten < 0){
            log_error("bytesWritten: %d", bytesWritten);
            break;
        }
        
        if(bytesWritten < arraySize){
            log_error("bytesWritten: %d < %d, loop iteration: %d", bytesWritten, 
                        arraySize, loopCount);
            ++bytesWrittenError;
        }

        bytesRemaining = bytesWritten;
        currentIndex = 0;

        while(bytesRemaining > 0){
            // expect the server to echo back the bytes sent
            bytesReceived = client.receive_data(client.get_fd(), readBuffer, bytesRemaining);
            if(bytesReceived > 0){
                bytesRemaining -= bytesReceived;

                // copy data from temporary readBuffer to dataReceived for later comparison
                for(int j = 0; j < bytesReceived && currentIndex < arraySize; ++j, ++currentIndex){
                    dataReceived[currentIndex] = readBuffer[j];
                }
            }

            log_trace("bytesReceived: %d, bytesRemaining: %d", bytesReceived, bytesRemaining);
        }

        
        // casting to int from ssize_t (aka long int) assumes bytes received
        // and written will not exceed the max int value
        mismatchByteCount += compareBytes(writeBuffer, dataReceived, int(bytesWritten));
        
        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        ++loopCount;
    }

    // print summary statistics
    std::cout << "\n\n*** Summary Stats ***\n";
    std::cout << "bytes Written Errors:  " << bytesWrittenError << std::endl;
    std::cout << "mismatch byte count:   " << mismatchByteCount << std::endl;
    
    // free memory
    delete[] readBuffer;
    delete[] writeBuffer;
    delete[] dataReceived;

    return 0;
}