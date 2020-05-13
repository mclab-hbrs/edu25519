#include "montgomery.h"
#include "field.h"

#include <string.h> /* memcpy */


/**
 * Function performing on step in the montgomery ladder.
 * See [2] and [3] for specifics.
 * @param res_double Result of 2xa
 * @param res_add Result of a + c
 * @param a Operand 1
 * @param c Operand 2
 * @param base X value of base point
 */
static void double_add(point *res_double, point *res_add, const point *a, const point *c, const s64 *base) {
    s64 A[20], B[20], C[20], D[20], E[20], F[20], G[20], H[20];

    COPY_ELEM(&A, a->x);
    COPY_ELEM(&B, a->z);
    COPY_ELEM(&C, c->x);
    COPY_ELEM(&D, c->z);

    add(A, B);
    sub(B, a->x);

    add(C, D);
    sub(D, c->x);

    mul_reduced(E, A, D);
    mul_reduced(F, B, C);

    COPY_ELEM(D, E);
    add(D, F);
    sub(E, F);

    // Calculate result of a + c
    square_reduced(G, D);
    square_reduced(H, E);
    mul_reduced(E, H, base);

    // Save result of a + c
    COPY_ELEM(&res_add->x, G);
    COPY_ELEM(&res_add->z, E);

    // Calculate result of 2a
    square_reduced(G, A);
    square_reduced(H, B);
    mul_reduced(A, G, H);
    COPY_ELEM(&res_double->x, A);

    // H = G - H
    sub(H, G);

    // C = 121665 * (G - H)
    mul_constant(C, H);
    reduce_coefficients(C);
    add(G, C);
    // D = (G-h) * (G + 121665 * (G - H))
    mul_reduced(D, H, G);
    COPY_ELEM(&res_double->z, D);
}

/**
 * Constant time way to swap two points based on parameter.
 * If swap is 1, a and b will be swapped. If swap is zero, they won't.
 * See [3] for description of this.
 * @param a Operand 1
 * @param b Operand 2
 * @param swap Decision Maker (has to be 0 or 1)
 */
static void swap_points(point *a, point *b, s64 swap) {
    u32 i;
    s32 mask = (s32) -swap;
    s32 x;

    for (i = 0; i < 10; ++i) {
        x = mask & (((s32) a->x[i]) ^ ((s32) b->x[i]));
        a->x[i] = ((s32) a->x[i]) ^ x;
        b->x[i] = ((s32) b->x[i]) ^ x;

        x = mask & (((s32) a->z[i]) ^ ((s32) b->z[i]));
        a->z[i] = ((s32) a->z[i]) ^ x;
        b->z[i] = ((s32) b->z[i]) ^ x;
    }
}


/**
 * Montgomery ladder using only X/Z coordinates. See [2] for details on the algorithm.
 * @param result Resulting point with X/Z value. Result = scalar x basepoint
 * @param scalar Scalar to multiply on basepoint
 * @param basepoint x value of the base point to use for scalar multiplication
 */
void montgomery_ladder(point *result, const u8 *scalar, const s64 *basepoint) {
    point A = {{1},
               {0}};
    point B = {{0},
               {1}};
    point C = {{0},
               {1}};
    point D = {{0},
               {1}};

    point *op_a = &A, *op_b = &B, *res_double = &C, *res_add = &D, *tmp;

    s32 i, j;
    u8 cur_byte, cur_bit;

    COPY_ELEM(B.x, basepoint);

    for (i = 31; i >= 0; --i) {
        cur_byte = scalar[i];
        for (j = 7; j >= 0; --j) {
            cur_bit = (cur_byte >> j) & 1;

            swap_points(op_a, op_b, cur_bit);
            double_add(res_double, res_add, op_a, op_b, basepoint);
            swap_points(res_double, res_add, cur_bit);

            tmp = res_double;
            res_double = op_a;
            op_a = tmp;

            tmp = res_add;
            res_add = op_b;
            op_b = tmp;
        }
    }

    memcpy(result, &A, sizeof(point));
}