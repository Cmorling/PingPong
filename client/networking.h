#include "config.h"

#ifndef NETWORKING_H
#define NETWORKING_H

#define DATA_SIZE 400
#define TIMEOUT_SEC 2
#define INTERFACE "eth0"

int listen_icmp(struct Configuration *c, size_t packet_size, size_t pp_size,void * recv_pp);
int send_icmp(struct Configuration *c, void *data, int data_len);

#endif
