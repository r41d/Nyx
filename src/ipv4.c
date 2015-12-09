#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "ipv4.h"

/*
typedef struct { // groups of 4 bytes each

	uint8_t version; // 4 bits
	uint8_t ihl; // 4 bits
	uint8_t tos; // 8 bits
	uint16_t length; // 16 bits

	uint16_t identification; // 16 bits
	_Bool flag_0;  //reserved, must be 0 (1 bit)
	_Bool flag_df; // Don't Fragment (1 bit)
	_Bool flag_mr; // More Fragments (1 bit)
	uint16_t fragment_offset; // 13 bits

	uint8_t ttl; // 8 bits
	uint8_t protocol; // 8 bits
	uint16_t checksum; // 16 bits

	uint32_t src_addr; // 32 bits

	uint32_t dest_addr; // 32 bits

	uint8_t optional[40]; // 40 additional bytes at maximum

} ipv4_header;
*/


int get_header_size(const ipv4_header* header) {
	// return ... ;
}

void serialize_ipv4 (char* buf, const ipv4_header* header) {
	uint8_t version_ihl = header->version << 4 | header->ihl;
	memcpy(&buf[0], &version_ihl, sizeof(uint8_t));
	memcpy(&buf[1], &header->tos, sizeof(uint8_t));
	memcpy(&buf[2], &header->length, sizeof(uint16_t));
	memcpy(&buf[4], &header->identification, sizeof(uint16_t));
	uint16_t flags_fragment = header->flag_0 << 15
 	                        | header->flag_df << 14
 	                        | header->flag_mr << 13
 	                        | header->fragment_offset;
	memcpy(&buf[6], &flags_fragment, sizeof(uint16_t));
	memcpy(&buf[8], &header->ttl, sizeof(uint8_t));
	memcpy(&buf[9], &header->protocol, sizeof(uint8_t));
	memcpy(&buf[10], &header->checksum, sizeof(uint16_t));
	memcpy(&buf[12], &header->src_addr, sizeof(uint32_t));
	memcpy(&buf[16], &header->dest_addr, sizeof(uint32_t));
	// optional
}

void deserialize_ipv4 (ipv4_header* header, const char* buf) {
	header->version = buf[0] >> 4;
	header->ihl = buf[0] % (1<<4);
	memcpy(&header->tos, &buf[1], sizeof(uint8_t));
	memcpy(&header->length, &buf[2], sizeof(uint16_t));
	memcpy(&header->identification, &buf[4], sizeof(uint16_t));

	header->flag_0 = (buf[6] & 0b10000000) >> 7 ;
	header->flag_df; (buf[6] & 0b01000000) >> 6 ; // Don't Fragment (1 bit)
	header->flag_mr; (buf[6] & 0b00100000) >> 5 ; // More Fragments (1 bit)
	header->fragment_offset = (buf[6] & 0b00011111) << 8 + buf[7]; // 13 bits
	memcpy(&header->ttl, &buf[8], sizeof(uint8_t));
	memcpy(&header->protocol, &buf[9], sizeof(uint8_t));
	memcpy(&header->checksum, &buf[10], sizeof(uint16_t));
	memcpy(&header->src_addr, &buf[12], sizeof(uint32_t));
	memcpy(&header->dest_addr, &buf[16], sizeof(uint32_t));

	//if () {
	//	int opt_len = ...
	//	memcpy(&(header->optional), &buf[20], opt_len);
	//}
}

ipv4_header* assemble_ipv4_header(uint16_t payload_length, uint32_t src, uint32_t dest) {
	ipv4_header* v4header = (ipv4_header*) malloc(sizeof(ipv4_header));

	static int ident = 0;
	ident += 1;

	v4header->version = 4;
	v4header->ihl = 20/4; // we always have header length 20 for now
	v4header->tos = 0b00000000;
	v4header->length = payload_length;
	v4header->identification = ident;
	v4header->flag_0 = 0b0;
	v4header->flag_df = 0b0;
	v4header->flag_mr = 0b0;
	v4header->fragment_offset = 0b0000000000000;
	v4header->ttl = 54; // Always 54
	v4header->protocol = 6; // 6 = TCP
	v4header->checksum = 0x0; // this is getting ignored anyway
	memcpy(&v4header->src_addr, &src, sizeof(uint32_t));
	memcpy(&v4header->dest_addr, &dest, sizeof(uint32_t));
	//v4header->optional[40];

	return v4header;
}
