#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <netinet/ip.h> // struct sockaddr_in
#include "buffer_queue.h"

typedef enum { // this enum is mainly used in update_state to simulate the above FSM
    SYN,
    SYNACK,
    FIN,
    FINACK,
    ACK,
    NOTHING,
    I_WANT_TO_SEND,
    I_WANT_TO_CLOSE,
    TIMEOUT_OVER,
    DELETE_TCB
} flag_t;

typedef enum {
    LISTEN,
    SYN_RECEIVED, // when in this state, SYN/ACK was already sent back
    //SYN_SENT, // not needed in this implementation
    ESTABLISHED,
    FIN_WAIT_1, // we want to close the connection and we sent a FIN
    FIN_WAIT_2, // we got the FIN/ACK
    CLOSING, // both parties want to close the connection
    TIME_WAIT,
    CLOSE_WAIT, // peer wants to close the connection
    LAST_ACK, // waiting for final ACK/FIN
    CLOSED
} tcp_conn_state_t;

typedef struct tcp_conn_t {
    int fd;
    uint32_t local_ipaddr;  // this is in host byte order
    uint16_t local_port;    // this is in host byte order

    uint32_t remote_ipaddr; // this is in host byte order
    uint16_t remote_port;   // this is in host byte order

    struct sockaddr_in sin; // sockaddr_in struct for sending. This is in network byte order

    tcp_conn_state_t state;
    tcp_conn_state_t newstate;
    flag_t last_flag_recv;
    flag_t flag_to_be_send;

    uint32_t local_seq_num; // this is our incrementing seq num for sending
    uint32_t last_ack_num_rcvd; // this is as far as we got ACKs

    uint32_t last_ack_num_sent; // outgoing
    uint32_t next_ack_num_to_send; // outgoing

    buffer_queue_t raw_read_queue; // incoming raw
    buffer_queue_t payload_read_queue; //incoming payload

    buffer_queue_t write_queue; //outgoing payload

    struct tcp_conn_t* next;
} tcp_conn_t;

typedef struct {
    bool ready;
    tcp_conn_t* connections; // linked list
} tcp_state_t;


void tcp_manager_initialize();
int tcp_manager_register(int fd, uint32_t ipaddress, uint16_t port);
int tcp_handshake(int fd);
int tcp_manager_read(int fd, void* buf, size_t count);
int tcp_manager_write(int fd, void* buf, size_t count);
int tcp_manager_close(int fd);

tcp_conn_t* fetch_con_by_fd(int fd);
