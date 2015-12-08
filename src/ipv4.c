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

void serialize (char* buf, const ipv4_header* header) {
	memcpy(&buf[0], &(header->version << 4 | header->ihl), sizeof(uint8_t));
	memcpy(&buf[1], &(header->tos), sizeof(uint8_t));
	memcpy(&buf[2], &(header->length), sizeof(uint16_t));
	memcpy(&buf[4], &(header->identification), sizeof(uint16_t));
	memcpy(&buf[6], &(header->flag_0 << 15
	                  | header->flag_df << 14
	                  | header->flag_mr << 13
	                  | header->fragment_offset), sizeof(uint16_t));
	memcpy(&buf[8], &(header->ttl), sizeof(uint8_t));
	memcpy(&buf[9], &(header->protocol), sizeof(uint8_t));
	memcpy(&buf[10], &(header->checksum), sizeof(uint16_t));
	memcpy(&buf[12], &(header->src_addr), sizeof(uint32_t));
	memcpy(&buf[16], &(header->dest_addr), sizeof(uint32_t));
	// optional
}

void deserialize (ipv4_header* header, const char* buf) {
	header->version = buf[0] >> 4;
	header->ihl = buf[0] % (1<<4);
	memcpy(&(header->tos), &buf[1], sizeof(uint8_t));
	memcpy(&(header->length), &buf[2], sizeof(uint16_t));
	memcpy(&(header->identification), &buf[4], sizeof(uint16_t));
	
	header->flag_0 = buf[6] & 0b10000000;
	header->flag_df; buf[6] & 0b01000000; // Don't Fragment (1 bit)
	header->flag_mr; buf[6] & 0b00100000; // More Fragments (1 bit)
	header->fragment_offset = (buf[6] & 0b00011111) << 8 + buf[7]; // 13 bits
	memcpy(&(header->ttl), &buf[8], sizeof(uint8_t));
	memcpy(&(header->protocol), &buf[9], sizeof(uint8_t));
	memcpy(&(header->checksum), &buf[10], sizeof(uint16_t));
	memcpy(&(header->src_addr), &buf[12], sizeof(uint32_t));
	memcpy(&(header->dest_addr), &buf[16], sizeof(uint32_t));

	//if () {
	//	int opt_len = ...
	//	memcpy(&(header->optional), &buf[20], opt_len);
	//}
}

ipv4_header* assemble_ipv4_header(src, dest) {
	ipv4_header* h = (ipv4_header*) malloc(size(ipv4_header));
	// ...
	return null;
}

