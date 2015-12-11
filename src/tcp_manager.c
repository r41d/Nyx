#include <stdbool.h>
#include <string.h>
#include <unistd.h> // read()
#include <sys/types.h> // ssize_t
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

    tcp_conn_t* con = malloc(sizeof(tcp_conn_t));

    con->fd = fd;
    con->remote_ipaddr = ipaddress;
    con->remote_port = port;

    con->state = LISTEN;
    con->newstate = LISTEN; // no transition yet

    con->last_flag_recv = NOTHING;
    con->flag_to_be_send = NOTHING;

    con->read_queue = malloc(sizeof(buf_queue_t));
    memset(con->read_queue, 0, sizeof(buf_queue_t));
    // buf, buflen and next are all 0
    con->write_queue = malloc(sizeof(buf_queue_t));
    memset(con->write_queue, 0, sizeof(buf_queue_t));

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

    tcp_conn_t* con = fetch_con_by_fd(fd);





    return -1;
}

int tcp_manager_write(int fd, void* buf, size_t count) {
    tcp_conn_t* con = fetch_con_by_fd(fd);


    void* buffer = malloc(count);
    ssize_t bytes_rcvd = read(con->fd, buffer, count);



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


// BUFFER QUEUE FUNCTIONS

/*
    Enqueues a buffer to the end of a buf_queue_t
*/
static void enqueue_buf(buf_queue_t* q, void* buf, size_t buflen) {
    buf_queue_t* aux;
    for (aux = q; aux->next != NULL; aux = aux->next);
    aux->next = malloc(sizeof(buf_queue_t));
    aux->next->buf = buf;
    aux->next->buflen = buflen;
    aux->next->next = NULL;
}

/*
  Get the numnber of total bytes present in thet
  specified buf_queue_t
*/
static size_t buf_queue_size(buf_queue_t* q) {
    size_t size;
    buf_queue_t* aux;
    for (aux = q; aux != NULL; aux = aux->next) {
        size += aux->buflen;
    }
    return size;
}

/*
    dequeue up to n bytes from a given buf_queue_t**  <--  DOUBLE POINTER!
*/
static size_t dequeue_bytes(buf_queue_t** queue, void* resultbuf, size_t n) {
    buf_queue_t* q = *queue; // for easier usage in below code

    if (q->buflen > n) {
        memcpy(resultbuf, q->buf, n);

        // shrink the entry in the queue
        void* smallerbuf = malloc(q->buflen - n);
        memcpy(smallerbuf, q->buf + n, q->buflen - n);
        free(q->buf);
        q->buf = smallerbuf; // assign new smaller buffer
        q->buflen -= n; // new smaller size

        return n; // we had n bytes, to we returned n bytes, gg
    } else { // first buf isn't big enough to satisfy n bytes
        // we know that q->buflen < n and that size(resultbuf) >= n
        memcpy(resultbuf, q->buf, q->buflen); // write first buffer to resultbuf

        buf_queue_t* aux = q; // later to be used for freeing
        queue = &(q->next); // here the magic happens
        // hier biegen wir den pointer aus dem originalen struct um

        size_t already_read = aux->buflen;
        // free buf which we deleted from the front
        free(aux->buf);
        free(aux);

        // call dequeue_bytes again with offset on resultbuf
        dequeue_bytes(queue, resultbuf+already_read, n - already_read);
    }

//    if (q.len > n) {
//        erste n bytes von q.buf zurückgeben und von q.buf entfernen und q.len verringern
//    } else {
//        newbuf = q.buf // malloc + memcpy
//        q = q.next // POINTER SCHEISS BEACHTEN und free() aufrufen
//        // hier biegen wir den pointer aus dem originalen struct um
//        newbuf += dequeue_bytes(q, n - bereits gelesen)
//    }
    return -1;

}
