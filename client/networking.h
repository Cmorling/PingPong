#ifndef NETWORKING_H
#define NETWORKING_H

#define DATA_SIZE 400
#define TIMEOUT_SEC 2
#define SRC_ADDR "172.20.10.6" //TODO Remove for development
#define DST_ADDR "172.20.10.4"
#define INTERFACE "eth0"

int listen_icmp(size_t packet_size, size_t pp_size,char *interface, void * recv_pp);
int send_icmp(char *src_addr, char * dst_addr, void *data, int data_len);

#endif