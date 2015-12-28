#include <stdbool.h>
#include <string.h>
#include <unistd.h> // read()
#include <sys/types.h> // ssize_t
#include <netinet/in.h> // ntohs()
#include <fcntl.h> // fcntl()
#include <errno.h>
#include "tcp_manager.h"
#include "ipv4.h"
#include "tcp.h"
#include "buffer_queue.h"
#include "update_state.h"

static uint16_t read_raw_socket(int fd);
static void handle_tcp_header(tcp_conn_t* con, tcp_header_t* tcp_head);
static uint16_t process_raw_queue(int fd, ipv4_header_t** ipv4_head_p, tcp_header_t** tcp_head_p, void** payload);
static void send_empty_ack_packet(int fd, int ack_num);
static int fd_set_blocking(int fd, int blocking);
static void raw2payload_shoveling(int fd, void* payload, size_t payload_len);

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

    con->local_port = htons(1337); // TODO: generate randomly

    con->state = LISTEN;
    con->newstate = LISTEN; // no transition yet

    con->last_flag_recv = NOTHING;
    con->flag_to_be_send = NOTHING;

    con->local_seq_num = 100; // start with 100 for fun and profit

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

    size_t bytes_ready = 0;

    while (bytes_ready < 1) {

        // printf("bytes ready < 1\n");

        // read from raw socket as much as possible in NONBLOCKing mode
        // printf("read_raw_socket\n");

        uint16_t got_raw = read_raw_socket(fd);
        if (got_raw)
            printf("got %d from read_raw_socket\n", got_raw);



        // shovel from raw queue to payload queue
        // printf("process_raw_queue\n");
        ipv4_header_t* ipv4_head = NULL;
        tcp_header_t* tcp_head = NULL;
        void* payload = NULL;
        uint16_t payload_bytes_available = process_raw_queue(fd, &ipv4_head, &tcp_head, &payload);
        // any of ipv4_head, tcp_head or payload may be NULL after this call!

        // here the state transitioning (and stuff) is done
        handle_tcp_header(con, tcp_head);

        if (con->state == ESTABLISHED) {
            if (payload_bytes_available)
                printf("process_raw_queue reports %d bytes of payload available \n", payload_bytes_available);
            raw2payload_shoveling(fd, payload, payload_bytes_available);

            // this ack number needs to be send with the next ACK
            if(tcp_head != NULL && payload_bytes_available > 0)
                con->next_ack_num_to_send = tcp_head->seq_num + payload_bytes_available;

            bytes_ready += payload_bytes_available;
        }
        // we will never leave this while loop unless we are in the ESTABLISHED state

    }


    if (con->next_ack_num_to_send > con->last_ack_num_sent) {
        // give the client the ACK
        printf("send_empty_ack_packet\n");
        send_empty_ack_packet(fd, con->next_ack_num_to_send);
    }

    printf("buffer_queue_dequeue\n");
    int payload_got = buffer_queue_dequeue(&con->payload_read_queue, buf, count);

    return payload_got;
}

static void send_empty_ack_packet(int fd, int ack_num) {

    printf("sending empty ack...\n");

    // get the connection struct
    tcp_conn_t* con = fetch_con_by_fd(fd);

    //tcp_header_t* assemble_tcp_header(uint16_t src_port,
    //                                  uint16_t dest_port,
    //                                  uint32_t seq_num,
    //                                  uint32_t ack_num,
    //                                  flag_t flags_to_be_send,
    //                                  uint16_t window);
    tcp_header_t* ack_tcp_head =
        assemble_tcp_header(con->local_port,
                            con->remote_port,
                            con->local_seq_num,
                            ack_num,
                            ACK,
                            4 << 8); // Receive Window = 4 kilobyte
    char* buf = malloc(TCP_HEADER_BASE_LENGTH);
    serialize_tcp(buf, ack_tcp_head);

    // write the ack tcp header to raw socket
    // we only need to send the tcp header, IP is taken care of
    write(fd, buf, TCP_HEADER_BASE_LENGTH);

    free(ack_tcp_head);
    free(buf);
}

// read from raw socket as much as possible
static uint16_t read_raw_socket(int fd) {

    //printf("tcp manager raw read\n");

    ssize_t rcvd;
    size_t total = 0;

    // get the connection struct
    tcp_conn_t* con = fetch_con_by_fd(fd);

    // turn to nonblocking mode
    fd_set_blocking(fd, false);

    do {
        //printf("mallocing 512...\n");
        void* rawbuf = malloc(512);
        //printf("read on raw...\n");
        rcvd = read(con->fd, rawbuf, 512);
        // printf("%zd \n", rcvd);
        if (rcvd <= 0 && errno == EWOULDBLOCK) { // no data available
            rcvd = 0; // was -1
        }
        rawbuf = realloc(rawbuf, rcvd); // deallocates if rcvd=0
        if (rcvd > 0) {
            // printf("read_raw_socket: %d: %x\n", rcvd, rawbuf);
            fwrite(rawbuf, rcvd, 1, stdout);
            printf("Enqueueing %zu bytes into raw_read_queue\n", rcvd);
            buffer_queue_enqueue(&con->raw_read_queue, rawbuf, rcvd);
            total += rcvd;
        }
    } while(rcvd > 0);

    // back to blocking mode
    fd_set_blocking(fd, true);

    return total;
}

