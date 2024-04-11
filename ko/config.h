#include <linux/inet.h>

#ifndef CONFIG_H
#define CONFIG_H

#define C2_ADDR "172.20.10.6"
#define PASSWORD "password"
// Define a struct for configuration
struct Configuration {
    __be32 c2addr;
    char password[sizeof(PASSWORD)];
};


#endif /* CONFIG_H */
