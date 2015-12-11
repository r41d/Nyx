#pragma once

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

/* from RFC 793
                             +---------+ ---------\      active OPEN
                             |  CLOSED |            \    -----------
                             +---------+<---------\   \   create TCB
                               |     ^              \   \  snd SYN
                 passive OPEN  |     |   CLOSE        \   \
                 ------------  |     | ----------       \   \
                 create TCB    |     | delete TCB         \   \
                               V     |                      \   \
                             +---------+            CLOSE    |    \
                             |  LISTEN |          ---------- |     |
                             +---------+          delete TCB |     |
                  rcv SYN      |     |     SEND              |     |
                 -----------   |     |    -------            |     V
+---------+      snd SYN,ACK  /       \   snd SYN          +---------+
|         |<------------------         ------------------->|         |
|   SYN   |                    rcv SYN                     |   SYN   |
|   RCVD  |<-----------------------------------------------|   SENT  |
|         |                    snd ACK                     |         |
|         |-------------------          -------------------|         |
+---------+   rcv ACK of SYN  \       /  rcv SYN,ACK       +---------+
  |           --------------   |     |   -----------
  |                  x         |     |     snd ACK
  |                            V     V
  |  CLOSE                   +---------+
  | -------                  |  ESTAB  |
  | snd FIN                  +---------+
  |                   CLOSE    |     |    rcv FIN
  V                  -------   |     |    -------
+---------+          snd FIN  /       \   snd ACK          +---------+
|  FIN    |<------------------          ------------------>|  CLOSE  |
| WAIT-1  |-------------------                             |   WAIT  |
+---------+          rcv FIN  \                            +---------+
  | rcv ACK of FIN   -------   |                            CLOSE  |
  | --------------   snd ACK   |                           ------- |
  V        x                   V                           snd FIN V
+---------+                  +---------+                   +---------+
|FINWAIT-2|                  | CLOSING |                   | LAST-ACK|
+---------+                  +---------+                   +---------+
  |                rcv ACK of FIN |                 rcv ACK of FIN |
  |  rcv FIN       -------------- |    Timeout=2MSL -------------- |
  |  -------              x       V    ------------        x       V
   \ snd ACK                 +---------+delete TCB         +---------+
    ------------------------>|TIME WAIT|------------------>| CLOSED  |
                             +---------+                   +---------+
*/

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

typedef struct {
    int fd;
    uint32_t remote_ipaddr;
    uint16_t remote_port;
    tcp_conn_state_t state;
    tcp_conn_state_t newstate;
    flag_t last_flag_recv;
    flag_t flag_to_be_send;

    struct tcp_conn_t* next;
} tcp_conn_t;

typedef struct {
    bool ready;
    tcp_conn_t* connections; // linked list
} tcp_state_t;
