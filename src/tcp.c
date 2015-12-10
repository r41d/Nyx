#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "tcp.h"

void serialize_tcp (char* buf, const tcp_header_t* header) {
    memcpy(&buf[0], &header->src_port, sizeof(uint16_t));
    memcpy(&buf[2], &header->dest_port, sizeof(uint16_t));
    memcpy(&buf[4], &header->seq_num, sizeof(uint32_t));
    memcpy(&buf[8], &header->ack_num, sizeof(uint32_t));
    uint8_t dat_off = header->data_offset << 4;
    memcpy(&buf[12], &dat_off, sizeof(uint8_t));
    uint8_t flags = 0b00000000
                  | header->urg << 5
                  | header->ack << 4
                  | header->psh << 3
                  | header->rst << 2
                  | header->syn << 1
                  | header->fin;
    memcpy(&buf[13], &flags, sizeof(uint8_t));
    memcpy(&buf[14], &header->window, sizeof(uint16_t));
    memcpy(&buf[16], &header->checksum, sizeof(uint16_t));
    memcpy(&buf[18], &header->urgent_pointer, sizeof(uint16_t));
	// optional
}

void deserialize_tcp (tcp_header_t* header, const char* buf) {
    memcpy(&header->src_port, &buf[0], sizeof(uint16_t));
    memcpy(&header->dest_port, &buf[2], sizeof(uint16_t));
    memcpy(&header->seq_num, &buf[4], sizeof(uint32_t));
    memcpy(&header->ack_num, &buf[8], sizeof(uint32_t));
    header->data_offset = buf[12] >> 4;
    header->urg = (buf[13] & 0b00100000) >> 5;
    header->ack = (buf[13] & 0b00010000) >> 4;
    header->psh = (buf[13] & 0b00001000) >> 3;
    header->rst = (buf[13] & 0b00000100) >> 2;
    header->syn = (buf[13] & 0b00000010) >> 1;
    header->fin = (buf[13] & 0b00000001);
    memcpy(&header->window, &buf[14], sizeof(uint16_t));
    memcpy(&header->checksum, &buf[16], sizeof(uint16_t));
    memcpy(&header->urgent_pointer, &buf[18], sizeof(uint16_t));
}

void dump_tc_header (tcp_header_t* header) {
	printf("TCP HEADER DUMP:\n");
	printf("TCP-src_port:    %d", header->src_port);
	printf("TCP-dest_port:   %d", header->dest_port);
	printf("TCP-seq_num:     %d", header->seq_num);
	printf("TCP-ack_num:     %d", header->ack_num);
	printf("TCP-data_offset: %d", header->data_offset);
	printf("TCP-urg:         %d", header->urg);
	printf("TCP-ack:         %d", header->ack);
	printf("TCP-psh:         %d", header->psh);
	printf("TCP-rst:         %d", header->rst);
	printf("TCP-syn:         %d", header->syn);
	printf("TCP-fin:         %d", header->fin);
	printf("TCP-window:      %d", header->window);
	printf("TCP-checksum:    %d", header->checksum);
	printf("TCP-urgent_ptr:  %d", header->urgent_pointer);
    //	uint32_t* options; // zero or more 32-bit-words
}

uint16_t tcp_checksum(const char* buf, uint32_t src, uint32_t dest, uint16_t len) {
  uint16_t sum = 0;

  // Pseudo header
  sum += ~((uint16_t) src);
  sum += ~((uint16_t) (src >> 16));
  sum += ~((uint16_t) dest);
  sum += ~((uint16_t) (dest >> 16));
  sum += ~((uint16_t) 6);
  sum += ~len;

  // Header and data (16 bit words at a time)
  size_t i;
  for (i = 0; i < (len >> 1); ++i) {
    if (i != 8) sum += ~*((uint16_t *) &buf[i]);
  }

  // One remaining byte in data?
  if (len & 1 != 0)
    sum += ~((uint16_t) buf[len - 1]);

  return ~sum;
}
