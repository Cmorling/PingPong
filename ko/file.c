#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>

#include "file.h"
#include "context.h"

struct FileContext *fc;

int init_file_download(struct Context *c, char * path, int buf_len) {
    struct file *file;

    fc = kmalloc(sizeof(struct FileContext), GFP_KERNEL);

    strcpy(fc->path, path);
    
    file = filp_open(fc->path, O_RDONLY, 0);
    if (IS_ERR(file)) {
        printk("filp_open error %ld\n", PTR_ERR(file));
        printk(KERN_ERR "Cannot open file: %s\n", fc->path);
        return 1;
    }
    
    fc->file_size = i_size_read(file_inode(file));
    fc->buf_len = buf_len - 1;
    fc->current_pos = 0;

    filp_close(file, NULL);

    c->download_active = 1;

    return fc->file_size;
}
void close_file_download(struct Context *c) {
    c->download_active = 0;
    kfree(fc);
    printk("closing file");
}

int retreive_bytes(struct Context *c, char * file_buf) {
    struct file *file;
    int bytes_read;

    
    file = filp_open(fc->path, O_RDONLY, 0);
    if (IS_ERR(file)) {
        printk(KERN_ERR "Cannot open file: %s\n", fc->path);
        return 1;
    }

    bytes_read = kernel_read(file, file_buf, fc->buf_len, &fc->current_pos);
    
    filp_close(file, NULL);

    printk(KERN_INFO "[POINTER DEBUG] current_pos %d, file_size %lld", fc->current_pos, fc->file_size);
    if (fc->current_pos == fc->file_size) {
        close_file_download(c);
        return 1;
    }
    
    return 0;
}


EXPORT_SYMBOL(init_file_download);
EXPORT_SYMBOL(retreive_bytes);
EXPORT_SYMBOL(close_file_download);