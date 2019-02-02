CFLAGS= -g -Wall -fstack-protector
LDFLAGS= -g

all: writer reader

writer: writer.o
	gcc $(LDFLAGS) -o writer writer.o 

reader: reader.o
	gcc $(LDFLAGS) -o reader reader.o

writer.o: writer.c common.h 
	gcc $(CFLAGS) -c writer.c 

reader.o: reader.c common.h
	gcc $(CFLAGS) -c reader.c 


clean:
	rm -f *.o writer reader