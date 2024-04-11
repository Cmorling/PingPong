#include <linux/fs.h>
#include <linux/types.h>

#include "context.h"

#ifndef FILE_H
#define FILE_H

struct FileContext {
    char path[254];
    int buf_len;
    loff_t current_pos;
    __u64 file_size;
}; 

int retreive_bytes(struct Context *c, char * file_buf);
int init_file_download(struct Context *c, char * path, int buf_len);
void close_file_download(struct Context *c);
#endif
