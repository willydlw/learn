All:etest 

etest: interrupt_byte_test.c
	g++ -g -Wall -Wextra -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align \
	-Wconversion interrupt_byte_test.c -o etest \
	-I /usr/local/include/cserial/ \
	-L /usr/local/lib/libcserial.so -lcserial 
	

clean: 
	rm etest










