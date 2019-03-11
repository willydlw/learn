CXX := c++ 

CXXFLAGS := -g -Wall -Wextra -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align  \
	-Wconversion -pedantic -fpic 

INCLUDES := -I /usr/local/include/debuglog/ 
LIBS	 := -L /usr/local/lib/debuglog.so -ldebuglog -lm -lc -lrt
# -lrt for mq_open

OBJECTS := socketClient.o socketServer.o 

all:  $(OBJECTS)
	$(CXX) -shared -Wl,-soname,libmysocket.so.1 -o libmysocket.so.1.0 \
	$(OBJECTS) $(LIBS)
	

%.o: %.cxx
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $(input) -o $(output) 
	 

prefix=/usr/local/include

.PHONY: install
install:
	mv *.so.1.0 /usr/local/lib
	ln -sf /usr/local/lib/libmysocket.so.1.0 /usr/local/lib/libmysocket.so
	ln -sf /usr/local/lib/libmysocket.so.1.0 /usr/local/lib/libmysocket.so.1
	cp *.h $(prefix)/mysocket
	rm *.o 
	
.PHONY: uninstall
uninstall:
	rm /usr/local/lib/libmysocket.so.1.0
	rm /usr/local/lib/libmysocket.so.1
	rm $(prefix)/mysocket/*.h
	
.PHONY: clean 
clean:
	rm -f *.o *.so.1.0

