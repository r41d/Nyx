#include <stdbool.h>
#include <string.h>
#include <unistd.h> // read()
#include <sys/types.h> // ssize_t
#include <netinet/in.h> // ntohs()
#include <errno.h>
#include "tcp_manager.h"
#include "ipv4.h"
#include "tcp.h"
#include "buffer_queue.h"
#include "update_state.h"
#include "util.h"

static uint16_t read_raw_socket(tcp_conn_t* con);
static int handle_tcp_header(tcp_conn_t* con, ipv4_header_t* ipv4_head, tcp_header_t* tcp_head);
static uint16_t process_raw_queue(tcp_conn_t* con, ipv4_header_t** ipv4_head_p, tcp_header_t** tcp_head_p, void** payload);
static void send_empty_ack_packet(tcp_conn_t* con);
static void send_synack_packet(tcp_conn_t* con);
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
    con->local_ipaddr = ipaddress;
    con->local_port = port;

    printf("local ip: %d local port: %d\n", con->local_ipaddr, con->local_port);

    con->remote_ipaddr = 0;
    con->remote_port = 0;

    con->sin.sin_family = AF_INET; // sin_family is always set to AF_INET.
    //con->sin->sin_port = 6; // 6 = TCP // The basic IP protocol does not supply port numbers, they are implemented by higher level protocols like udp(7) and tcp(7). On raw sockets sin_port is set to the IP protocol.
    con->sin.sin_port = htons(port);
    con->sin.sin_addr.s_addr = htonl(ipaddress);

    con->state = LISTEN;
    con->newstate = LISTEN; // no transition yet

    con->last_flag_recv = NOTHING;
    con->flag_to_be_send = NOTHING;

    con->local_seq_num = 100; // start with 100 for fun and profit
    con->last_ack_num_rcvd = 0;

    con->last_ack_num_sent = 0;
    con->next_ack_num_to_send = 0;

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

int tcp_handshake(int fd) {

    printf("HANDSHAKING!\n");

    // get the connection struct
    tcp_conn_t* con = fetch_con_by_fd(fd);

    do {
        read_raw_socket(con);
        ipv4_header_t* ipv4_head = NULL;
        tcp_header_t* tcp_head = NULL;
        void* payload = NULL;
        uint16_t payload_bytes_available = process_raw_queue(con, &ipv4_head, &tcp_head, &payload);
        if (payload_bytes_available != 0) {
            printf("GOT PAYLOAD DURING HANDSHAKE\n");
        }
        if (tcp_head != NULL) {
            handle_tcp_header(con, ipv4_head, tcp_head);
        }
    } while (con->state != ESTABLISHED);

    printf(" _   _    _    _   _ ____  ____  _   _    _    _  _____ _   _  ____   ____   ___  _   _ _____ ");
    printf("| | | |  / \\  | \\ | |  _ \\/ ___|| | | |  / \\  | |/ /_ _| \\ | |/ ___| |  _ \\ / _ \\| \\ | | ____|");
    printf("| |_| | / _ \\ |  \\| | | | \\___ \\| |_| | / _ \\ | ' / | ||  \\| | |  _  | | | | | | |  \\| |  _|  ");
    printf("|  _  |/ ___ \\| |\\  | |_| |___) |  _  |/ ___ \\| . \\ | || |\\  | |_| | | |_| | |_| | |\\  | |___ ");
    printf("|_| |_/_/   \\_\\_| \\_|____/|____/|_| |_/_/   \\_\\_|\\_\\___|_| \\_|\\____| |____/ \\___/|_| \\_|_____|");

    return 0;
}

int tcp_manager_read(int fd, void* buf, size_t count) {

    // get the connection struct
    tcp_conn_t* con = fetch_con_by_fd(fd);

    if (con->state != ESTABLISHED) {
        printf("tcp_manager_read call in non-ESTABLISHED state!!!");
        return -1;
    }

    size_t bytes_ready = 0;

    while (bytes_ready < 1) {

        // printf("bytes ready < 1\n");

        // read from raw socket as much as possible in NONBLOCKing mode
        // printf("read_raw_socket\n");

        uint16_t got_raw = read_raw_socket(con);
        if (got_raw)
            printf("got %d from read_raw_socket\n", got_raw);

        // shovel from raw queue to payload queue
        // printf("process_raw_queue\n");
        ipv4_header_t* ipv4_head = NULL;
        tcp_header_t* tcp_head = NULL;
        void* payload = NULL;
        uint16_t payload_bytes_available = process_raw_queue(con, &ipv4_head, &tcp_head, &payload);
        // any of ipv4_head, tcp_head or payload may be NULL after this call!

        if (tcp_head != NULL && con->state)

        // here the state transitioning (and stuff) is done
        handle_tcp_header(con, ipv4_head, tcp_head);

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
        send_empty_ack_packet(con);
    }

    printf("buffer_queue_dequeue\n");
    int payload_got = buffer_queue_dequeue(&con->payload_read_queue, buf, count);

    return payload_got;
}

