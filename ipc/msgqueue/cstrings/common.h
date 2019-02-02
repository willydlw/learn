#ifndef COMMON_INCLUDED_H
#define COMMON_INCLUDED_H

#define BUFFER_SIZE 128

typedef struct message_buffer_t{
    long mtype;
    char mtext[BUFFER_SIZE];
}message_buffer;

char *commonFileName = "writer.c";

int proj_id = 65;


#endif