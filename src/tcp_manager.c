#include <stdbool.h>
#include <string.h>
#include "tcp_manager.h"

tcp_state_t TCPMGR; // global TCP manager instance

int tcp_manager_initialize() {
    TCPMGR.connections = NULL;
    TCPMGR.ready = true;
}

// ...
int tcp_manager_register(int fd, uint32_t ipaddress, uint16_t port) {

}





int main() {
    return 0;
}
