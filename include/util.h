#pragma once

#include "tcp_manager.h"

int write_to_raw_socket(tcp_conn_t* con, void* datagram, size_t dgram_len);
int fd_set_blocking(int fd, int blocking);
