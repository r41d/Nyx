#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h> // ssize_t
#include <netinet/in.h> // ntohs()
#include "api.h"

int main (void) {
    // int fd = nyx_accept(htons(4711), htonl(3232235521)); // Listen on port 4711 and IP 192.168.0.1
    //int fd = nyx_accept(4711, 2130706433); // Listen on port 4711 and IP 127.0.0.1
    int fd = nyx_accept(4711, 2130706434); // Listen on port 4711 and IP 127.0.0.2
    const int buf_size = 1024;
    char buf[buf_size];
    while (nyx_read(fd,&buf,buf_size) > 0) {
        //buf[buf_size-1] = '\0'; // ensure 0 termination
        //printf("%s\n", buf);
        // TODO: write data to file
    }
    nyx_close(fd);
    printf("Connection terminated or error\n");
    return 0;
}
