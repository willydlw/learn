All:eclient

# create executables
eclient: clbytearray.o
	g++ -o eclient clbytearray.o  \
	-L /usr/local/lib/ -ldebuglog -lmysocket -lm -lc


# create object files
clbytearray.o:	client_send_byte_array.cpp 
	g++ -Wall -Wextra -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align  \
	-Wconversion -pedantic -g -o clbytearray.o -c client_send_byte_array.cpp   \
	-I /usr/local/include/



# clean rule is marked as phony because its target is not an actual file
# that will be generated
.PHONY: clean 
clean: 
	rm -f *.o 		
	rm eclient
