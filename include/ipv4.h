#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct { // groups of 4 bytes each

	uint8_t version : 4; // 4 bits
	uint8_t ihl : 4; // 4 bits // IP Header Length // IHL = HeaderLength(bytes) / 4
	uint8_t tos; // 8 bits
	uint16_t length; // 16 bits

	uint16_t identification; // 16 bits
	bool flag_0 : 1;  //reserved, must be 0 (1 bit)
	bool flag_df : 1; // Don't Fragment (1 bit)
	bool flag_mr : 1; // More Fragments (1 bit)
	uint16_t fragment_offset : 13; // 13 bits

	uint8_t ttl; // 8 bits
	uint8_t protocol; // 8 bits
	uint16_t checksum; // 16 bits

	uint32_t src_addr; // 32 bits

	uint32_t dest_addr; // 32 bits

	uint32_t* options; // 40 additional bytes at maximum, in 4 byte steps

} ipv4_header_t;

void serialize_ipv4 (char* buf, const ipv4_header_t* header);
void deserialize_ipv4 (ipv4_header_t* header, const char* buf);
void dump_ipv4_header (ipv4_header_t* header);
ipv4_header_t* assemble_ipv4_header(uint16_t payload_length, uint32_t src, uint32_t dest);
