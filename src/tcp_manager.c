#include <stdbool.h>
#include <string.h>
#include "tcp_manager.h"
#include "update_state.h"

tcp_state_t TCPMGR; // global TCP manager instance

void tcp_manager_initialize() {
    TCPMGR.connections = NULL;
    TCPMGR.ready = true;
}

// ...
int tcp_manager_register(int fd, uint32_t ipaddress, uint16_t port) {
    // insert into TCPMGR.connections

    return -1;
}

int tcp_manager_read(int fd, void* buf, size_t count) {

    return -1;
}

int tcp_manager_write(int fd, void* buf, size_t count) {

    return -1;
}

int tcp_manager_close(int fd) {

    return -1;
}
