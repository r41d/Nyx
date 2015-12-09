
#ifndef __IPv4_H__
#define __IPv4_H__

#include <stdint.h>

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

#endif
