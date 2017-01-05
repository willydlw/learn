Date:  2/26/15, Time 3:00 p.m.

These programs are used to test sending/receiving serial port data between 
an atmega32 microprocessor and a cpp program running in a Linux OS.

The programs are working correctly. The atmega program, serialDataCounter.c is 
sending 2 bytes of data to the linuxSerialSide.cpp program at 1/100 second intervals.

There are two Linux programs used to test data reception. 

linuxSerialSidePolling.cpp
reads from the serial port then sleeps for a second. It is in a polling loop that performs
only this action. It receives almost every byte, but occassionally misses one. We
need to add some testing to this program by letting it run for a long interval and adding
code to see how many bytes it loses. We can do this by comparing the count received to
the expected count received. I suspect it loses a byte due to the fact that the microprocessor
count is not exactly 1 second. 

linuxSerialSide.cpp
reads from the serial port after getting a signal interrupt from the OS. Program 
successfully reads one byte at a time. After reading two bytes, it combines them into the
count received.

The program is configured to count any errors between the previous count and current count.
Testing it over a loop of 1000 and 10000 resulted in no lost bytes at 1/100 second.




Next steps:

1) Program is ready to be used to read serial data from a microprocessor.
2) Please maintain this program as is, and create a new program for specific read applications.

Reference for asynchronous input, serial programming:

http://tldp.org/HOWTO/Serial-Programming-HOWTO/x115.html


