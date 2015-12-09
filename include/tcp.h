
#ifndef __TCP_H__
#define __TCP_H__

#include <stdint.h>

typedef struct { // groups of 4 bytes each

	uint16_t src_port; // 16 bit
	uint16_t dest_port; // 16 bit

	uint32_t seq_num; // 32 bit

	uint32_t ack_num; // 32 bit

	uint8_t data_offset; // 4 bit
	// 4 reserved bits
	// 2 reserved bits
	_Bool urg; // 1 bit
	_Bool ack; // 1 bit
	_Bool psh; // 1 bit
	_Bool rst; // 1 bit
	_Bool syn; // 1 bit
	_Bool fin; // 1 bit
	uint16_t window; // 16 bit

	uint16_t checksum; // 16 bit
	uint16_t urgent_pointer; // 16 bit

	uint32_t* options; // zero or more 32-bit-words

} tcp_header;

#endif
