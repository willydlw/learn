all: runServer runClient
	
runServer: server.o
	gcc -o runServer -Wall -Wextra server.c
runClient: client.o
	gcc -o runClient -Wall -Wextra client.c

clean:
	rm -f *.o runServer runClient