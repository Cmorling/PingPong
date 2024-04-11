#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "networking.h"
#include "handler.h"

// Function to calculate checksum for ICMP packet
struct Configuration *config_ptr;

void init_config(struct Configuration *c) {
    strcpy(c->src_addr, SRC_ADDR);
    strcpy(c->dst_addr, DST_ADDR);
    strcpy(c->interface, INTERFACE);
    strcpy(c->password, PASSWORD);
}

int main(int argc, char *argv[]) {
    if (getuid() != 0)
    {
        fprintf(stderr, "%s: This program requires root privileges!\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    config_ptr = malloc(sizeof(struct Configuration));
    init_config(config_ptr);
    printf("    ____  _             ____                   \n");
    printf("   / __ \\(_)___  ____ _/ __ \\____  ____  ____ _\n");
    printf("  / /_/ / / __ \\/ __ `/ /_/ / __ \\/ __ \\/ __ `/ \n");
    printf(" / ____/ / / / / /_/ / ____/ /_/ / / / / /_/ /  \n");
    printf("/_/   /_/_/ /_/\\__, /_/    \\____/_/ /_/\\__, /   \n");
    printf("              /____/                  /____/    \n\n");
    printf("Author - cb4ng\n\n");
    printf("[CONFIG] INTERFACE %s\n", config_ptr->interface);
    printf("[CONFIG] SRC %s ---- DST %s\n", config_ptr->src_addr, config_ptr->dst_addr);
    printf("[CONFIG] PASSWORD %s\n\n", config_ptr->password);
    
    start_handler(config_ptr);
    
    free(config_ptr);
    return 0;
}
