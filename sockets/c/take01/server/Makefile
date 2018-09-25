all: eserver
	
eserver: server.o
	gcc -o eserver -Wall -Wextra main.c server.c

clean:
	rm -f *.o eserver