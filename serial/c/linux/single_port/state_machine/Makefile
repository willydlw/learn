All:erun 

erun: main.o serial.o communication_state.o
	g++ -std=c++0x -o erun main.o serial.o communication_state.o\
	


main.o:	main.c serial.h communication_state.h
	g++ -std=c++0x -Wall -Wextra -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align  \
	-Wconversion -pedantic -fpic -g -c main.c   \

	

serial.o: serial.c serial.h
	g++ -std=c++0x -Wall -Wextra -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align  \
	-Wconversion -pedantic -fpic -g -c serial.c \


communication_state.o: communication_state.c communication_state.h
	g++ -std=c++0x -Wall -Wextra -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align  \
	-Wconversion -pedantic -fpic -g -c communication_state.c \
	

clean: 
	rm -f *.o			
	rm erun


	
	
	
	