// read from raw socket until we get something
static uint16_t read_raw_socket(tcp_conn_t* con) {

    //printf("tcp manager raw read\n");

    ssize_t rcvd;
    size_t total = 0;

    // turn to nonblocking mode
    //fd_set_blocking(fd, false);

    do {
        void* rawbuf = malloc(512);
        rcvd = read(con->fd, rawbuf, 512); // this blocks per default
        //if (rcvd <= 0 && errno == EWOULDBLOCK) { // no data available
        //    rcvd = 0; // was -1 most probably
        //}
        if (rcvd < 0) {
            break;
        }
        rawbuf = realloc(rawbuf, rcvd); // deallocates if rcvd=0
        if (rcvd > 0) {
            // printf("read_raw_socket: %d: %x\n", rcvd, rawbuf);
            // fwrite(rawbuf, rcvd, 1, stdout);
            //printf("Enqueueing %zu bytes into raw_read_queue\n", rcvd);
            buffer_queue_enqueue(&con->raw_read_queue, rawbuf, rcvd);
            total += rcvd;
        }
    } while(total <= 0);

    // back to blocking mode
    //fd_set_blocking(fd, true);

    return total;
}

// shovel from raw queue to payload queue
static uint16_t process_raw_queue(tcp_conn_t* con, ipv4_header_t** ipv4_head_p, tcp_header_t** tcp_head_p, void** payload) {
    *ipv4_head_p = NULL;
    *tcp_head_p = NULL;
    *payload = NULL;

    // printf("tcp manager process raw queue\n");

    int ret;

    if (buffer_queue_length(&con->raw_read_queue) < 20) {
        // printf("raw read queue < 20\n");
        return 0;
    }

    //size_t buffer_queue_top(buffer_queue_t* q, void* dest, size_t length);
    // printf("mallocing checkbuf\n");
    void* checkbuf = malloc(IPV4_HEADER_BASE_LENGTH);
    // printf("buffer queue top -> checkbuf\n");
    size_t got = buffer_queue_top(&con->raw_read_queue, checkbuf, IPV4_HEADER_BASE_LENGTH);

    if (got < IPV4_HEADER_BASE_LENGTH) {
        ret = 0;
        goto noipheader;
    }

    // if we got this far, we have a complete IPv4 Header!!!

    ipv4_header_t* ipv4_head = malloc(sizeof(ipv4_header_t));
    deserialize_ipv4(ipv4_head, checkbuf);
    // additional options on this one may be filled with gargabe because checkbuf is capped at 20 bytes

    if (buffer_queue_length(&con->raw_read_queue) >= ipv4_head->length) {
        // WE ARE GO, THERE'S A WHOLE PACKET READY FOR PICKUP

        void* pkg = malloc(ipv4_head->length);
        size_t got = buffer_queue_dequeue(&con->raw_read_queue, pkg, ipv4_head->length);

        if (got != ipv4_head->length)
            printf("ERROR: got != ipv4_head->length\n");

        // do the ip thing
        deserialize_ipv4(ipv4_head, pkg);
        //printf("Got an IPv4 header:\n");
        //dump_ipv4_header(ipv4_head);
        *ipv4_head_p = ipv4_head;

        // do the tcp thing
        tcp_header_t* tcp_head = malloc(sizeof(tcp_header_t));
        deserialize_tcp(tcp_head, pkg + (ipv4_head->ihl << 2));
        //printf("Got a TCP header\n");
        //dump_tcp_header(tcp_head);
        *tcp_head_p = tcp_head;

        // determine if we have any payload and return the amount of bytes it has
        size_t payload_offset = (ipv4_head->ihl << 2) + (tcp_head->data_offset << 2);
        size_t payload_len = ipv4_head->length - payload_offset;
        if (payload_len > 0) {
            // let's hope this works
            *payload = malloc(payload_len);
            memcpy(*payload, pkg+payload_offset, payload_len);
        }
        ret = payload_len;

        free(pkg);
    }

    noipheader:

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

static int handle_tcp_header(tcp_conn_t* con, ipv4_header_t* ipv4_head, tcp_header_t* tcp_head) {

    if (tcp_head == NULL)
        return -1;

    if (con->state != LISTEN // still listening
        && (tcp_head->src_port != con->remote_port || tcp_head->dest_port != con->local_port)) // or alrady initialized
    {
        printf("TCP packet doesn't look like it's for us! (%d->%d)\n", tcp_head->src_port, tcp_head->dest_port);
        return -1;
    }

    int ret = 0;

    con->last_flag_recv = NOTHING;

    // set last_flag_recv according to flags
    if (tcp_head->syn && tcp_head->ack)
        con->last_flag_recv = SYNACK;
    else if (tcp_head->fin && tcp_head->ack)
        con->last_flag_recv = FINACK;
    else if (tcp_head->syn) {
        printf("SETTING SYN!!!!!!!!!!!!!!!!!!!!!!!!!\n");
        con->last_flag_recv = SYN;
    }
    else if (tcp_head->fin) {
        printf("SETTING ACK!!!!!!!!!!!!!!!!!!!YEAH!!!!BABY!!!!!!!!!!!!!!!\n");
        con->last_flag_recv = FIN;
    }
    else if (tcp_head->ack)
        con->last_flag_recv = ACK;

    // update received ACKs
    if (tcp_head->ack)
        con->last_ack_num_rcvd = tcp_head->ack_num;

    con->next_ack_num_to_send = tcp_head->seq_num;

    // update the state according to TCP FSA (see update_state.h)
    update_state(con);

    if (con->state == LISTEN && con->remote_port == 0 && con->remote_ipaddr == 0) {
        printf("Now setting remote_ipaddr and remote_port...\n");
        con->remote_ipaddr = ipv4_head->src_addr;
        con->remote_port = tcp_head->src_port;
    }

    if (con->flag_to_be_send == SYNACK) {
        // we just went from LISTEN to SYN_RECEIVED, we now need to send a SYNACK
        send_synack_packet(con);
        con->flag_to_be_send = NOTHING;
    }

    // new the new state the actual state
    if (con->state != con->newstate)
        printf("Changing state from %u to %u\n", con->state, con->newstate);
    con->state = con->newstate;

    // after this, state and newstate are the same

    return ret;
}

int tcp_manager_write(int fd, void* buf, size_t count) {
    return -1;
}

int tcp_manager_close(int fd) {
    return -1;
}


static void send_empty_ack_packet(tcp_conn_t* con) {

    printf("sending empty ack...\n");

    //ipv4_header_t* ack_ip_head =
    //    assemble_ipv4_header(IPV4_HEADER_BASE_LENGTH, con->local_ipaddr, con->remote_ipaddr);
    //char* ipbuf = malloc(IPV4_HEADER_BASE_LENGTH);
    //serialize_ipv4(ipbuf, ack_ip_head);
    //free(ack_ip_head);
    //write_to_raw_socket(con, ipbuf, IPV4_HEADER_BASE_LENGTH);
    //free(ipbuf);

    tcp_header_t* ack_tcp_head =
        assemble_tcp_header(con->local_port, con->remote_port,
                            con->local_seq_num, con->next_ack_num_to_send,
                            ACK, (4 << 8) << 2); // Receive Window = 4 kilobyte
    char* tcpbuf = malloc(TCP_HEADER_BASE_LENGTH);
    serialize_tcp(tcpbuf, ack_tcp_head);
    free(ack_tcp_head);
    write_to_raw_socket(con, tcpbuf, TCP_HEADER_BASE_LENGTH);
    free(tcpbuf);

}

static void send_synack_packet(tcp_conn_t* con) {
    printf("SENDING SYNACK!!\n");

    //ipv4_header_t* synack_ip_head =
    //    assemble_ipv4_header(IPV4_HEADER_BASE_LENGTH, con->local_ipaddr, con->remote_ipaddr);
    //char* ipbuf = malloc(IPV4_HEADER_BASE_LENGTH);
    //serialize_ipv4(ipbuf, synack_ip_head);
    //free(synack_ip_head);
    // dump_ipv4_header(synack_ip_head);
    //write_to_raw_socket(con, ipbuf, IPV4_HEADER_BASE_LENGTH);
    //free(ipbuf);

    tcp_header_t* synack_tcp_head =
        assemble_tcp_header(con->local_port, con->remote_port,
                            con->local_seq_num, con->next_ack_num_to_send,
                            SYNACK, (4 << 8) << 2); // Receive Window = 4 kilobyte
    char* tcpbuf = malloc(TCP_HEADER_BASE_LENGTH);
    serialize_tcp(tcpbuf, synack_tcp_head);
    dump_tcp_header(synack_tcp_head);
    free(synack_tcp_head);
    write_to_raw_socket(con, tcpbuf, TCP_HEADER_BASE_LENGTH);
    free(tcpbuf);

    printf("SENT SYNACK PACKET...\n");
}

tcp_conn_t* fetch_con_by_fd(int fd) {
    tcp_conn_t* aux;
    for (aux = TCPMGR.connections; aux != NULL; aux = aux->next) {
        if (aux->fd == fd) {
            return aux;
        }
    }
    printf("CON NOT FOUND!\n");
    return NULL;
}
