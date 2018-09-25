
all: eserver eclient

eserver: server_test.o 
	gcc -std=c99 -o eserver server_test.o  \
	-L /usr/local/lib/libclientserver.so -lclientserver 

eclient: client_test.o 
	gcc -std=c99 -o eclient client_test.o  \
	-L /usr/local/lib/libclientserver.so -lclientserver 

server_test.o: server_test.c 
	gcc -std=c99 -Wall -Wextra -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align  \
	-Wconversion -pedantic -fpic -g -c server_test.c   \
	-I /usr/local/include/clientserver/ 

client_test.o: client_test.c 
	gcc -std=c99 -Wall -Wextra -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align  \
	-Wconversion -pedantic -fpic -g -c client_test.c   \
	-I /usr/local/include/clientserver/ 

clean:
	rm -f *.o 
	rm eserver eclient
