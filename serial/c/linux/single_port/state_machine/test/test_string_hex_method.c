#include <stdio.h>
#include <stdint.h>
#include <string.h>


void convert_array_to_hex_string(uint8_t* destination, ssize_t dlength, const uint8_t* source, ssize_t slength)
{
	ssize_t i;
	memset(destination, 0, dlength);
	for(i = 0; i < slength; ++i){
		sprintf(&destination[i*3], "%02x ", source[i]); 
	}
}


int main(void)
{
	uint8_t* msg = "abc ";
	uint8_t hexmsg[13];
	int i;

	//printf("strlen(msg) %ld\n", strlen(msg));

	/*memset(hexmsg, 0, 9);
	for(i = 0; i < strlen(msg); ++i){
		printf("%02x ", msg[i]);
		sprintf(&hexmsg[i*3], "%02x ", msg[i]);
	} */

	convert_array_to_hex_string(hexmsg, 13, msg, strlen(msg));

	puts("");
	printf("hexmsg: %s\n", hexmsg);

	return 0;
}