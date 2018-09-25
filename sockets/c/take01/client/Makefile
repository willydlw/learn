all: eclient
	
eclient: client.o
	gcc -o eclient -Wall -Wextra main.c client.c

clean:
	rm -f *.o eclient