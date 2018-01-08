All: consoleTest fileLogTest

consoleTest: console_test.c
	gcc -std=c11 -Wall -Wextra -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align \
	-Wconversion console_test.c -o consoleTest \
	-I /usr/local/include/debuglog/ \
	-L /usr/local/lib/debuglog.so -ldebuglog -lm

fileLogTest: file_log_test.c
	gcc -std=c11 -Wall -Wextra -Wshadow -Wpointer-arith -Wcast-qual -Wcast-align \
	-Wconversion file_log_test.c -o fileLogTest \
	-I /usr/local/include/debuglog/ \
	-L /usr/local/lib/debuglog.so -ldebuglog -lm
	

clean: 
	rm consoleTest fileLogTest

