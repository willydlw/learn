All:etest

etest: test_import_sensor_data.o sensor_device.o
	gcc -std=c11 -o etest test_import_sensor_data.o sensor_device.o \
	-L /usr/local/lib/debuglog.so -ldebuglog -lm
	


test_import_sensor_data.o:	test_import_sensor_data.c ../sensor_device.h
	gcc -std=c11 -Wall -Wextra -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align  \
	-Wconversion -pedantic -fpic -g -c test_import_sensor_data.c   \
	-I /usr/local/include/debuglog/ 
	

sensor_device.o:	../sensor_device.c ../sensor_device.h
	gcc -std=c11 -Wall -Wextra -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align  \
	-Wconversion -pedantic -fpic -g -c ../sensor_device.c   \
	-I /usr/local/include/debuglog/ 

clean: 
	rm -f *.o			
	rm etest


	
	
	
	












