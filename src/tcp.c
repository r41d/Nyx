#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <arpa/inet.h>
#include "tcp.h"

void serialize_tcp (char* buf, const tcp_header_t* header) {
    buf[0] = htons(header->src_port);
    buf[2] = htons(header->dest_port);
    buf[4] = htonl(header->seq_num);
    buf[8] = htonl(header->ack_num);
    buf[12] = ( header->data_offset );
    buf[13] = 0b00000000
             | header->urg << 5
             | header->ack << 4
             | header->psh << 3
             | header->rst << 2
             | header->syn << 1
             | header->fin;
    buf[14] = htons(header->window);
    buf[16] = htons(header->checksum);
    buf[18] = htons(header->urgent_pointer);
	// optional
}

void deserialize_tcp (tcp_header_t* header, const char* buf) {
    header->src_port = ntohs(*((uint16_t*) &buf[0]));
    header->dest_port = ntohs(*((uint16_t*) &buf[2]));
    header->seq_num = ntohl(*((uint32_t*) &buf[4]));
    header->ack_num = ntohl(*((uint32_t*) &buf[8]));
    header->data_offset = buf[12] >> 4;
    header->urg = (buf[13] & 0b00100000) >> 5;
    header->ack = (buf[13] & 0b00010000) >> 4;
    header->psh = (buf[13] & 0b00001000) >> 3;
    header->rst = (buf[13] & 0b00000100) >> 2;
    header->syn = (buf[13] & 0b00000010) >> 1;
    header->fin = (buf[13] & 0b00000001);
    header->window = ntohs(*((uint16_t*) &buf[14]));
    header->checksum = ntohs(*((uint16_t*) &buf[16]));
    header->urgent_pointer = ntohs(*((uint16_t*) &buf[18]));
}

void dump_tcp_header (tcp_header_t* header) {
	printf("TCP HEADER DUMP:\n");
	printf("TCP-src_port:    %d\n", header->src_port);
	printf("TCP-dest_port:   %d\n", header->dest_port);
	printf("TCP-seq_num:     %d\n", header->seq_num);
	printf("TCP-ack_num:     %d\n", header->ack_num);
	printf("TCP-data_offset: %d\n", header->data_offset);
	printf("TCP-urg:         %d\n", header->urg);
	printf("TCP-ack:         %d\n", header->ack);
	printf("TCP-psh:         %d\n", header->psh);
	printf("TCP-rst:         %d\n", header->rst);
	printf("TCP-syn:         %d\n", header->syn);
	printf("TCP-fin:         %d\n", header->fin);
	printf("TCP-window:      %d\n", header->window);
	printf("TCP-checksum:    %x\n", header->checksum);
	printf("TCP-urgent_ptr:  %d\n", header->urgent_pointer);
    //	uint32_t* options; // zero or more 32-bit-words
}

uint16_t tcp_checksum(const char* buf, uint32_t src, uint32_t dest, uint16_t len) {
  uint32_t sum = 0;

  // Pseudo header
  src  = htonl(src);
  dest = htonl(dest);

  sum += (uint16_t) src;
  sum += (uint16_t) (src >> 16);
  sum += (uint16_t) dest;
  sum += (uint16_t) (dest >> 16);
  sum += htons(6);
  sum += htons(len);

  // Header and data (16 bit words at a time)
  size_t i;
  for (i = 0; i < (len >> 1); ++i) {
    if (i != 8) sum += *(uint16_t *) &buf[i << 1];
  }

  // One remaining byte in data?
  if (len & 1 != 0) sum += (uint16_t) buf[len - 1];

  // Folding carry values
  sum = (sum & 0xFFFF) + (sum >> 16);
  sum = (sum & 0xFFFF) + (sum >> 16);

  return htons((uint16_t) ~sum);
}
