//
// Created by rg on 5/12/20.
//

#ifndef EDU25519_MONTGOMERY_H
#define EDU25519_MONTGOMERY_H

#include "types.h"

typedef struct {
    s64 x[20];
    s64 z[20];
} point;

void montgomery_ladder(point *result, const u8 *scalar, const s64 *basepoint);

#endif //EDU25519_MONTGOMERY_H
