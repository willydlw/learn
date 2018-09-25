All: clientserver.o 
	gcc -std=c99 -shared -Wl,-soname,libclientserver.so.1 -o libclientserver.so.1.0 clientserver.o \
	-lc

# command line option ‘-Wtraditional’ is valid for C/ObjC 
#			but not for C++ [enabled by default]
# command line option ‘-Wstrict-prototypes’ is valid for Ada/C/ObjC 
#			but not for C++ [enabled by default]
# command line option ‘-Wmissing-prototypes’ is valid for Ada/C/ObjC 
#			but not for C++ [enabled by default]


# c99 does not recognize getaddrinfo, struct addrinfo, freeaddrinfo, gai_strerror
# add -D_POSIX_C_SOURCE=200112L to get rid of warnings generated


clientserver.o: clientserver.c clientserver.h 
	gcc -std=c99 -D_POSIX_C_SOURCE=200112L -Wall -Wextra -Wshadow -Wpointer-arith \
	-Wcast-qual -Wcast-align  \
	-Wconversion -Wstrict-prototypes -pedantic -fpic -g -c clientserver.c



install:
	mv *.so.1.0 /usr/local/lib
	ln -sf /usr/local/lib/libclientserver.so.1.0 /usr/local/lib/libclientserver.so
	ln -sf /usr/local/lib/libclientserver.so.1.0 /usr/local/lib/libclientserver.so.1
	cp clientserver.h /usr/local/include/clientserver
	

uninstall:
	rm /usr/local/lib/libclientserver.so.1.0
	rm /usr/local/lib/libclientserver.so.1
	rm /usr/local/include/client_server/clientserver.h
	

clean:
	rm -f *.o *.so.1.0










