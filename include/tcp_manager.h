#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

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

typedef struct {
    int fd;
    uint32_t remote_ipaddr;
    uint16_t remote_port;
    tcp_conn_state_t stat;
    struct tcp_conn_t* next;
} tcp_conn_t;

typedef struct {
    bool ready;
    tcp_conn_t* connections; // linked list
} tcp_state_t;
