All:erun 

erun: main.o matrix.o utility.o 
	gcc -std=c11 -o erun main.o matrix.o utility.o  \
	-L -lm -lc
	


main.o:	main.c matrix.h utility.h
	gcc -std=c11 -Wall -Wextra -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align  \
	-Wconversion -pedantic -fpic -g -c main.c   \
	
matrix.o:	matrix.c matrix.h 
	gcc -std=c11 -Wall -Wextra -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align  \
	-Wconversion -pedantic -fpic -g -c matrix.c   \
	

utility.o: utility.c utility.h
	gcc -std=c11 -Wall -Wextra -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align  \
	-Wconversion -pedantic -fpic -g -c utility.c \
	

clean: 
	rm -f *.o			
	rm erun


	
	
	
	












