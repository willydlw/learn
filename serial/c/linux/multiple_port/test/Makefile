All:etest

etest: test_import_sensor_data.o sensor.o
	gcc -std=c11 -o etest test_import_sensor_data.o sensor.o \
	-L /usr/local/lib/debuglog.so -ldebuglog -lm
	


test_import_sensor_data.o:	test_import_sensor_data.c ../sensor.h
	gcc -std=c11 -Wall -Wextra -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align  \
	-Wconversion -pedantic -fpic -g -c test_import_sensor_data.c   \
	-I /usr/local/include/debuglog/ 
	

sensor.o:	../sensor.c ../sensor.h
	gcc -std=c11 -Wall -Wextra -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align  \
	-Wconversion -pedantic -fpic -g -c ../sensor.c   \
	-I /usr/local/include/debuglog/ 

clean: 
	rm -f *.o			
	rm etest


	
	
	
	












