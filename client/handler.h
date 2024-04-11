#include <stdint.h>
#include "config.h"

#ifndef HANDLER_H
#define HANDLER_H

struct HandleContext {
    __uint16_t current_id;
    __uint16_t current_seqid;
    char currentPath[258];
};

int start_handler(struct Configuration *c);

#endif 
