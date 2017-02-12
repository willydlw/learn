All: cserial.o 
	g++ -shared -Wl,-soname,libcserial.so.1 -o libcserial.so.1.0 cserial.o -lc


cserial.o: cserial.c cserial.h
	g++ -Wall -Wextra -Wshadow -Wpointer-arith \
	-Wcast-qual -Wcast-align  \
	-Wconversion \
	-pedantic -fpic -g -c cserial.c

install:
	mv *.so.1.0 /usr/local/lib
	ln -sf /usr/local/lib/libcserial.so.1.0 /usr/local/lib/libcserial.so
	ln -sf /usr/local/lib/libcserial.so.1.0 /usr/local/lib/libcserial.so.1
	cp cserial.h /usr/local/include/cserial
	

uninstall:
	rm /usr/local/lib/libcserial.so.1.0
	rm /usr/local/lib/libcserial.so.1
	rm /usr/local/include/cserial/cserial.h

clean:
	rm -f *.o *.so.1.0










