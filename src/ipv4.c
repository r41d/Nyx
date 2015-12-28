#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h> // inet_ntoa
#include "ipv4.h"

int get_header_size(const ipv4_header_t* header) {
	return header->length << 2;
}

void serialize_ipv4(char* buf, const ipv4_header_t* header) {
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
	int options_len = header->ihl*4 - 20;
	if (options_len > 0) {
		printf("DEBUG: serialize_ipv4: header > 20, buf should be 60 bytes big.\n");
		// buf should be 60 bytes
		memcpy(&buf[20], header->options, options_len);
	}
}

void deserialize_ipv4(ipv4_header_t* header, const char* buf) {
	header->version = buf[0] >> 4;
	if (header->version != 4)
		printf("MALFORMED IP HEADER: VERSION IS NOT 4!\n");
	header->ihl = buf[0] & 0b1111;
	if (header->ihl*4 < 20)
		printf("MALFORMED IP HEADER: HEADER IS SUPPOSEDLY SMALLER THAN 20 BYTES!\n");
	header->tos = buf[1];
	header->length = ntohs( *((uint16_t*) &buf[2]));
	if (header->length < 40 || header->length > 1500)
		printf("MALFORMED IP HEADER: LENGTH IS NOT IN {40,...,1500}!\n");
	header->identification = ntohs( *((uint16_t*) &buf[4]));
	header->flag_0 = (buf[6] >> 7) & 0b1;
	if (header->flag_0 != 0)
		printf("MALFORMED IP HEADER: FLAG_0 IS NOT 0!\n");
	header->flag_df = (buf[6] >> 6) & 0b1;
	header->flag_mr = (buf[6] >> 5) & 0b1;
	header->fragment_offset = ((buf[6] & 0b00011111) << 8) + buf[7]; // ???
	header->ttl = buf[8];
	if (header->ttl < 0)
		printf("MALFORMED IP HEADER: TTL IS NEGATIVE!\n");
	header->protocol = buf[9];
	if (header->protocol != IPPROTO_TCP)
		printf("MALFORMED IP HEADER: PROTOCOL IS NOT TCP!\n");
	header->checksum = ntohs( *((uint16_t*) &buf[10]));
	header->src_addr = ntohl( *((uint32_t*) &buf[12]));
	header->dest_addr = ntohl( *((uint32_t*) &buf[16]));
	int options_len = header->ihl*4 - 20;
	if (options_len > 0) {
		header->options = (uint32_t*) malloc(options_len);
		memcpy(header->options, &buf[20], options_len);
	}
}

void dump_ipv4_header(ipv4_header_t* header) {
	printf("IPv4 HEADER DUMP:\n");
	printf("IPv4-Version:    %d\n", header->version);
	printf("IPv4-IHL:        %d (%d bytes)\n", header->ihl, header->ihl*4);
	printf("IPv4-TOS:        %d\n", header->tos);
	printf("IPv4-Length:     %d\n", header->length);
	printf("IPv4-Ident:      %d\n", header->identification);
	printf("IPv4-Flag0:      %d\n", header->flag_0);
	printf("IPv4-FlagDF:     %d\n", header->flag_df);
	printf("IPv4-FlagMR:     %d\n", header->flag_mr);
	printf("IPv4-FragOffset: %d\n", header->fragment_offset);
	printf("IPv4-TTL:        %d\n", header->ttl);
	printf("IPv4-Protocol:   %d\n", header->protocol);
	printf("IPv4-Checksum:   %x\n", header->checksum);

	struct in_addr addr;
	addr.s_addr = ntohl(header->src_addr);
	printf("IPv4-SrcAddr:    %s (%08x)\n", inet_ntoa(addr), header->src_addr);

	addr.s_addr = ntohl(header->dest_addr);
	printf("IPv4-DestAddr:   %s (%08x)\n", inet_ntoa(addr), header->dest_addr);


	if (header->ihl*4 > 20) {
    	printf("IPv4-Options:   ");
        for (int i = 0; i < header->ihl*4-20; i+=4) {
            printf(" %08x", header->options[i]);
        }
    }
}

//uint16_t ipv4_checksum(...) {
//
//}

ipv4_header_t* assemble_ipv4_header(uint16_t payload_length, uint32_t src, uint32_t dest) {
	ipv4_header_t* v4header = (ipv4_header_t*) malloc(sizeof(ipv4_header_t));

	static int ident = 0;
	ident += 1;

	v4header->version = 4;
	v4header->ihl = 20/4; // we always have header length 20 for now
	v4header->tos = 0b00000000;
	v4header->length = IPV4_HEADER_BASE_LENGTH + payload_length; // Always filled in by kernel
	v4header->identification = 0 /*ident*/; // Filled in when zero
	v4header->flag_0 = false;
	v4header->flag_df = false;
	v4header->flag_mr = false;
	v4header->fragment_offset = 0b0000000000000;
	v4header->ttl = 54; // Always 54
	v4header->protocol = 6; // 6 = TCP
	v4header->checksum = 0x0; // Always filled in by kernel
	//memcpy(&v4header->src_addr, &src, sizeof(uint32_t));
	v4header->src_addr = 0; // Filled in when zero
	memcpy(&v4header->dest_addr, &dest, sizeof(uint32_t));
	//v4header->optional[40];

	return v4header;
}
