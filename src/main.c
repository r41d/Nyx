#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <tcp.h>
#include <ipv4.h>

int main (void) {
  int sock = socket(AF_INET, SOCK_RAW, 6);

  if (sock < 0) {
    printf("Error while opening socket:");
    if (errno == EPERM) {
      printf("Permission denied.\n");
    } else {
      printf("%d\n", errno);
    }

    return 1;
  }

  printf("Ready\n");

  ssize_t size;
  char buf[513];
  buf[512] = 0;

  ipv4_header_t hdr_ip;
  tcp_header_t hdr_tcp;

  memset(&hdr_ip, 0, sizeof(ipv4_header_t));
  memset(&hdr_tcp, 0, sizeof(tcp_header_t));

  while ((size = read(sock, &buf, 512)) > 0) {
    // void deserialize_ipv4 (ipv4_header_t* hdr_ip, const char* buf)
    deserialize_ipv4(&hdr_ip, &buf);
    printf("Received something (%d bytes):\n", size);

    size_t i;
    for (i = 0; i < size; ++i) {
      printf("%02x", *((uint8_t*) &buf[i]));
    }

    printf("\n");
    printf("IP version: %d\n", hdr_ip.version);

    size_t offset = (size_t) (hdr_ip.ihl << 2);

    dump_ipv4_header(&hdr_ip);

    printf("Offset: %d %02X\n", offset, offset);

    deserialize_tcp(&hdr_tcp, &buf[offset]);
    dump_tcp_header(&hdr_tcp);

    printf("\n");
  }

  close(sock);
  return 0;
}
