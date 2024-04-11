#include <linux/inet.h>
#include <linux/etherdevice.h>
#include <linux/workqueue.h>
#include <linux/kthread.h>

#include "config.h"

#ifndef CONTEXT_H
#define CONTEXT_H

struct Context {
    struct Configuration config;
    char smac[ETH_ALEN];
    char dmac[ETH_ALEN];
    __be32 daddr;
    __be32 saddr;
    bool in_session;
    __be32 session_saddr;
    bool handler_handling;
    bool download_active;
    struct work_struct my_work;
    struct task_struct *threadid;
    char recv_data[1024];
};

#endif 
