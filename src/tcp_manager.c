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

static uint16_t tcp_manager_raw_read(int fd);
static void handle_tcp_header(tcp_conn_t* con, tcp_header_t* tcp_head);

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

    uint16_t pkg_len = tcp_manager_raw_read(fd);

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

// read from raw socket as much as possible
static uint16_t tcp_manager_raw_read(int fd) {
    size_t rcvd = -1;
    size_t total = 0;

    // get the connection struct
    tcp_conn_t* con = fetch_con_by_fd(fd);

    while(rcvd != 0) {
        void* rawbuf = malloc(512);
        rcvd = read(con->fd, rawbuf, 512);
        rawbuf = realloc(rawbuf, rcvd); // deallocates if rcvd=0
        if (rcvd > 0) {
            buffer_queue_enqueue(&con->raw_read_queue, rawbuf, rcvd);
            total += rcvd;
        }
    }

    return total;
}

// shovel from raw queue to payload queue
static uint16_t tcp_manager_process_raw_queue(int fd) {
    int ret;

    // get the connection struct
    tcp_conn_t* con = fetch_con_by_fd(fd);

    //size_t buffer_queue_top(buffer_queue_t* q, void* dest, size_t length);
    void* checkbuf = malloc(IPV4_HEADER_BASE_LENGTH);
    size_t got = buffer_queue_top(&con->raw_read_queue, checkbuf, IPV4_HEADER_BASE_LENGTH);

    if (got < IPV4_HEADER_BASE_LENGTH) {
        free(checkbuf);
        return 0;
    }

    // we have a complete IPv4 Header!

    ipv4_header_t ipv4_head; // crumple the stack, it's fun
    deserialize_ipv4(&ipv4_head, checkbuf); // additional options on this one may be filled with gargabe

    if (buffer_queue_length(&con->raw_read_queue) > ipv4_head.length) { // WE ARE GO

        uint16_t pkg_len = ipv4_head.length;

        void* pkg = malloc(pkg_len);
        size_t got = buffer_queue_dequeue(&con->raw_read_queue, pkg, pkg_len);

        if (got != pkg_len)
            printf("ERROR: got != pkg_len\n");

        // do the ip thing
        deserialize_ipv4(&ipv4_head, pkg);

        // do the tcp thing
        tcp_header_t tcp_head; // crumple, crumple, ...
        deserialize_tcp(&tcp_head, pkg + (ipv4_head.ihl << 2));

        handle_tcp_header(con, &tcp_head);

        size_t payload_offset = (ipv4_head.ihl << 2) + (tcp_head.data_offset << 2);
        size_t payload_len = pkg_len - payload_offset;
        ret = payload_len;
        if (payload_len > 0) {
            void* payload = malloc(payload_len);
            memcpy(payload, pkg+payload_offset, payload_len);
            buffer_queue_enqueue(&con->payload_read_queue, pkg+payload_offset, payload_len);
            send_ack_immediately(...);
        }
        free(pkg);
    }

    // clean up
    free(checkbuf);

    return ret;

}

static void handle_tcp_header(tcp_conn_t* con, tcp_header_t* tcp_head) {

    // set last_flag_recv according to flags
    if (tcp_head->syn && tcp_head->ack)
        con->last_flag_recv = SYNACK;
    else if (tcp_head->fin && tcp_head->ack)
        con->last_flag_recv = FINACK;
    else if (tcp_head->syn)
        con->last_flag_recv = SYN;
    else if (tcp_head->fin)
        con->last_flag_recv = FIN;
    else if (tcp_head->ack)
        con->last_flag_recv = ACK;

    // update received ACKs
    if (tcp_head->ack)
        con->last_ack_num_rcvd = tcp_head->ack_num;

    // update the state according to TCP FSA (see update_state.h)
    update_state(con);

    // here, we can insert stuff that needs to know both the old and the new state
    // ...

    // new the new state the actual state
    con->state = con->newstate;

    // after this, state and newstate are the same

    if (con->flag_to_be_send == SYNACK) {
        // send SYN ACK packet!! (second phase of 3 way handshake)
    }


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
