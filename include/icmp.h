#pragma once

#include <stdint.h>
#include <stdbool.h>

//enum Himmelsrichtung{NORD, OST=90, SUED=180, WEST=270};
typedef enum {
    ECHO_REPLY = 0,
    // DESTINATION_UNREACHABLE = 3,
    // SOURCE_QUENCH = 4,
    // REDIRECT_MESSAGE = 5,
    ECHO_REQUEST = 8,
    // TIME_EXCEEDED = 11,
    // PARAMETER_PROBLEM = 12,
    // TIMESTAMP = 13,
    // TIMESTAMP_REPLY = 14,
} icmp_type_t;

typedef struct {

	icmp_type_t type;
	uint8_t code;
    uint16_t checksum;

    uint16_t ident;
    uint16_t seq_num;

    char* payload;
        size_t payload_len;

} icmp_header_t;

void serialize_icmp(char* buf, const icmp_header_t* header);
void deserialize_icmp(icmp_header_t* header, const char* buf);
void dump_icmp_header(icmp_header_t* header);
uint16_t icmp_checksum(const icmp_header_t* head);
icmp_header_t* assemble_echo_request();
icmp_header_t* assemble_echo_reply();
