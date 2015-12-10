#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "ipv4.h"

int get_header_size(const ipv4_header_t* header) {
	return header->length << 2;
}

void serialize_ipv4 (char* buf, const ipv4_header_t* header) {
	buf[0] = header->version << 4 | header->ihl;
	buf[1] = header->tos;
	buf[2] = htons(header->length);
	buf[4] = htons(header->identification);
	buf[6] = htons(header->flag_0 << 15
 	             | header->flag_df << 14
 	             | header->flag_mr << 13
 	             | header->fragment_offset);
	buf[8] = header->ttl;
	buf[9] = htons(header->protocol);
	buf[10] = htons(header->checksum);
	buf[12] = htonl(header->src_addr);
	buf[16] = htonl(header->dest_addr);
	// optional
}

void deserialize_ipv4 (ipv4_header_t* header, const char* buf) {
	header->version = buf[0] >> 4;
	header->ihl = buf[0] & 0b1111;
	header->tos = buf[1];
	header->length = ntohs( *((uint16_t*) &buf[2]));
	header->identification = ntohs( *((uint16_t*) &buf[4]));

	header->flag_0 = (buf[6] >> 7) & 0b1;
	header->flag_df = (buf[6] >> 6) & 0b1;
	header->flag_mr = (buf[6] >> 5) & 0b1;
	header->fragment_offset = (buf[6] & 0b00011111) << 8 + buf[7]; // ???
	header->ttl = buf[8];
	header->protocol = buf[9];
	header->checksum = ntohs( *((uint16_t*) &buf[10]));
	header->src_addr = ntohl( *((uint32_t*) &buf[12]));
	header->dest_addr = ntohl( *((uint32_t*) &buf[16]));

	//if () {
	//	int opt_len = ...
	//	memcpy(&(header->optional), &buf[20], opt_len);
	//}
}

void dump_ipv4_header (ipv4_header_t* header) {
	printf("IPv4 HEADER DUMP:\n");
	printf("IPv4-Version:    %d\n", header->version);
	printf("IPv4-IHL:        %d\n", header->ihl);
	printf("IPv4-TOS:        %d\n", header->tos);
	printf("IPv4-Length:     %d\n", header->length);
	printf("IPv4-Ident:      %d\n", header->identification);
	printf("IPv4-Flag0:      %d\n", header->flag_0);
	printf("IPv4-FlagDF:     %d\n", header->flag_df);
	printf("IPv4-FlagMR:     %d\n", header->flag_mr);
	printf("IPv4-FragOffset: %d\n", header->fragment_offset);
	printf("IPv4-TTL:        %d\n", header->ttl);
	printf("IPv4-Protocol:   %d\n", header->protocol);
	printf("IPv4-Checksum:   %d\n", header->checksum);
	printf("IPv4-SrcAddr:    %d\n", header->src_addr);
	printf("IPv4-DestAddr:   %d\n", header->dest_addr);
	//uint8_t optional[40]; // 40 additional bytes at maximum
}


ipv4_header_t* assemble_ipv4_header_t(uint16_t payload_length, uint32_t src, uint32_t dest) {
	ipv4_header_t* v4header = (ipv4_header_t*) malloc(sizeof(ipv4_header_t));

	static int ident = 0;
	ident += 1;

	v4header->version = 4;
	v4header->ihl = 20/4; // we always have header length 20 for now
	v4header->tos = 0b00000000;
	v4header->length = payload_length;
	v4header->identification = ident;
	v4header->flag_0 = false;
	v4header->flag_df = false;
	v4header->flag_mr = false;
	v4header->fragment_offset = 0b0000000000000;
	v4header->ttl = 54; // Always 54
	v4header->protocol = 6; // 6 = TCP
	v4header->checksum = 0x0; // this is getting ignored anyway
	memcpy(&v4header->src_addr, &src, sizeof(uint32_t));
	memcpy(&v4header->dest_addr, &dest, sizeof(uint32_t));
	//v4header->optional[40];

	return v4header;
}
