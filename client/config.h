#include <stdint.h>
#ifndef CONFIG_H
#define CONFIG_H

#define SRC_ADDR "172.20.10.6"
#define DST_ADDR "172.20.10.4"
#define INTERFACE "eth0"
#define PASSWORD "password"
#define OUTPUT_FILE "/dev/shm/coredump"
#define RC4_KEY ((uint8_t[]){0xde, 0xad, 0xbe, 0xef, 0xde, 0xad, 0xbe, 0xef})
#define RC4_KEY_LENGTH 8

struct Configuration {
    char src_addr[15];
    char dst_addr[15];
    char interface[8];
    char password[sizeof(PASSWORD)];
    uint8_t rc4_key[RC4_KEY_LENGTH];
    short rc4_key_length;
};


#endif 
