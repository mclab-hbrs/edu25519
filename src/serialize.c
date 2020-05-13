#include "serialize.h" /* masks */
#include "types.h" /* short type names */
#include "field.h" /* IS_ODD */


/*
 * As the serialization code is quite boring, it was taken from Adam Langleys Donna Code:
 * https://github.com/agl/curve25519-donna/blob/master/LICENSE.md
 */

/***
 * Assigns a polynomial coefficient by shuffling the byte array.
 * Helper function for deserialize.
 * @param poly Pointer to polynomial
 * @param bytes pointer to input bytes
 * @param index index of coefficient
 * @param offset offset in bytes at which to start
 * @param cutoff How many bits have to be cut off at the end of the coeff (because they were part of the previous coeff)
 * @param mask How many bits to keep (26 for even index, 25 for odd...)
 */
static inline void assign_coeff(
        s64 *poly, const u8 *bytes, const u32 index, const u32 offset, const u32 cutoff, const u32 mask) {
    poly[index] = ((u32) bytes[offset + 0]) | ((u32) bytes[offset + 1]) << 8 | ((u32) bytes[offset + 2]) << 16 |
                  ((u32) bytes[offset + 3]) << 24;
    poly[index] >>= cutoff;
    poly[index] &= mask;
}

/***
 * Turn a 32 byte string into polynomial form.
 * @param poly Output Poly, array of 10 64 bit limbs
 * @param bytes Little endian 32B byte array
 */
void deserialize(s64 *poly, const u8 *bytes) {
    // This is hard coded, just so we don't need to compute anything at runtime
    assign_coeff(poly, bytes, 0, 0, 0, MASK_L26);
    assign_coeff(poly, bytes, 1, 3, 2, MASK_L25);
    assign_coeff(poly, bytes, 2, 6, 3, MASK_L26);
    assign_coeff(poly, bytes, 3, 9, 5, MASK_L25);
    assign_coeff(poly, bytes, 4, 12, 6, MASK_L26);
    assign_coeff(poly, bytes, 5, 16, 0, MASK_L25);
    assign_coeff(poly, bytes, 6, 19, 1, MASK_L26);
    assign_coeff(poly, bytes, 7, 22, 3, MASK_L25);
    assign_coeff(poly, bytes, 8, 25, 4, MASK_L26);
    assign_coeff(poly, bytes, 9, 28, 6, MASK_L25);
}

/* s32_eq returns 0xffffffff iff a == b and zero otherwise. */
static s32 s32_eq(s32 a, s32 b) {
    a = ~(a ^ b);
    a &= a << 16;
    a &= a << 8;
    a &= a << 4;
    a &= a << 2;
    a &= a << 1;
    return a >> 31;
}


/* s32_gte returns 0xffffffff if a >= b and zero otherwise, where a and b are
 * both non-negative. */
static s32 s32_gte(s32 a, s32 b) {
    a -= b;
    /* a >= 0 iff a >= b. */
    return ~(a >> 31);
}

/***
 * Turn a reduced polynomial into a 32 byte little endian array.
 * @param bytes Little endian 32B byte array
 * @param poly Reduced output Poly
 */
