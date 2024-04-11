#ifndef HANDLER_H
#define HANDLER_H

#include "context.h"

int handle_packet(void *data);
int handle_hello(struct Context *c);
int handle_gdbye(struct Context *c);
int handle_exec(struct Context *c);
#endif
