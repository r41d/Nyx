#include <stdio.h>
#include <fcntl.h> // fcntl()
#include "tcp_manager.h"

int write_to_raw_socket(tcp_conn_t* con, void* datagram, size_t dgram_len) {
    int n;
    if ((n=sendto(con->fd,                   // our socket
              datagram,                      // the buffer containing headers and data
              dgram_len,                     // total length of our datagram
              0,                             // routing flags, normally always 0
              (struct sockaddr *) &con->sin, // socket addr, just like in
              sizeof(con->sin))) < 0)        // a normal send()
        printf("sendto() error!!!\n");
    else
        printf("sendto() success\n");

    return n;
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
int fd_set_blocking(int fd, int blocking) {
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
