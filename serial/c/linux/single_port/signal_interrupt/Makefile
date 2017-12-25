All:erun 

erun: main.o command_line.o
	g++ -std=c++0x -o erun main.o command_line.o \
	-L /usr/local/lib/libcserial.so -lcserial -lm
	


main.o:	main.c command_line.h
	g++ -std=c++0x -Wall -Wextra -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align  \
	-Wconversion -pedantic -fpic -g -c main.c   \
	-I /usr/local/include/cserial/ \
	
	

command_line.o: command_line.c command_line.h
	g++ -std=c++0x -Wall -Wextra -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align  \
	-Wconversion -pedantic -fpic -g -c command_line.c \
	

clean: 
	rm -f *.o			
	rm erun


	
	
	
	












