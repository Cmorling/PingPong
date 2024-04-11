#include <stdint.h>

#ifndef PROTOCOL_H
#define PROTOCOL_H

#define MPS 1024
#define FLAG_PINGPONG 1

#define FLAG_HELLO (FLAG_PINGPONG)
#define FLAG_GDBYE (FLAG_PINGPONG << 1)
#define FLAG_BEGIN_STREAM (FLAG_PINGPONG << 2)
#define FLAG_END_STREAM (FLAG_PINGPONG << 3)
#define FLAG_DATA_STREAM (FLAG_PINGPONG << 4)
#define FLAG_FILE_DOWNLOAD (FLAG_PINGPONG << 5)
#define FLAG_FILE_UPLOAD (FLAG_PINGPONG << 6)
#define FLAG_EXECUTE_COMMAND (FLAG_PINGPONG << 7)

struct ProtocolHeader {
    char magic[4];
    __uint16_t id;
    __uint16_t seqid;
    __uint16_t payload_len;
    __uint16_t flags;
};

struct ProtocolHello {
    struct ProtocolHeader pphdr;
    __uint8_t is_response;
    char passphrase[15];
};

struct ProtocolGoodbye {
    struct ProtocolHeader pphdr;
    __uint8_t is_response;
};

struct ProtocolDataStream {
    struct ProtocolHeader pphdr;
    char data[MPS];
};

struct ProtocolExec{
    struct ProtocolHeader pphdr;
    __uint8_t is_response;
    __uint8_t exit_status;
    char cmdline[254];
};

struct ProtocolDownload{
    struct ProtocolHeader pphdr;
    __uint16_t mps;
    char path[254];
};

#endif 