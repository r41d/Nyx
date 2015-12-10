#pragma once
#include <stdint.h>
#include <stdbool.h>

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

uint16_t tcp_checksum(const char* buf, uint32_t src, uint32_t dest, uint16_t len);
