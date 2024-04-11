#ifndef CONFIG_H
#define CONFIG_H

#define SRC_ADDR ""
#define DST_ADDR ""
#define INTERFACE "eth0"
#define PASSWORD "password"
#define OUTPUT_FILE "/tmp/asd.txt"

struct Configuration {
    char src_addr[15];
    char dst_addr[15];
    char interface[8];
    char password[sizeof(PASSWORD)];
};


#endif 
