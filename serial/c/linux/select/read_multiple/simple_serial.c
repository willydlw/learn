#include <stdio.h>    // Standard input/output definitions 
#include <unistd.h>   // UNIX standard function definitions 
#include <fcntl.h>    // File control definitions 
#include <errno.h>    // Error number definitions 
#include <termios.h>  // POSIX terminal control definitions 
#include <string.h>   // String function definitions 
#include <stdint.h>	
#include <stdbool.h>
#include <sys/ioctl.h>


// takes the string name of the serial port (e.g. "/dev/tty.usbserial","COM1")
// and a baud rate (bps) and connects to that port at that speed and 8N1.
// opens the port in fully raw mode so you can send binary data.
// returns valid fd, or -1 on error
int serialport_init(const char* serialport, int baud)
{
    struct termios toptions;
    int fd;
    
    //fd = open(serialport, O_RDWR | O_NOCTTY | O_NDELAY);
    fd = open(serialport, O_RDWR | O_NONBLOCK );
    
    if (fd == -1)  {
        perror("serialport_init: Unable to open port ");
        return -1;
    }
    
    //int iflags = TIOCM_DTR;
    //ioctl(fd, TIOCMBIS, &iflags);     // turn on DTR
    //ioctl(fd, TIOCMBIC, &iflags);    // turn off DTR

    if (tcgetattr(fd, &toptions) < 0) {
        perror("serialport_init: Couldn't get term attributes");
        return -1;
    }
    speed_t brate = baud; // let you override switch below if needed
    switch(baud) {
    case 4800:   brate=B4800;   break;
    case 9600:   brate=B9600;   break;
#ifdef B14400
    case 14400:  brate=B14400;  break;
#endif
    case 19200:  brate=B19200;  break;
#ifdef B28800
    case 28800:  brate=B28800;  break;
#endif
    case 38400:  brate=B38400;  break;
    case 57600:  brate=B57600;  break;
    case 115200: brate=B115200; break;
    }
    cfsetispeed(&toptions, brate);
    cfsetospeed(&toptions, brate);

    // 8N1
    toptions.c_cflag &= ~PARENB;
    toptions.c_cflag &= ~CSTOPB;
    toptions.c_cflag &= ~CSIZE;
    toptions.c_cflag |= CS8;
    // no flow control
    toptions.c_cflag &= ~CRTSCTS;

    //toptions.c_cflag &= ~HUPCL; // disable hang-up-on-close to avoid reset

    toptions.c_cflag |= CREAD | CLOCAL;  // turn on READ & ignore ctrl lines
    toptions.c_iflag &= ~(IXON | IXOFF | IXANY); // turn off s/w flow ctrl

    toptions.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); // make raw
    toptions.c_oflag &= ~OPOST; // make raw

    // see: http://unixwiz.net/techtips/termios-vmin-vtime.html
    toptions.c_cc[VMIN]  = 0;
    toptions.c_cc[VTIME] = 0;
    //toptions.c_cc[VTIME] = 20;
    
    tcsetattr(fd, TCSANOW, &toptions);
    if( tcsetattr(fd, TCSAFLUSH, &toptions) < 0) {
        perror("init_serialport: Couldn't set term attributes");
        return -1;
    }

    return fd;
}


int serialport_read_until(int fd, uint8_t* buf, char until, int buf_max, int timeout)
{
    char b[1];  // read expects an array, so we give it a 1-byte array
    int i=0;
    do { 
        int n = read(fd, b, 1);  // read a char at a time
        if( n==-1) return -1;    // couldn't read
        if( n==0 ) {
            usleep( 1 * 1000 );  // wait 1 msec try again
            timeout--;
            if( timeout==0 ) return -2;
            continue;
        }
 
        printf("serialport_read_until: i=%d, n=%d b='%c'\n",i,n,b[0]); // debug

        buf[i] = b[0]; 
        i++;
    } while( b[0] != until && i < buf_max && timeout>0 );

    buf[i] = 0;  // null terminate the string
    return i;
}


int main(){

	int fd = -1;

	const char* serial_device_path = "/dev/ttyACM0";

	int baudrate = 9600;

	int bytes_read;

	uint8_t buf[16];

	int receiveIndex = 0;

	bool receivingData = false;
	bool newSensorData = false;
	bool formingSensorID = false;

	uint16_t sensorData;

	fd = serialport_init(serial_device_path, baudrate);

	if(fd == -1){
		puts("serial port did not open");
		return -1;
	}

	while(1){

		bytes_read = read(fd, buf, 6);
		//bytes_read = serialport_read_until(fd, buf, 'Z', 16, 100);

		TODO: REWRITE WITH STATE MACHINE
		 if(bytes_read){
		 	for(int i = 0; i < bytes_read; ++i){
		 		if(receivingData){
		 			if(formingSensorID){
		 				sensorID[idIndex] = buf[i];
		 				++idIndex;
		 				if(idIndex > 1){
		 					formingSensorID = false;
		 				}
		 			}
		 			if(buf[i] == '>'){
		 				// end marker
		 				receivingData = false;
		 				newSensorData = true;
		 			}
		 			else if(buf[i] == )
		 		}
		 		else if(buf[i] == '<'){
		 			// start marker
		 			receivingData = true;
		 			formingSensorID = true;
		 			idIndex = 0;
		 		}
		 	}

		 	if(!receivingData){

		 	}

		 	fprintf(stderr, "bytes_read: %d, buf[0]: %x\n", bytes_read, buf[0]);  
		 }

;
		 //usleep(100);


	}


   


	return 0;
}