#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "common.h"

extern int proj_id;


int main(){

    key_t key;
    int msgid;
    int keeprunning = 1;
    size_t len;

    struct message_buffer_t themsg = { 1, "\0" };

    /* ftok uses file name and 8 bit int value to generate
       a unique key value.
       Note: file path name must exist and be accessible
    */
    key = ftok(commonFileName, proj_id);

    if(key == -1){
        perror("key value error");
        return 1;
    }

    /* create message queue
    *  0 for octal, 666 gives read/write privileges to owner, group, other
    *  IPC_CREAT flag will create message queue if it does not already exist
    */
    msgid = msgget(key, 0666 | IPC_CREAT);
    if(msgid == -1){
        perror("msgget error ");
        return 1;
    }


    
    while(keeprunning){
        printf("type message: ");
        if( fgets(themsg.mtext, sizeof(themsg.mtext), stdin) == NULL){
            break;
        }

        // fgets appends line feed
        if(strcmp(themsg.mtext, "q\n") == 0){
            keeprunning = 0;
        }

        /* msgsnd appends a copy of the message to the message queue
        *
        *  If sufficient space is available in the queue, msgsnd succeeds
        *  immediately. Default behavior of msgsnd is to block until space
        *  becomes available in the queue.
        *
        */
        len = strlen(themsg.mtext); 
        if( msgsnd(msgid, &themsg, len+1, 0) != 0){
            perror("msgsnd error ");
        }

        printf("%s sent %s\n", __FILE__, themsg.mtext);
    }


    return 0;
}