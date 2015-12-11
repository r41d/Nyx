#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <arpa/inet.h>
#include "tcp.h"
#include "tcp_manager.h"

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

/*
typedef struct { // groups of 4 bytes each

	uint16_t src_port; // 16 bit
	uint16_t dest_port; // 16 bit

	uint32_t seq_num; // 32 bit

	uint32_t ack_num; // 32 bit

	uint8_t data_offset : 4; // 4 bit
	// 4 reserved bits
	// 2 reserved bits
	bool urg : 1; // 1 bit
	bool ack : 1; // 1 bit
	bool psh : 1; // 1 bit
	bool rst : 1; // 1 bit
	bool syn : 1; // 1 bit
	bool fin : 1; // 1 bit
	uint16_t window; // 16 bit

	uint16_t checksum; // 16 bit
	uint16_t urgent_pointer; // 16 bit

	uint32_t* options; // zero or more 32-bit-words

} tcp_header_t;
*/

tcp_header_t* assemble_tcp_header(uint16_t src_port,
                                  uint16_t dest_port,
                                  uint32_t seq_num,
                                  uint32_t ack_num,
                                  flag_t flags_to_be_send,
                                  uint16_t window) {
    tcp_header_t* header = (tcp_header_t*) malloc(sizeof(tcp_header_t));

    header->src_port = src_port;
    header->dest_port = dest_port;
    header->seq_num = seq_num;
    header->ack_num = ack_num;
    switch (flags_to_be_send) {
        case SYN:
            header->syn = 1;
            break;
        case SYNACK:
            header->syn = header->ack = 1;
            break;
        case FIN:
            header->fin = 1;
            break;
        case FINACK:
            header->fin = header->ack = 1;
            break;
        case ACK:
            header->ack = 1;
            break;
        default: // everything else doesn't matter for us
            break;
    }
    header->window = window;
    // der Rest fehlt noch...
}
