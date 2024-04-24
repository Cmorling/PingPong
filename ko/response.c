#include <linux/module.h>           // Core header for loading LKMs into the kernel
#include <linux/kernel.h>           // Contains types, macros, functions for the kernel

#include <linux/types.h>
#include <linux/sched.h>
#include <linux/mm.h>
#include <linux/fcntl.h>
#include <linux/socket.h>
#include <linux/in.h>
#include <linux/inet.h>
#include <linux/skbuff.h>
#include <linux/if_arp.h>
#include <linux/netdevice.h> 
#include <linux/etherdevice.h>
//#include <linux/string.h>
#include <linux/ip.h> 
#include <linux/udp.h>
#include <linux/icmp.h>

#include "response.h"
#include "context.h"
#include "crypto.h"
unsigned short cksum(void *b, int len) {
    unsigned short *buf = b;
    unsigned int sum = 0;
    unsigned short result;

    for (sum = 0; len > 1; len -= 2)
        sum += *buf++;
    if (len == 1)
        sum += *(unsigned char *)buf;
    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    result = ~sum;
    return result;
}

struct net_device* dev_get_by_mac(const unsigned char *mac) {
    struct net_device *dev = NULL;
    struct net_device *temp_dev = NULL;

    // Iterate over each network device
    read_lock(&dev_base_lock); // Protect the device list
    for_each_netdev(&init_net, temp_dev) {
        if (ether_addr_equal(temp_dev->dev_addr, mac)) {
            // MAC address matches
            dev = temp_dev;
            break;
        }
    }
    read_unlock(&dev_base_lock);

    if (dev) {
        // Found a device with the matching MAC address
        dev_hold(dev); // Increment device reference count if you plan to use it
    }

    return dev;
}

int send_icmp(void *payload, ssize_t payload_len, struct Context *c)
{
  struct sk_buff *skb;
    struct net_device *dev;
    struct ethhdr *eth;
    struct iphdr *iph;
    struct icmphdr *icmph;
    unsigned char *data;
    //unsigned char src_mac[ETH_ALEN];


    //dev = dev_get_by_name(&init_net, dev_name);
    dev = dev_get_by_mac(c->smac);
    if (!dev) {
        printk(KERN_ERR "pingpong xmit: Device not found\n");
        return -ENODEV;
    }

    skb = alloc_skb(LL_MAX_HEADER + sizeof(struct iphdr) + sizeof(struct icmphdr) + payload_len, GFP_ATOMIC);
    if (!skb) {
        printk(KERN_ERR "pingpong xmit: skb allocation failed\n");
        return -ENOMEM;
    }

    skb_reserve(skb, LL_MAX_HEADER); // Reserve space for headers
    skb->dev = dev;
    skb->pkt_type = PACKET_OUTGOING;

    // Prepare ICMP payload
    data = skb_put(skb, payload_len);
    memcpy(data, payload, payload_len);
    rc4_crypt(&c->config, data, payload_len);

    // ICMP Header
    icmph = (struct icmphdr *)skb_push(skb, sizeof(struct icmphdr));
    icmph->type = ICMP_ECHOREPLY;
    icmph->code = 0;
    icmph->un.echo.sequence = htons(1);
    icmph->un.echo.id = htons(1234); // Arbitrary ID
    icmph->checksum = 0;
    icmph->checksum = cksum(icmph, sizeof(struct icmphdr) + payload_len);

    // IP Header
    iph = (struct iphdr *)skb_push(skb, sizeof(struct iphdr));
    iph->version = 4;
    iph->ihl = 5;
    iph->ttl = 64;
    iph->saddr = c->saddr; // Source IP
    iph->daddr = c->daddr; // Destination IP
    iph->protocol = IPPROTO_ICMP;
    iph->tot_len = htons(skb->len);
    iph->check = 0;
    iph->check = cksum(iph, iph->ihl*4);

    // Ethernet Header
    skb_reset_mac_header(skb);
    eth = (struct ethhdr *)skb_push(skb, ETH_HLEN);
    skb->protocol = eth->h_proto = htons(ETH_P_IP);
    memcpy(eth->h_source, c->smac, ETH_ALEN);
    memcpy(eth->h_dest, c->dmac, ETH_ALEN);

    // Send the packet
    if (dev_queue_xmit(skb) < 0) {
        printk(KERN_ERR "my_icmp_init: Error sending packet\n");
        return -EFAULT;
    }

    printk(KERN_INFO "DEV_QUEUE_XMIT: ICMP packet sent\n");
    return 0;
} 

EXPORT_SYMBOL(send_icmp);
EXPORT_SYMBOL(cksum);