Date:  2/12/17

These programs are used to test sending/receiving serial port data between 
Arduino and a c or cpp program running in a Linux OS.

The programs are under development and not ready for distribution. 


Program pairs

Arduino send_byte and Linux interrupt_byte_test.c




linuxSerialSidePolling.cpp
reads from the serial port then sleeps for a second. It is in a polling loop that performs
only this action. It receives almost every byte, but occassionally misses one. We
need to add some testing to this program by letting it run for a long interval and adding
code to see how many bytes it loses. We can do this by comparing the count received to
the expected count received. I suspect it loses a byte due to the fact that the microprocessor
count is not exactly 1 second. 




Next steps:

1) Continue development and testing of interrupt_byte_test.c
2) Please maintain this program as is, and create a new program for specific read applications.

Reference for asynchronous input, serial programming:

http://tldp.org/HOWTO/Serial-Programming-HOWTO/x115.html


