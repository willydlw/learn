/** Reads and displays the mac address for the network
 *  interface
 * 
 */

#include <sys/ioctl.h>              // struct ifreq
#include <net/if.h>                 // struct ifreq
#include <sys/types.h>              // socket
#include <sys/socket.h>             // socket

#include <errno.h>


#include <cstdio>
#include <cstring> 
#include <unistd.h>                 // close


// mac address is six two hex digits fields, separated by 5 colons
int getMacAddress(char *macAddress, const char* interfaceName, int domain){
        int fd;
        struct ifreq ifr;
        unsigned char *mac;

        // open socket connection 
        fd = socket(AF_INET, SOCK_DGRAM, 0);
        if(fd == -1){
            fprintf(stderr, "socket failed to return file descriptor, interfaceName: %s\n", interfaceName);
            return -1;
        }

        ifr.ifr_addr.sa_family = domain;
        strncpy((char*)ifr.ifr_name, interfaceName, IFNAMSIZ-1);

        // get device hardware address
        if( ioctl(fd, SIOCGIFHWADDR, &ifr) == -1){
            fprintf(stderr, "ioctl failure, errno: %s\n", strerror(errno));
            return -1;
        }

        close(fd);

        mac = (unsigned char*)(ifr.ifr_hwaddr.sa_data);

        sprintf(macAddress, (const char*)"%2x:%2x:%2x:%2x:%2x:%2x%c",
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], '\0');

        return 0;
}

/**
 *  @param[in] argv[1] is network interface name
 * 
 *  examples: eth0, wlan0
 * 
 */
int main(int argc, char *argv[])
{
    char macAddress[32];

    if(argc < 2){
        fprintf(stderr, "usage: %s <interface name>\n", argv[0]);
        return 1;
    }

    getMacAddress(macAddress, argv[1], AF_INET);
    fprintf(stderr, "macAddress: %s\n", macAddress);
    return 0;
}