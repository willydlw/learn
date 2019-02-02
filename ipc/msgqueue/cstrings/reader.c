#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "common.h"

extern int proj_id;

int main(){
    key_t key;
    int msgid;

    int keepRunning = 1;

    struct message_buffer_t readermsg;

    key = ftok(commonFileName, proj_id);

    if(key == -1){
        perror("key value error");
        return 1;
    }


    msgid = msgget(key, 0666 | IPC_CREAT);
    if(msgid == -1){
        perror("msgget error ");
        return 1;
    }

    while(keepRunning){
        /* msgrcv removes message from queue and places it in
        *  buffer readermsg.
        *  
        *  1 says read message from queue that has mtype 1
        */
        msgrcv(msgid, &readermsg, sizeof(readermsg), 0, 0);
        
        printf("reader received: %s\n", readermsg.mtext);
        
        if(strcmp(readermsg.mtext, "q\n") == 0){
            keepRunning = 0;
        }
    }

    // destroy message queue
    msgctl(msgid, IPC_RMID, NULL);


    return 0;
}