#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h> // inet_aton()

#include <sys/ioctl.h> // SIOCGIFINDEX
#include <net/if.h> // ifreq

#include <errno.h>
#include "api.h"
#include "tcp_manager.h"


bool SET_IP_HDRINCL = false;
bool BINDING = true;
bool RESTRICT_LOOPBACK = false;


int nyx_accept(uint16_t port, uint32_t ipaddress) {
    int s;

    // make new raw socket
    int raw_fd = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    //int raw_fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW); // IPPROTO_RAW = we need to also do IPv4 on our own
    printf("Got raw fd: %d %s\n", raw_fd, strerror(errno));

    if (SET_IP_HDRINCL) {
        // we want to do the header on our own
        int tmp = 1;
        const int *val = &tmp;
        if (setsockopt(raw_fd, IPPROTO_IP, IP_HDRINCL, val, sizeof(tmp)) < 0) {
            printf("Error: setsockopt() - Cannot set HDRINCL!\n");
            exit(-1);
        }
    }

    if (BINDING) {
        // Bind socket so that we can receive stuff
        struct sockaddr_in myaddr;
        myaddr.sin_family = AF_INET; // sin_family is always set to AF_INET.
        // The basic IP protocol does not supply port numbers, they are implemented by higher level protocols like udp(7) and tcp(7). On raw sockets sin_port is set to the IP protocol.
        myaddr.sin_port = 6; // 6 = TCP // htons(port);
        //inet_aton("127.0.0.1", &myaddr.sin_addr.s_addr);
        myaddr.sin_addr.s_addr = htonl(ipaddress);
        s = bind(raw_fd, (struct sockaddr *) &myaddr, sizeof(struct sockaddr_in));
        printf("bind(): %d\n", s);
    }

    /* RESTRICT SOCKET TO LOCAL LOOPBACK INTERFACE */
    if (RESTRICT_LOOPBACK) {
        char* interface = "lo"; // Interface to send packet through.
        struct ifreq ifr;
        memset(&ifr, 0, sizeof(ifr));
        snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", interface);
        ioctl(raw_fd, SIOCGIFINDEX, &ifr); // Use ioctl() to look up interface index which we will use to
        //printf("Index for interface %s is %i\n", interface, ifr.ifr_ifindex);
        printf("Binding raw socket to interface %s\n", interface);
        setsockopt(raw_fd, SOL_SOCKET, SO_BINDTODEVICE, &ifr, sizeof(ifr)); // bind socket descriptor fd to specified interface with setsockopt()
    }

    // remember this particular connection in our TCP state
    tcp_manager_register(raw_fd, ipaddress, port);

    // initiate three-way handshake (ONLY THE SERVER SIDE)
    int handshake = tcp_handshake(raw_fd);

    return raw_fd;
}

ssize_t nyx_read(int fd, void* buf, size_t count) {
    return tcp_manager_read(fd, buf, count);
}

ssize_t nyx_write(int fd, void* buf, size_t count) {
    return tcp_manager_write(fd, buf, count);
}

void nyx_close(int fd) {
    tcp_manager_close(fd);
}
