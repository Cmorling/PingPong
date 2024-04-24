#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <net/if.h>
#include <netdb.h>

#include "networking.h"
#include "config.h"

unsigned short cksum(unsigned short *addr, int len)
{
    int nleft = len;
    int sum = 0;
    unsigned short *w = addr;
    unsigned short answer = 0;

    while (nleft > 1)
    {
      sum += *w++;
      nleft -= 2;
    }

    if (nleft == 1)
    {
      *(unsigned char *)(&answer) = *(unsigned char *)w;
      sum += answer;
    }

    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    answer = ~sum;
    
    return (answer);
}
int listen_icmp(struct Configuration *c, size_t packet_size, size_t pp_size, void * recv_pp) {
    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    struct iphdr* ip_header;
    struct icmphdr* icmp_header;
    char recv_buffer[packet_size];
    unsigned char* data;
    int sockfd;

    sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);

    if(setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, c->interface, strlen(c->interface)) < 0) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    if (sockfd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    int bytes_received = recvfrom(sockfd, recv_buffer, packet_size, 0, (struct sockaddr*)&addr, &addrlen);
    if (bytes_received < 0) {
        perror("recvfrom");
        exit(EXIT_FAILURE);
    }
    
    ip_header = (struct iphdr*)recv_buffer;
    icmp_header = (struct icmphdr*)(recv_buffer + sizeof(struct iphdr));
    data = (unsigned char*)(recv_buffer + sizeof(struct iphdr) + sizeof(struct icmphdr));
    
    rc4_crypt(c, data, pp_size);

    memcpy(recv_pp, data, pp_size);
    
    close(sockfd);
    
    return 0;
}

int send_icmp(struct Configuration *c, void *data, int data_len) {
    int sock;
    char send_buf[sizeof(struct ip) + sizeof(struct icmp) + data_len], src_name[256], src_ip[15], dst_ip[15];
    struct ip *d_ip = (struct ip *)send_buf;
    struct icmp *icmp = (struct icmp *)((char *)d_ip + sizeof(struct ip));
    struct sockaddr_in src, dst;
    struct in_addr ip_addr_src;
    struct in_addr ip_addr_dst;
    int on = 1;
    int failed_count = 0;
    int bytes_sent, bytes_recv;
    int dst_addr_len;
    int result;
    fd_set socks;

    rc4_crypt(c, data, data_len);

    memset(&send_buf, 0, sizeof(send_buf));
    memset(&src, 0, sizeof(struct sockaddr_in));
    memset(&dst, 0, sizeof(struct sockaddr_in));
    memset(&ip_addr_dst, 0, sizeof(struct in_addr));
    memset(&ip_addr_src, 0, sizeof(struct in_addr));

    
    if (inet_pton(AF_INET, c->src_addr, &ip_addr_src) != 1) {
        fprintf(stderr, "Invalid IP address format\n");
        return EXIT_FAILURE;
    }
    if (inet_pton(AF_INET, c->dst_addr, &ip_addr_dst) != 1) {
        fprintf(stderr, "Invalid IP address format\n");
        return EXIT_FAILURE;
    }

    d_ip->ip_src = ip_addr_src;
    

    d_ip->ip_dst = ip_addr_dst;
    dst.sin_addr = ip_addr_dst;
    sprintf(src_ip, "%s\n", inet_ntoa(d_ip->ip_src));
    sprintf(dst_ip, "%s\n", inet_ntoa(d_ip->ip_dst));

    if((sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
    {
        perror("socket() error");

        exit(EXIT_FAILURE);
    }

    if(setsockopt(sock, IPPROTO_IP, IP_HDRINCL, &on, sizeof(on)) < 0)
    {
        perror("setsockopt() for IP_HDRINCL error");
        exit(EXIT_FAILURE);
    }

    d_ip->ip_v = 4;
    d_ip->ip_hl = 5;
    d_ip->ip_tos = 0;
    d_ip->ip_len = htons(sizeof(send_buf));
    d_ip->ip_id = htons(321);
    d_ip->ip_off = htons(0);
    d_ip->ip_ttl = 255;
    d_ip->ip_p = IPPROTO_ICMP;

    icmp->icmp_type = ICMP_ECHO;
    icmp->icmp_code = 8;
    icmp->icmp_id = 123;
    icmp->icmp_seq = 0;
    
    dst.sin_family = AF_INET;
    memcpy(icmp->icmp_data, data, data_len);

    d_ip->ip_sum = cksum((unsigned short *)send_buf, d_ip->ip_hl);
    icmp->icmp_cksum = cksum((unsigned short *)icmp, sizeof(icmp) + data_len);
    
    dst_addr_len = sizeof(dst);
    

    FD_ZERO(&socks);
    FD_SET(sock, &socks);
    
    if((bytes_sent = sendto(sock, send_buf, sizeof(send_buf), 0,
                        (struct sockaddr *)&dst, dst_addr_len)) < 0)
    {
        perror("sendto() error");
        failed_count++;
        printf("Failed to send packet.\n");
        fflush(stdout);
    }
    else
    {
        //printf("Sent %d byte packet... \n", bytes_sent);
        close(sock);
        
    }
    close(sock);
    return 0;
};