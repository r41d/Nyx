#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "tcp_manager.h"

#define TCP_HEADER_BASE_LENGTH 20

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

	uint32_t* options; // 40 additional bytes at maximum, in 4 byte steps

} tcp_header_t;

void serialize_tcp(char* buf, const tcp_header_t* header);
void deserialize_tcp(tcp_header_t* header, const char* buf);
void dump_tcp_header(tcp_header_t* header);
uint16_t tcp_checksum(const char* buf, uint32_t src, uint32_t dest, uint16_t len);
tcp_header_t* assemble_tcp_header(uint16_t src_port,
                                  uint16_t dest_port,
                                  uint32_t seq_num,
                                  uint32_t ack_num,
                                  flag_t flags_to_be_send,
                                  uint16_t window);
