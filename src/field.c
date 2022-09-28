#include "field.h"

#include <string.h>  /* memset */

/**
 * Does straight forward integer multiplication (operand scanning form), see [1] p.32.
 * The returned polynomial is not reduced.
 * @param result Resulting non-reduced polynomial
 * @param a Operand 1
 * @param b Operand 2
 */
void mul(s64 *result, const s64 *a, const s64 *b) {
    u32 i, j;

    memset(result, 0, ELEMENT_SIZE_BYTES);

    for (i = 0; i < 10; ++i) {
        for (j = 0; j < 10; ++j) {
            // Since we deal with numbers < 2^26, we don't have to worry about the carry
            if (IS_ODD(i) && IS_ODD(j)) {
                // Here we store the product of two odd indices,
                // that is two 25bit numbers in an even index,
                // therefore a 26bit number
                result[i + j] += 2 * a[i] * b[j];
            } else {
                result[i + j] += a[i] * b[j];
            }
        }
    }
}

/**
 * Multiply two polynomials and reduce them degree and coefficient wise.
 * result = a * b (mod p)
 * @param result Reduced polynomial product of a and b
 * @param a Operand 1
 * @param b Operand 2
 */
void mul_reduced(s64 *result, const s64 *a, const s64 *b) {
    mul(result, a, b);
    reduce_degree(result);
    reduce_coefficients(result);
}

/**
 * Multiply the evaluation of the polynomial at 1
 * with constant 121665. See [1] for explanation of constant.
 * @param result a(1) * 121665
 * @param a Operand 1
 */
void mul_constant(s64 *result, const s64 *a) {
    u32 i;
    for (i = 0; i < 10; ++i) {
        result[i] = a[i] * 121665;
    }
}

/**
 * Square the polynomial.
 * Result = a * a
 * @param result The squared element
 * @param a Operand 1
 */
void square(s64 *result, const s64 *a) {
    mul(result, a, a);
}

/**
 * Square the polynomial and reduce it.
 * @param result
 * @param a
 */
void square_reduced(s64 *result, const s64 *a) {
    square(result, a);
    reduce_degree(result);
    reduce_coefficients(result);
}

/**
 * Calculate the sum of two reduced polynomials.
 * Result += a
 * @param result Result and Operand 1
 * @param a Operand 2
 */
void add(s64 *result, const s64 *a) {
    u32 i;
    for (i = 0; i < 10; ++i) {
        result[i] += a[i];
    }
}


/**
 * Calculate the difference of two reduced polynomials.
 * Result =  result - in
 * @param result Result and Operand 1
 * @param a Operand 2
 */
void sub(s64 *result, const s64 *a) {
    u32 i;
    for (i = 0; i < 10; ++i) {
        result[i] = a[i] - result[i];
    }
}


/**
 * Reduce the number represented by the polynomial (the evaluation
 * of the polynomial at 1) by the modulus 2^255-19. The resulting
 * polynomial uses only 10 coefficients.
 * poly(1) %= 2^255-19
 * @param poly The poly to be reduced.
 */
void reduce_degree(s64 *poly) {
    u32 i;
    /* Since 2**255*x**10 - 19 is in the ring
     * one can multiply 19 to all polynomial parts with exponent > 10
     * and then add it to the polynomial part (exponent%10)
     * For a good explanation of what is going on, see [3].
     */
    for (i = 0; i < 9; ++i) {
        poly[i] += 19 * poly[i + 10];
    }

    for (i = 10; i < 20; ++i) {
        poly[i] = 0;
    }
}

/**
 * Takes a reduced degree polynomial and reduces the coefficients,
 * forcing them under 25/26 bit size.
 * @param poly Reduced degree polynomial
 */
void reduce_coefficients(s64 *poly) {
    u32 i;
    s64 carry;

    for (i = 0; i < 10; i += 2) {
        /* Even index case.
         * Take bits with position > 26 and add
         * them to lower bits of next highest
         * coefficient.
         */
        carry = poly[i] / (1 << 26);
        poly[i + 1] += carry;
        poly[i] -= carry << 26;

        /* Odd ind case.
         * Do the same as above, but with
         * 25 bits.
         */
        carry = poly[i + 1] / (1 << 25);
        poly[i + 2] += carry;
        poly[i + 1] -= carry << 25;
    }
    /* Now, since we shoved carrys to the higher order
     * coefficients, poly[10] might be >0.
     * It is, however, bounded by poly[10] < 281*2^29.
     * We take care of this by reducing with our
     * "times 19" trick.
     */

    poly[0] += 19 * poly[10];
    poly[10] = 0;

    /* Now poly[0] might be slightly too big.
     * Since poly[0] was reduced, and poly[10] bounded by < 281*2^29
     * poly[0] can be too big by a maximum of 19*281*2^29, so the carry
     * can't be larger than 16 bits.
     * So we carry to poly[1].
     */
    carry = poly[0] / (1 << 26);
    poly[1] += carry;
    poly[0] -= carry << 26;
    /* Now |output[1]| < 2^25 + 2^16 < 2^26, which is not necessarily
     * 25 bits long, but good enough.
     */
}

/**
 * Invert the polynomial by taking it to the power of p-2.
 * Result = a^-1 (mod p)
 * @param result Inverse element of a.
 * @param a Operand 1
 */
void invert(s64 *result, const s64 *a) {
    s64 tmp_result[20] = {0,};
    tmp_result[0] = 1;
    u32 i;

    // Use the hardcoded binary representation of 2^255-21
    // 250 x 1
    for (i = 0; i < 250; ++i) {
        square_reduced(result, tmp_result);
        mul_reduced(tmp_result, result, a);
    }

    // 0
    square_reduced(result, tmp_result);

    // 1
    square_reduced(tmp_result, result);
    mul_reduced(result, tmp_result, a);

    // 0
    square_reduced(tmp_result, result);

    // 1
    square_reduced(result, tmp_result);
    mul_reduced(tmp_result, result, a);

    // 1
    square_reduced(result, tmp_result);
    mul_reduced(tmp_result, result, a);

    COPY_ELEM(result, tmp_result);
}
