//
// Created by rg on 5/12/20.
//

#ifndef EDU25519_CURVE25519_H
#define EDU25519_CURVE25519_H

#include "types.h"

#define KEY_SIZE_BYTES 32

void curve25519_getpub(u8 *pubkey, const u8 *secret);

void curve25519_getshared(u8 *shared, const u8 *pubkey, const u8 *privkey);

#endif //EDU25519_CURVE25519_H
