
                 Read Me


*******************************************************
*  Description 
*******************************************************

Objective: 		Transmit multiple serial bytes.

Functionality: 	send_bytes.ino program 
					
					setup
						enables serial communication at 9600
						intialize count to 0

					infinite while loop
						transmit start marker
						transmit sensor id
						transmit msb of count
						transmit lsb of count
						transmit end marker


*******************************************************
*  Source files
*******************************************************


Name:	send_bytes.ino   


*******************************************************
*  Circumstances of programs
*******************************************************
Date: 12/24/2017

	The program compiles and runs successfully. 
   
	The program was developed and tested on Ubuntu 16.04,
	using Arduino IDE 1.8.1

   	Hardware: Arduino UNO R3



Program pair

Arduino send_bytes.ino and c/linux/simple_serial/main.c

These two programs are designed to be run together.

   a) Arduino program serially transmits five bytes of binary data
   			 Baud rate = 9600
             
             Transmits:
             	start marker
             	sensor id
             	count value as two bytes (msb transmitted first)
             	end marker
            

   b) Linux simple_serial/main.c
   			Baud rate = 9600

   			Reads and displays serial bytes recevied from Arduino

   			 

Please maintain this program as is, and create a new program for other applications.





*******************************************************
*  How to build and run the program
*******************************************************

Instructions: 	Use the Arduino IDE software to compile and upload the program
				to an Arduino board.


