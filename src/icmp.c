#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include "icmp.h"

void serialize_icmp(char* buf, const icmp_header_t* header) {

}

void deserialize_icmp(icmp_header_t* header, const char* buf) {

}

void dump_icmp_header(icmp_header_t* header) {

}

uint16_t icmp_checksum(const icmp_header_t* head) {
    uint32_t sum = 0;

    sum += head->type << 8 | head->code;
    sum += htons(head->checksum);
    sum += htons(head->ident);
    sum += htons(head->seq_num);

    // Payload (16 bit words at a time)
    for (size_t i = 0; i < (head->payload_len >> 1); ++i) {
        sum += *(uint16_t *) &head->payload[i << 1];
    }

    // Folding carry values
    sum = (sum & 0xFFFF) + (sum >> 16);
    sum = (sum & 0xFFFF) + (sum >> 16);

    return htons((uint16_t) ~sum);
}

icmp_header_t* assemble_echo_request() {
    static uint16_t seq = 0;
	seq += 1;

    icmp_header_t* icmp = (icmp_header_t*) malloc(sizeof(icmp_header_t));
    icmp->type = ECHO_REQUEST;
    icmp->code = 0;

    icmp->ident = 42;
    icmp->seq_num = seq;

    char* str = "lolroflxd";
    icmp->payload_len = sizeof(str);
    icmp->payload = (char*) malloc(icmp->payload_len);
    memcpy(icmp->payload, str, icmp->payload_len);

    icmp->checksum = 0;
    icmp->checksum = icmp_checksum(icmp);

    return icmp;
}

icmp_header_t* assemble_echo_reply(icmp_header_t* echo_req) {
    icmp_header_t* icmp = (icmp_header_t*) malloc(sizeof(icmp_header_t));
    icmp->type = ECHO_REPLY;
    icmp->code = 0;

    icmp->ident = echo_req->ident;
    icmp->seq_num = echo_req->seq_num;

    icmp->payload_len = echo_req->payload_len;
    memcpy(icmp->payload, echo_req->payload, echo_req->payload_len);

    icmp->checksum = 0;
    icmp->checksum = icmp_checksum(icmp);

    return icmp;
}
