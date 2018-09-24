#include <unistd.h>
#include <stdio.h>					// fprintf
#include <stdlib.h>					// EXIT_FAILURE
#include <sys/types.h>				// accept 
#include <sys/socket.h>				// socket 
#include <sys/un.h>					// sockaddr_un 



/* Unix communication within the same machine, using pathnames as addresses */
const char *SOCKET_PATH = "/tmp/srv_socket";


const int QUEUE_LENGTH = 3;

const int BUFFER_LENGTH = 64;



int main(void){

	struct sockaddr_un  socket_address;
	int socket_fd;
	int accept_fd;
	char buffer[BUFFER_LENGTH];

	ssize_t bytes_read;


	/* create an endpoint for communication and return file descriptor
	   that refers to that endpoint
	  AF_UNIX used for local communication 

	  SOCK_STREAM provides sequenced, reliable, two-way, connection-based
	  byte streams.

	  0 specifies use the single protocol specified for this family
	*/

    if ((socket_fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) 
    { 
        perror("socket error"); 
        exit(EXIT_FAILURE); 
    } 

    // initialize all fields to 0
    memset(&socket_address, 0, sizeof(struct sockaddr_un));
    socket_address.sun_family = AF_UNIX;

    // leave room for null terminator by subtracting 1 from sizeof
    strncpy(socket_address.sun_path, SOCKET_PATH, sizeof(socket_address.sun_path) - 1);

    /* unlink deletes name from the file system. If that name was the last link to a file and 
       no processes have the file open the file is deleted and the space it was using is made 
       available for reuse.

       If the name referred to a socket, fifo or device the name for it is removed but processes 
       which have the object open may continue to use it.
    */
    if( unlink(SOCKET_PATH) == -1){
    	perror("unlink error");
    	exit(EXIT_FAILURE);
    }


    /* Binding Unix domain sockets
    	File system entry is created at bind time. If the file already exists, bind will fail.
    	User must remove stale sockets. The unlink call above is intended to remove stale socket

    	Write permission is needed for the corresponding file
    */

    if(bind(socket_fd, (struct sockaddr *) &socket_address, sizeof(struct sockaddr_un)) == -1){
    	perror("bind error");
    	exit(EXIT_FAILURE);
    }

    /* listen - willingness to accept incoming connections and a queue limit for incoming connections

    	queue length for completely established sockets waiting to be accepted
    */

    if( listen(socket_fd, QUEUE_LENGTH) == -1){
    	perror("listen error");
    	exit(EXIT_FAILURE);
    }

    while(1){
    	/* 	extracts the first connection request on the queue of pending connections for the 
    		listening socket, sockfd, creates a new connected socket, and returns a new file
    		descriptor referring to that socket.  The newly created socket is not in the listening state.  
    		The original socket sockfd is unaffected by this call.

    		If no pending connections are present on the queue, and the socket is not marked as nonblocking, 
    		accept() blocks the caller until a connection is present.
    	*/
    	if( (accept_fd = accept(socket_fd, NULL, NULL)) == -1 ){
    		perror("accept error");
    		continue;
    	}

    	while( (bytes_read = read(accept_fd, buffer, sizeof(buffer))) > 0){
    		fprintf(stderr, "read %ld bytes: %.*s\n", bytes_read, (int)bytes_read, buffer);
    	}

    	if( bytes_read == -1){
    		perror("read error");
    		exit(EXIT_FAILURE);
    	}
    	else if (bytes_read == 0){
    		fprintf(stderr, "EOF\n");
    		close(accept_fd);
    	}

    }


	return 0;
}