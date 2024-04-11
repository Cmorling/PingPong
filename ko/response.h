#include <linux/inet.h>
#include "context.h"

#ifndef RESPOND_H
#define RESPOND_H

unsigned short cksum(void *b, int len);
int send_icmp(void *payload, ssize_t payload_len, struct Context *c);

#endif
