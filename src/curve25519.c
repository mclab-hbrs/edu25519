//
// Created by rg on 5/12/20.
//

#include "curve25519.h"
#include "field.h"
#include "montgomery.h"
#include "serialize.h"

#include <string.h>


/**
 * Specified generator point with x=9,z=1
 */
static const s64 generator[ELEMENT_SIZE] = {9};

/**
 * Curve25519 primitive as described in the djb paper.
 * This function can be used via the wrappers below.
 * @param out Output of the sunction. out = scalar*basepoint
 * @param scalar Scalar stating how often the basepoint has to be added to itself
 * @param basepoint Basepoint to add.
 */
static void curve25519(u8 *out, const u8 *scalar, const s64 *basepoint) {
    s64 z_inv[ELEMENT_SIZE];
    uint8_t e[KEY_SIZE_BYTES];
    point P;

    memcpy(e, scalar, KEY_SIZE_BYTES);

    // set lowest 3 bits to zero to get multiple of 8, to avoid small subgroups
    e[0] &= 0xF8;

    // discard highest bit
    e[31] &= 0x7F;
    e[31] |= 0x40;

    montgomery_ladder(&P, e, basepoint);
    invert(z_inv, P.z);
    mul_reduced(P.z, P.x, z_inv);
    serialize(out, P.z);
}


/**
 * Calculate the public key for a given private key.
 * This uses the specified generator (x=9, z=1) as basepoint.
 * @param pubkey Secret key multiplied with base point.
 * @param secret 32 byte little-endian scalar to multiply on generator
 */
void curve25519_getpub(u8 *pubkey, const u8 *secret) {
    curve25519(pubkey, secret, generator);
}


/**
 * Calculate the shared secret for a private key and a foreign public key.
 * @param pubkey Secret key multiplied with base point.
 * @param secret 32 byte little-endian scalar to multiply on generator
 */
void curve25519_getshared(u8 *shared, const u8 *pubkey, const u8 *privkey) {
    s64 pubkey_fe[ELEMENT_SIZE] = {0,};
    deserialize(pubkey_fe, pubkey);
    curve25519(shared, privkey, pubkey_fe);
}
