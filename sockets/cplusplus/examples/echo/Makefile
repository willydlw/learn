All:eserver clbytearray sendbyte

# create executables
eserver: eserver.o 
	g++ -o eserver eserver.o  \
	-L /usr/local/lib/ -ldebuglog -lmysocket -lm -lc 
	
clbytearray: clbytearray.o 
	g++ -o clbytearray clbytearray.o  \
	-L /usr/local/lib/ -ldebuglog -lmysocket -lm -lc

clsendbyte: sendbyte.o 
	g++ -o sendbyte sendbyte.o  \
	-L /usr/local/lib/ -ldebuglog -lmysocket -lm -lc


# create object files
eserver.o:	echoServer.cpp 
	g++ -Wall -Wextra -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align  \
	-Wconversion -pedantic -g -o eserver.o -c echoServer.cpp   \
	-I /usr/local/include/


clbytearray.o:	client_send_byte_array.cpp 
	g++ -Wall -Wextra -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align  \
	-Wconversion -pedantic -g -o clbytearray.o -c client_send_byte_array.cpp   \
	-I /usr/local/include/


clsendbyte.o: client_send_byte.cpp
	g++ -Wall -Wextra -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align  \
	-Wconversion -pedantic -g -o clsendbyte.o -c client_send_byte.cpp   \
	-I /usr/local/include/


# clean rule is marked as phony because its target is not an actual file
# that will be generated
.PHONY: clean 
clean: 
	rm -f *.o 		
	rm eserver clbytearray clsendbyte
