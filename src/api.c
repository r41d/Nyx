#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <netdb.h> // needed for socket(), getprotobyname() and [struct protoent]
//These are included in netdb.h:
//  <sys/socket.h>
//  <netinet/in.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include "api.h"
#include "tcp_manager.h"


static int interface_id(int fd, const char *interface) {
    struct ifreq ir;

    memset(&ir, 0, sizeof(ir));
    strncpy(ir.ifr_name, interface, IFNAMSIZ - 1);
    ir.ifr_name[IFNAMSIZ - 1] = '\0';

    if (ioctl(fd, SIOCGIFINDEX, &ir) == -1) {
        return -1;
    }

    return ir.ifr_ifindex;
}

int nyx_accept(uint16_t port, uint32_t ipaddress) {

    // make new raw socket
    struct protoent* protocol_tcp = (struct protoent*) getprotobyname("TCP");
    int new_raw_fd = socket(AF_INET, SOCK_RAW, protocol_tcp->p_proto);
    free(protocol_tcp);

    int iid = interface_id(new_raw_fd, "lo");
    struct sockaddr_ll sall;
    memset(&sall, 0, sizeof(sall));
    sall.sll_family = AF_INET;
    sall.sll_protocol = htons(ETH_P_IP);
    sall.sll_ifindex = iid;
    bind(new_raw_fd, (struct sockaddr *) &sall, sizeof(sall));

    // remember this particular connection in our TCP state
    tcp_manager_register(new_raw_fd, ipaddress, port);



    // initiate three-way handshake
}

ssize_t nyx_read(int fd, void* buf, size_t count) {

}

ssize_t nyx_write(int fd, void* buf, size_t count) {

}

void nyx_close(int fd) {

}
