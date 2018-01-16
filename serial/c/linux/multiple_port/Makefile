All:erun 

erun: main.o operations.o serial.o communication_state.o sensor.o
	gcc -std=c11 -o erun main.o operations.o serial.o  \
	communication_state.o sensor.o  \
	-L /usr/local/lib/debuglog.so -ldebuglog -lm
	


main.o:	main.c operations.h sensor.h
	gcc -std=c11 -Wall -Wextra -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align  \
	-Wconversion -pedantic -fpic -g -c main.c   \
	-I /usr/local/include/debuglog/ 

operations.o:	operations.c operations.h communication_state.h
	gcc -std=c11 -Wall -Wextra -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align  \
	-Wconversion -pedantic -fpic -g -c operations.c   \
	-I /usr/local/include/debuglog/ 
	

serial.o: serial.c serial.h
	gcc -std=c11 -Wall -Wextra -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align  \
	-Wconversion -pedantic -fpic -g -c serial.c \
	-I /usr/local/include/debuglog/ 
	#g++ -std=c++0x


communication_state.o: communication_state.c communication_state.h serial.h
	gcc -std=c11 -Wall -Wextra -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align  \
	-Wconversion -pedantic -fpic -g -c communication_state.c \
	-I /usr/local/include/debuglog/ 


sensor.o: sensor.c sensor.h
	gcc -std=c11 -Wall -Wextra -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align  \
	-Wconversion -pedantic -fpic -g -c sensor.c \
	-I /usr/local/include/debuglog/ 
	

clean: 
	rm -f *.o			
	rm erun


	
	
	
	












