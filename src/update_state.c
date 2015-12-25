#include "tcp_manager.h"
#include "update_state.h"

static void update_listen(tcp_conn_t* conn) {
    if (conn->last_flag_recv == SYN) {
        conn->flag_to_be_send = SYNACK;
        conn->newstate = SYN_RECEIVED;
    }
    // I_WANT_TO_SEND -> SYN_SENT // not necessary to implement this
}

static void update_syn_received(tcp_conn_t* conn) {
    if (conn->last_flag_recv == I_WANT_TO_CLOSE) {
        conn->flag_to_be_send = FIN;
        conn->newstate = FIN_WAIT_1;
    } else if (conn->last_flag_recv == ACK) { // ACK of SYN
        conn->flag_to_be_send = NOTHING;
        conn->newstate = ESTABLISHED;
    }
}

static void update_established(tcp_conn_t* conn) {
    if (conn->last_flag_recv == I_WANT_TO_CLOSE) {
        conn->flag_to_be_send = FIN;
        conn->newstate = FIN_WAIT_1;
    } else if (conn->last_flag_recv == FIN
        || conn->last_flag_recv == FINACK) { // need this cuz Linux bundles the terminating ACK with a FIN
        conn->flag_to_be_send = ACK;
        conn->newstate = CLOSE_WAIT;
    }
}

static void update_fin_wait_1(tcp_conn_t* conn) {
    if (conn->last_flag_recv == ACK) { // ACK of FIN
        conn->flag_to_be_send = NOTHING;
        conn->newstate = FIN_WAIT_2;
    } else if (conn->last_flag_recv == FIN) {
        conn->flag_to_be_send = ACK;
        conn->newstate = CLOSING;
    } else if (conn->last_flag_recv == FINACK) {
        // fast connection closing
        // skip FIN_WAIT_2 when we get ACK+FIN
        conn->flag_to_be_send = ACK;
        conn->newstate = TIME_WAIT;
    }
}

static void update_fin_wait_2(tcp_conn_t* conn) {
    if (conn->last_flag_recv == FIN) {
        conn->flag_to_be_send = ACK;
        conn->newstate = TIME_WAIT;
    }
}

static void update_closing(tcp_conn_t* conn) {
    if (conn->last_flag_recv == ACK) { // ACK of FIN
        conn->flag_to_be_send = NOTHING;
        conn->newstate = TIME_WAIT;
    }
}

static void update_time_wait(tcp_conn_t* conn) {
    if (conn->last_flag_recv == TIMEOUT_OVER) {
        conn->flag_to_be_send = DELETE_TCB;
        conn->newstate = CLOSED;
    }
}

static void update_close_wait(tcp_conn_t* conn) {
    if (conn->last_flag_recv == I_WANT_TO_CLOSE) {
        conn->flag_to_be_send = FIN;
        conn->newstate = LAST_ACK;
    }
}

static void update_last_ack(tcp_conn_t* conn) {
    if (conn->last_flag_recv == ACK) { // ACK of FIN
        conn->flag_to_be_send = NOTHING;
        conn->newstate = CLOSED;
    }
}

static void update_closed(tcp_conn_t* conn) {
    // ...
}

void update_state(tcp_conn_t* conn) {
    switch (conn->state) {
        case LISTEN:
            update_listen(conn);
            break;
        case SYN_RECEIVED:
            update_syn_received(conn);
            break;
        case ESTABLISHED:
            update_established(conn);
            break;
        case FIN_WAIT_1:
            update_fin_wait_1(conn);
            break;
        case FIN_WAIT_2:
            update_fin_wait_2(conn);
            break;
        case CLOSING:
            update_closing(conn);
            break;
        case TIME_WAIT:
            update_time_wait(conn);
            break;
        case CLOSE_WAIT:
            update_close_wait(conn);
            break;
        case LAST_ACK:
            update_last_ack(conn);
            break;
        case CLOSED:
            update_closed(conn);
            break;
        default: // must not happen
            break;
    }
}
