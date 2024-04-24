#include <linux/inet.h>

#ifndef CONFIG_H
#define CONFIG_H

#define C2_ADDR "172.20.10.6"
#define PASSWORD "password"

#define RC4_KEY ((uint8_t[]){0xde, 0xad, 0xbe, 0xef, 0xde, 0xad, 0xbe, 0xef})
#define RC4_KEY_LENGTH 8
// Define a struct for configuration
struct Configuration {
    __be32 c2addr;
    char password[sizeof(PASSWORD)];
    uint8_t rc4_key[RC4_KEY_LENGTH];
    short rc4_key_length;
};


#endif /* CONFIG_H */
