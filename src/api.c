#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <netinet/in.h>

#include <sys/ioctl.h> // SIOCGIFINDEX
#include <net/if.h> // ifreq

#include <errno.h>
#include "api.h"
#include "tcp_manager.h"

int nyx_accept(uint16_t port, uint32_t ipaddress) {

    // make new raw socket
    int raw_fd = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
    printf("Got raw fd: %d %s\n", raw_fd, strerror(errno));


    /* BIND SOCKET TO LOCAL LOOPBACK INTERFACE */

    // Interface to send packet through.
    char* interface = "lo";
    // Use ioctl() to look up interface index which we will use to
    // bind socket descriptor fd to specified interface with setsockopt()
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    snprintf(ifr.ifr_name, sizeof(ifr.ifr_name), "%s", interface);
    ioctl(raw_fd, SIOCGIFINDEX, &ifr);
    //printf("Index for interface %s is %i\n", interface, ifr.ifr_ifindex);
    // Bind socket to interface index.
    printf("Binding raw socket to interface %s\n", interface);
    setsockopt(raw_fd, SOL_SOCKET, SO_BINDTODEVICE, &ifr, sizeof(ifr));


    // remember this particular connection in our TCP state
    tcp_manager_register(raw_fd, ipaddress, port);

    // initiate three-way handshake (ONLY THE SERVER SIDE)

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
