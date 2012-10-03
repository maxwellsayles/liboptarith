#include "mpz_xgcd.h"



void mpz_xgcd_init(mpz_xgcd_t* this, int nbits) {
    mpz_init2(this->q, nbits);
    mpz_init2(this->r, nbits);
    mpz_init2(this->t1, nbits);
    mpz_init2(this->t2, nbits);
}

void mpz_xgcd_clear(mpz_xgcd_t* this) {
    mpz_clear(this->q);
    mpz_clear(this->r);
    mpz_clear(this->t1);
    mpz_clear(this->t2);
}


/**
 * Partial Euclidean algorithm.
 * Lehmer's version for computing GCD
 * (for Book's version of NUCOMP, NUDUPL, and NUCUBE algorithm).
 *
 * Input:  R2 = R_{-1} , R1 = R_{0}, bound
 *  - R_i is the R - sequence from "Solving the Pell Equation"
 *    ( R_i = R_{i-2}-q_i R_{i-1} )
 * Output: R2 = R_{i-1}, R1 = R_i, C2 = C_{i-1}, C1 = C_i,
 *  - R_i = 0 or R_i <= bound < R_{i-1}
 *  - C_i sequence from "Solving the Pell Equation" defined as
 *     C_{-1}=0, C_{1}=-1  C_i=C_{i-2}-q_i C_{i-1}
 */
void mpz_xgcd_partial(mpz_xgcd_t* this, mpz_t R2, mpz_t R1, mpz_t C2, mpz_t C1, const mpz_t bound) {
    long A2, A1, B2, B1, T, T1, rr2, rr1, qq, bb;
    int i;

    A2 = A1 = B2 = B1 = T = T1 = rr2 = rr1 = qq = bb = 0;
    mpz_set_ui(this->q, 0);
    mpz_set_ui(this->r, 0);
    mpz_set_ui(this->t1, 0);
    mpz_set_ui(this->t2, 0);

    mpz_set_si(C1, -1);
    mpz_set_si(C2, 0);

    while ((mpz_sgn(R1) != 0) && (mpz_cmp(R1, bound) > 0)) {
        T = mpz_sizeinbase(R2, 2) - (GMP_LIMB_BITS-1);
        T1 = mpz_sizeinbase(R1, 2) - (GMP_LIMB_BITS-1);
        if (T < T1) T = T1;
        if (T < 0)  T = 0;

        mpz_tdiv_q_2exp(this->r, R2, T);
        rr2 = mpz_get_si(this->r);

        mpz_tdiv_q_2exp(this->r, R1, T);
        rr1 = mpz_get_si(this->r);

        mpz_tdiv_q_2exp(this->r, bound, T);
        bb = mpz_get_si(this->r);

        A2 = 0;  A1 = 1;
        B2 = 1;  B1 = 0;
        i = 0;

        // Euclidean Step
        while (rr1 != 0 && rr1 > bb) {
            qq = rr2 / rr1;

            T = rr2 - qq*rr1; rr2 = rr1; rr1 = T;
            T = A2 - qq*A1; A2 = A1; A1 = T;
            T = B2 - qq*B1; B2 = B1; B1 = T;

            if (i & 1) {
                if ((rr1 < -B1) || (rr2 - rr1 < A1 - A2))
                    break;
            }
            else {
                if ((rr1 < -A1) || (rr2 - rr1 < B1 - B2))
                    break;
            }
            i++;
        }

        if (i == 0) {
            // multiprecision step
            mpz_fdiv_qr(this->q, this->r, R2, R1);
            mpz_set(R2, R1);
            mpz_set(R1, this->r);
            mpz_mul(this->r, this->q, C1);
            mpz_sub(C2, C2, this->r);
            mpz_swap(C2, C1);
        }
        else {
            // recombination
            // r = R2*B2 + R1*A2;  R1 = R2*B1 + R1*A1;  R2 = r;
            mpz_mul_si(this->t1, R2, B2);
            mpz_mul_si(this->t2, R1, A2);
            mpz_add(this->r, this->t1, this->t2);

            mpz_mul_si(this->t1, R2, B1);
            mpz_mul_si(this->t2, R1, A1);
            mpz_add(R1, this->t1, this->t2);

            mpz_set(R2, this->r);

            // r = C2*B2 + C1*A2;  C1 = C2*B1 + C1*A1;  C2 = r;
            mpz_mul_si(this->t1, C2, B2);
            mpz_mul_si(this->t2, C1, A2);
            mpz_add(this->r, this->t1, this->t2);

            mpz_mul_si(this->t1, C2, B1);
            mpz_mul_si(this->t2, C1, A1);
            mpz_add(C1, this->t1, this->t2);

            mpz_set(C2, this->r);

            // make sure R1 and R2 are positive
            if (mpz_sgn(R1) < 0) {
                mpz_neg(R1, R1);
                mpz_neg(C1, C1);
            }
            if (mpz_sgn(R2) < 0) {
                mpz_neg(R2, R2);
                mpz_neg(C2, C2);
            }
        }
    }

    // make sure R2 is positive
    if (mpz_sgn(R2) < 0) {
        mpz_neg(R2, R2);
        mpz_neg(C1, C1);
        mpz_neg(C2, C2);
    }
}


