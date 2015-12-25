#include <stdbool.h>
#include <string.h>
#include <unistd.h> // read()
#include <sys/types.h> // ssize_t
#include <netinet/in.h> // ntohs()
#include "tcp_manager.h"
#include "ipv4.h"
#include "tcp.h"
#include "buffer_queue.h"
#include "update_state.h"

static uint16_t tcp_manager_raw_read(int fd, void* buf, size_t count);

tcp_state_t TCPMGR; // global TCP manager instance

void tcp_manager_initialize() {
    TCPMGR.connections = NULL;
    TCPMGR.ready = true;
}

// ...
int tcp_manager_register(int fd, uint32_t ipaddress, uint16_t port) {
    // insert into TCPMGR.connections

    tcp_conn_t* con = malloc(sizeof(tcp_conn_t));

    con->fd = fd;
    con->remote_ipaddr = ipaddress;
    con->remote_port = port;

    con->state = LISTEN;
    con->newstate = LISTEN; // no transition yet

    con->last_flag_recv = NOTHING;
    con->flag_to_be_send = NOTHING;

    buffer_queue_init(&con->raw_read_queue);
    buffer_queue_init(&con->payload_read_queue);
    buffer_queue_init(&con->write_queue);

    con->next = NULL;

    if (TCPMGR.connections == NULL) {
        TCPMGR.connections = con;
    } else { // append to the end
        tcp_conn_t* aux;
        for (aux = TCPMGR.connections; aux->next != NULL; aux = aux->next);
        aux->next = con;
    }

    return 0;
}

int tcp_manager_read(int fd, void* buf, size_t count) {

    // get the connection struct
    tcp_conn_t* con = fetch_con_by_fd(fd);

    uint16_t pkg_len = tcp_manager_raw_read(fd, buf, count);

    void* pkg = malloc(pkg_len);

    //size_t buffer_queue_dequeue(buffer_queue_t* q, void* dest, size_t length);
    size_t got = buffer_queue_dequeue(&(con->raw_read_queue), pkg, pkg_len);

    if (got != pkg_len) {
        printf("[got != pkg_len] well, for now it's broken, fix this case later...\n");
    }

    // do the ip thing
    ipv4_header_t ipv4_head; // crumple the stack, it's fun
    deserialize_ipv4(&ipv4_head, pkg);

    // do the tcp thing
    tcp_header_t tcp_head; // crumple, crumple, ...
    deserialize_tcp(&tcp_head, pkg + (ipv4_head.ihl << 2));



    return -1;
}

static uint16_t tcp_manager_raw_read(int fd, void* buf, size_t count) {
    size_t rcvd;

    // get the connection struct
    tcp_conn_t* con = fetch_con_by_fd(fd);

    // read base IPv4 header from raw socket
    void* ipbuf = malloc(IPV4_HEADER_BASE_LENGTH);
    rcvd = read(con->fd, ipbuf, IPV4_HEADER_BASE_LENGTH);
    // and enqueue in raw read queue
    buffer_queue_enqueue(&con->raw_read_queue, ipbuf, rcvd);

    // read the rest of the packet from raw socket
    uint16_t tot_len = ntohs( *( (uint16_t*)  buf+2 ) ); // trust me on this one
    void* pkgbuf = malloc(IPV4_HEADER_BASE_LENGTH);
    rcvd = read(con->fd, pkgbuf, tot_len - IPV4_HEADER_BASE_LENGTH);
    // and enqueue in raw read queue
    buffer_queue_enqueue(&con->raw_read_queue, pkgbuf, rcvd);

    return tot_len;
}


int tcp_manager_write(int fd, void* buf, size_t count) {




    return -1;
}

int tcp_manager_close(int fd) {

    return -1;
}

tcp_conn_t* fetch_con_by_fd(int fd) {
    tcp_conn_t* aux;
    for (aux = TCPMGR.connections; aux != NULL; aux = aux->next) {
        if (aux->fd == fd) {
            return aux;
        }
    }
    return NULL; // no result was found
}
