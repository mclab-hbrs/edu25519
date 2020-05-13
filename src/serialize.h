#ifndef EDU25519_SERIALIZE_H
#define EDU25519_SERIALIZE_H

#include "types.h"

#define MASK_L25 0x1ffffff
#define MASK_L26 0x3ffffff

void deserialize(s64 *poly, const u8 *bytes);

void serialize(u8 *bytes, const s64 *poly);

#endif //EDU25519_SERIALIZE_H