// shovel from raw queue to payload queue
static uint16_t process_raw_queue(int fd, ipv4_header_t** ipv4_head_p, tcp_header_t** tcp_head_p, void** payload) {
    *ipv4_head_p = NULL;
    *tcp_head_p = NULL;
    *payload = NULL;

    // printf("tcp manager process raw queue\n");

    int ret;

    // get the connection struct
    tcp_conn_t* con = fetch_con_by_fd(fd);

    if (buffer_queue_length(&con->raw_read_queue) < 20) {
        // printf("raw read queue < 20\n");
        return 0;
    }

    //size_t buffer_queue_top(buffer_queue_t* q, void* dest, size_t length);
    printf("mallocing checkbuf\n");
    void* checkbuf = malloc(IPV4_HEADER_BASE_LENGTH);
    printf("buffer queue top -> checkbuf\n");
    size_t got = buffer_queue_top(&con->raw_read_queue, checkbuf, IPV4_HEADER_BASE_LENGTH);

    if (got < IPV4_HEADER_BASE_LENGTH) {
        free(checkbuf);
        return 0;
    }

    // we have a complete IPv4 Header at this point!

    ipv4_header_t ipv4_head; // crumple the stack, it's fun
    printf("Got an IPv4 header\n");
    deserialize_ipv4(&ipv4_head, checkbuf); // additional options on this one may be filled with gargabe

    if (buffer_queue_length(&con->raw_read_queue) >= ipv4_head.length) {
        // WE ARE GO, THERE'S A WHOLE PACKET READY FOR PICKUP

        uint16_t pkg_len = ipv4_head.length;

        void* pkg = malloc(pkg_len);
        size_t got = buffer_queue_dequeue(&con->raw_read_queue, pkg, pkg_len);

        if (got != pkg_len)
            printf("ERROR: got != pkg_len\n");

        // do the ip thing
        deserialize_ipv4(&ipv4_head, pkg);
        *ipv4_head_p = &ipv4_head;

        // do the tcp thing
        tcp_header_t tcp_head; // crumple, crumple, ...
        printf("Got a TCP header\n");
        deserialize_tcp(&tcp_head, pkg + (ipv4_head.ihl << 2));
        *tcp_head_p = &tcp_head;

        // determine if we have any payload and return the amount of bytes it has
        size_t payload_offset = (ipv4_head.ihl << 2) + (tcp_head.data_offset << 2);
        size_t payload_len = pkg_len - payload_offset;
        if (payload_len > 0) {
            // let's hope this works
            *payload = malloc(payload_len);
            memcpy(*payload, pkg+payload_offset, payload_len);
        }
        ret = payload_len;

        free(pkg);
    }

    // clean up
    free(checkbuf);

    return ret;

}

static void raw2payload_shoveling(int fd, void* payload, size_t payload_len) {
    // get the connection struct
    tcp_conn_t* con = fetch_con_by_fd(fd);

    if (payload != NULL && payload_len > 0) {
        void* payload = malloc(payload_len);
        memcpy(payload, payload, payload_len);
        buffer_queue_enqueue(&con->payload_read_queue, payload, payload_len);
        printf("Enqueueing %zu bytes of payload\n", payload_len);
    }
}

static void handle_tcp_header(tcp_conn_t* con, tcp_header_t* tcp_head) {

    if(tcp_head == NULL)
        return;

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
    printf("Changing state from %u to %u\n", con->state, con->newstate);
    con->state = con->newstate;

    // after this, state and newstate are the same

}

int tcp_manager_write(int fd, void* buf, size_t count) {


    return -1;
}

int tcp_manager_close(int fd) {

    return -1;
}

tcp_conn_t* fetch_con_by_fd(int fd) {
    //printf("fetching con by fd\n");
    tcp_conn_t* aux;
    for (aux = TCPMGR.connections; aux != NULL; aux = aux->next) {
        if (aux->fd == fd) {
            //printf("Returning con %d\n", aux->fd);
            return aux;
        }
    }
    printf("CON NOT FOUND!\n");
    return NULL; // no result was found
}

/** http://code.activestate.com/recipes/577384-setting-a-file-descriptor-to-blocking-or-non-block/
 *
 * Set a file descriptor to blocking or non-blocking mode.
 *
 * @param fd The file descriptor
 * @param blocking 0:non-blocking mode, 1:blocking mode
 *
 * @return 1:success, 0:failure.
 **/
static int fd_set_blocking(int fd, int blocking) {
    /* Save the current flags */
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
        return 0;

    if (blocking)
        flags &= ~O_NONBLOCK;
    else
        flags |= O_NONBLOCK;

    return (fcntl(fd, F_SETFL, flags) != -1);
}
