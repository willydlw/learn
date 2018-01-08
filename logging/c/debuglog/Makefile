All: debuglog.o 
	gcc -shared -Wl,-soname,libdebuglog.so.1 -o libdebuglog.so.1.0 debuglog.o \
	-lc

# command line option ‘-Wtraditional’ is valid for C/ObjC 
#			but not for C++ [enabled by default]
# command line option ‘-Wstrict-prototypes’ is valid for Ada/C/ObjC 
#			but not for C++ [enabled by default]
# command line option ‘-Wmissing-prototypes’ is valid for Ada/C/ObjC 
#			but not for C++ [enabled by default]


debuglog.o: debuglog.c debuglog.h 
	gcc -std=c11 -Wall -Wextra -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align  \
	-Wconversion -pedantic -fpic -g -c debuglog.c


install:
	mv *.so.1.0 /usr/local/lib
	ln -sf /usr/local/lib/libdebuglog.so.1.0 /usr/local/lib/libdebuglog.so
	ln -sf /usr/local/lib/libdebuglog.so.1.0 /usr/local/lib/libdebuglog.so.1
	cp debuglog.h /usr/local/include/debuglog

uninstall:
	rm /usr/local/lib/libdebuglog.so.1.0
	rm /usr/local/lib/libdebuglog.so.1
	rm /usr/local/include/debuglog/debuglog.h
	

clean:
	rm -f *.o *.so.1.0










