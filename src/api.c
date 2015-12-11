#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <netinet/in.h>
#include "api.h"
#include "tcp_manager.h"

int nyx_accept(uint16_t port, uint32_t ipaddress) {

    // make new raw socket
    int raw_fd = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);

    // remember this particular connection in our TCP state
    tcp_manager_register(raw_fd, ipaddress, port);



    // initiate three-way handshake


    return -1;
}

ssize_t nyx_read(int fd, void* buf, size_t count) {

    return -1;
}

ssize_t nyx_write(int fd, void* buf, size_t count) {

    return -1;
}

void nyx_close(int fd) {

}