void serialize(u8 *bytes, const s64 *poly) {
    /*
     * This code is largely taken from Adam Langley's donna implementation.
     * It makes sure that we evaluate the polynomial at 1 and takes care of
     * negative coefficients in a time-invariant manner.
     */
    u32 i;
    u32 j;
    s32 input[10];
    s32 mask, carry;

    // coefficients are < 2^26, so s32 works.
    for (i = 0; i < 10; i++) {
        input[i] = poly[i];
    }

    /* Make all coefficients positive, by borrowing from higher coefficients.
     * This always works, since we can "add" the 25th (odd case) and 26th (even case)
     * bit to a coefficient and "subtract" the equivalent bit in the higher order
     * coefficient.
     */
    // We need two iterations, since input[0] might be negative after the first one.
    for (j = 0; j < 2; ++j) {
        for (i = 0; i < 9; ++i) {
            if (IS_ODD(i)) {
                mask = input[i] >> 31;
                carry = -((input[i] & mask) >> 25);
                input[i] = input[i] + (carry << 25);
                input[i + 1] = input[i + 1] - carry;
            } else {
                mask = input[i] >> 31;
                carry = -((input[i] & mask) >> 26);
                input[i] = input[i] + (carry << 26);
                input[i + 1] = input[i + 1] - carry;
            }
        }
        /* input[9] is the highest used coefficient in the reduced form, so
         * it is not possible to borrow from a higher coeff.
         * However, since we work mod 2^255-19, we can borrow from input[0]
         * by multiplying the carry by 19.
         * This is possible because it would affect the highest bits in input[9],
         * which multiplied by 19 would end up in input[10], which would affect
         * input[0] after reduction.
         */
        mask = input[9] >> 31;
        carry = -((input[9] & mask) >> 25);
        input[9] = input[9] + (carry << 25);
        input[0] = input[0] - (carry * 19);
    }


    /* The first borrow-propagation pass above ended with every limb
       except (possibly) input[0] non-negative.

       If input[0] was negative after the first pass, then it was because of a
       carry from input[9]. On entry, input[9] < 2^26 so the carry was, at most,
       one, since (2**26-1) >> 25 = 1. Thus input[0] >= -19.

       In the second pass, each limb is decreased by at most one. Thus the second
       borrow-propagation pass could only have wrapped around to decrease
       input[0] again if the first pass left input[0] negative *and* input[1]
       through input[9] were all zero.  In that case, input[1] is now 2^25 - 1,
       and this last borrow-propagation step will leave input[1] non-negative. */

    mask = input[0] >> 31;
    carry = -((input[0] & mask) >> 26);
    input[0] = input[0] + (carry << 26);
    input[1] = input[1] - carry;

    /* All input[i] are now non-negative. However, there might be values between
     * 2^25 and 2^26 in a limb which is, nominally, 25 bits wide. */
    for (j = 0; j < 2; j++) {
        for (i = 0; i < 9; i++) {
            if ((i & 1) == 1) {
                const s32 carry = input[i] >> 25;
                input[i] &= 0x1ffffff;
                input[i + 1] += carry;
            } else {
                const s32 carry = input[i] >> 26;
                input[i] &= 0x3ffffff;
                input[i + 1] += carry;
            }
        }

        {
            const s32 carry = input[9] >> 25;
            input[9] &= 0x1ffffff;
            input[0] += 19 * carry;
        }
    }
    /* If the first carry-chain pass, just above, ended up with a carry from
     * input[9], and that caused input[0] to be out-of-bounds, then input[0] was
     * < 2^26 + 2*19, because the carry was, at most, two.
     *
     * If the second pass carried from input[9] again then input[0] is < 2*19 and
     * the input[9] -> input[0] carry didn't push input[0] out of bounds. */

    /* It still remains the case that input might be between 2^255-19 and 2^255.
     * In this case, input[1..9] must take their maximum value and input[0] must
     * be >= (2^255-19) & 0x3ffffff, which is 0x3ffffed. */
    mask = s32_gte(input[0], 0x3ffffed);
    for (i = 1; i < 10; i++) {
        if ((i & 1) == 1) {
            mask &= s32_eq(input[i], 0x1ffffff);
        } else {
            mask &= s32_eq(input[i], 0x3ffffff);
        }
    }

    /* mask is either 0xffffffff (if input >= 2^255-19) and zero otherwise. Thus
     * this conditionally subtracts 2^255-19. */
    input[0] -= mask & 0x3ffffed;

    for (i = 1; i < 10; i++) {
        if ((i & 1) == 1) {
            input[i] -= mask & 0x1ffffff;
        } else {
            input[i] -= mask & 0x3ffffff;
        }
    }

    input[1] <<= 2;
    input[2] <<= 3;
    input[3] <<= 5;
    input[4] <<= 6;
    input[6] <<= 1;
    input[7] <<= 3;
    input[8] <<= 4;
    input[9] <<= 6;
#define F(i, s) \
    bytes[s+0] |=  input[i] & 0xff; \
    bytes[s+1]  = (input[i] >> 8) & 0xff; \
    bytes[s+2]  = (input[i] >> 16) & 0xff; \
    bytes[s+3]  = (input[i] >> 24) & 0xff;
    bytes[0] = 0;
    bytes[16] = 0;
    F(0, 0);
    F(1, 3);
    F(2, 6);
    F(3, 9);
    F(4, 12);
    F(5, 16);
    F(6, 19);
    F(7, 22);
    F(8, 25);
    F(9, 28);
#undef F
}
