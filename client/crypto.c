#include <stdint.h>
#include <string.h>

#include "config.h"
#include "crypto.h"
//implements RC4
void rc4_crypt(struct Configuration *c, char *inData, int data_len) {
    uint8_t i = 0, j = 0;
    uint8_t state[256];

    for (int i = 0; i < 256; i++) {
        state[i] = i;
    }

    for (int i = 0; i < 256; i++) {
        j = (j + state[i] + c->rc4_key[i % c->rc4_key_length]) % 256;
        uint8_t temp = state[i];
        state[i] = state[j];
        state[j] = temp;
    }
    i = 0;
    j = 0;
    for (int n = 0; n < data_len; n++) {
        i = (i + 1) % 256;
        j = (j + state[i]) % 256;
        
        uint8_t temp = state[i];
        state[i] = state[j];
        state[j] = temp;
        
        uint8_t k = state[(state[i] + state[j]) % 256];
        inData[n] ^= k;
    }
}
