#ifndef EDU25519_FIELD_H
#define EDU25519_FIELD_H

#include "types.h"

#define ELEMENT_SIZE 20
#define ELEMENT_SIZE_BYTES (ELEMENT_SIZE * sizeof(s64))

#define IS_ODD(x) ((x)&1)
#define COPY_ELEM(x, y) memcpy((x), (y), ELEMENT_SIZE_BYTES)

void mul(s64 *result, const s64 *a, const s64 *b);
void reduce_degree(s64 *poly);
void reduce_coefficients(s64 *poly);
void mul_reduced(s64 *result, const s64 *a, const s64 *b);
void add(s64 *result, const s64 *a);
void sub(s64 *result, const s64 *a);
void square(s64 *result, const s64 *a);
void square_reduced(s64 *result, const s64 *a);
void invert(s64 *result, const s64 *a);
void mul_constant(s64 *result, const s64 *a);

#endif //EDU25519_FIELD_H