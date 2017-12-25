All:erun 

erun: main.o serial.o
	g++ -std=c++0x -o erun main.o serial.o \
	


main.o:	main.c serial.h
	g++ -std=c++0x -Wall -Wextra -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align  \
	-Wconversion -pedantic -fpic -g -c main.c   \

	
	

serial.o: serial.c serial.h
	g++ -std=c++0x -Wall -Wextra -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align  \
	-Wconversion -pedantic -fpic -g -c serial.c \
	

clean: 
	rm -f *.o			
	rm erun


	
	
	
	












