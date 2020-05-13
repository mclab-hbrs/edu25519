//
// Created by rg on 5/12/20.
//

#include "src/curve25519.h"
#include <stdio.h>


void print_bytes(u8 *bytes) {
    for (int i = 0; i < 32; i++) {
        printf("%02x", bytes[i]);
    }
    puts("");
}


int main(void) {
    u8 priv[32] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0xAB,0};
    u8 priv2[32] = {0xCC,0xCC,0xCC,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0xAB,0};
    u8 pub[32];
    u8 pub2[32];
    u8 shared1[32];
    u8 shared2[32];

    curve25519_getpub(pub, priv);
    puts("Pubkey 1:");
    print_bytes(pub);

    curve25519_getpub(pub2, priv2);
    puts("Pubkey 2:");
    print_bytes(pub2);

    curve25519_getshared(shared1, pub, priv2);
    puts("Shared key 1:");
    print_bytes(shared1);

    curve25519_getshared(shared2, pub2, priv);
    puts("Shared key 2:");
    print_bytes(shared2);
}