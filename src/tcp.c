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
    int options_len = header->data_offset*4 - 20;
    if (options_len > 0) {
        printf("DEBUG: serialize_tcp: header > 20, buf should be 60 bytes big.\n");
        // buf should be 60 bytes
        memcpy(&buf[20], header->options, options_len);
    }
}

void deserialize_tcp (tcp_header_t* header, const char* buf) {
    header->src_port = ntohs(*((uint16_t*) &buf[0]));
    header->dest_port = ntohs(*((uint16_t*) &buf[2]));
    header->seq_num = ntohl(*((uint32_t*) &buf[4]));
    header->ack_num = ntohl(*((uint32_t*) &buf[8]));
    header->data_offset = buf[12] >> 4;
    if (header->data_offset*4 < 20)
        printf("MALFORMED IP HEADER: HEADER IS SUPPOSEDLY SMALLER THAN 20 BYTES!\n");
    header->urg = (buf[13] & 0b00100000) >> 5;
    header->ack = (buf[13] & 0b00010000) >> 4;
    header->psh = (buf[13] & 0b00001000) >> 3;
    header->rst = (buf[13] & 0b00000100) >> 2;
    header->syn = (buf[13] & 0b00000010) >> 1;
    header->fin = (buf[13] & 0b00000001);
    header->window = ntohs(*((uint16_t*) &buf[14]));
    header->checksum = ntohs(*((uint16_t*) &buf[16]));
    header->urgent_pointer = ntohs(*((uint16_t*) &buf[18]));
    int options_len = header->data_offset*4 - 20;
    if (options_len > 0) {
        header->options = (uint32_t*) malloc(options_len);
        memcpy(header->options, &buf[20], options_len);
    }
}

void dump_tcp_header (tcp_header_t* header) {
	printf("TCP HEADER DUMP:\n");
	printf("TCP-src_port:    %d\n", header->src_port);
	printf("TCP-dest_port:   %d\n", header->dest_port);
	printf("TCP-seq_num:     %x\n", header->seq_num);
	printf("TCP-ack_num:     %x\n", header->ack_num);
	printf("TCP-data_offset: %d (%d bytes)\n",header->data_offset,header->data_offset*4);
	printf("TCP-urg:         %d\n", header->urg);
	printf("TCP-ack:         %d\n", header->ack);
	printf("TCP-psh:         %d\n", header->psh);
	printf("TCP-rst:         %d\n", header->rst);
	printf("TCP-syn:         %d\n", header->syn);
	printf("TCP-fin:         %d\n", header->fin);
	printf("TCP-window:      %d\n", header->window);
	printf("TCP-checksum:    %x\n", header->checksum);
    printf("TCP-urgent_ptr:  %d\n", header->urgent_pointer);
    if (header->data_offset*4 > 20) {
        printf("TCP-options:    ");
        for (int i = 0; i < header->data_offset*4-20; i+=4) {
            printf(" %08x", header->options[i]);
        }
    }
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
  if ((len & 1) != 0)
    sum += (uint16_t) buf[len - 1];

  // Folding carry values
  sum = (sum & 0xFFFF) + (sum >> 16);
  sum = (sum & 0xFFFF) + (sum >> 16);

  return htons((uint16_t) ~sum);
}

tcp_header_t* assemble_tcp_header(uint16_t src_port,
                                  uint16_t dest_port,
                                  uint32_t seq_num,
                                  uint32_t ack_num,
                                  flag_t flags_to_be_send,
                                  uint16_t window) {

    // allocate a new header
    tcp_header_t* header = malloc(sizeof(tcp_header_t));
    memset(header, 0, sizeof(tcp_header_t));

    header->src_port = src_port;
    header->dest_port = dest_port;
    header->seq_num = seq_num;
    header->ack_num = ack_num;
    header->data_offset = TCP_HEADER_BASE_LENGTH >> 2;
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
    header->urgent_pointer = 0; // too advanced, we don't use URG flag
    char* tmp = malloc(TCP_HEADER_BASE_LENGTH);
    serialize_tcp(tmp, header);
    header->checksum = tcp_checksum(tmp, src_port, dest_port, TCP_HEADER_BASE_LENGTH);
    free(tmp);
    return header;
}
