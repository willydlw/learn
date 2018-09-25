#include <stdio.h>      /// fprintf
#include <stdlib.h>     /// exit
#include <string.h>     /// memset
#include <unistd.h>

#include <clientserver/clientserver.h>


int main(int argc, char *argv[])
{

    char buffer[256];

    int client_socketfd;
    ssize_t bytes_read;


    if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }

    client_socketfd = client_connect_socket(argv[1], argv[2]);


    /// send message to server
    fprintf(stdout, "enter a message to send to the server: ");
    memset(buffer, 0, 256);
    fgets(buffer, 255,stdin);               /// read message from keyboard

    fprintf(stderr, "main, buffer: %s\n", buffer);

    if(send_data(client_socketfd, buffer, strlen(buffer)) == -1)
    {
        fprintf(stderr, "error, send_data returned -1, socket closed\n");
    }

    memset(buffer, 0, 256);

    /// read server response

    bytes_read = recv(client_socketfd, buffer, 255, 0);
    if(  bytes_read < 0)
    {
        perror("recv ");
        fprintf(stderr, "bytes_read: %ld\n", bytes_read);
    }
    else if (bytes_read > 0)
    {
        printf("%s\n",buffer);
    }


    close(client_socketfd);

    return 0;

}
