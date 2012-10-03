#include "group_pow.h"
#include "math32.h"
#include "math_mpz.h"

#define DEFAULT_CUBES_SIZE 2048

void group_pow_init(group_pow_t* pow, group_t* group) {
    pow->group = group;
    pow->R = group_elem_alloc(group);
    pow->T = group_elem_alloc(group);
    pow->I = group_elem_alloc(group);
    pow->E = group_elem_alloc(group);

    pow->cubes = group_elem_array_alloc(group, DEFAULT_CUBES_SIZE);
    pow->cubes_size = DEFAULT_CUBES_SIZE;

    mpz_init(pow->ex);
    mpz_init(pow->t);
}

void group_pow_clear(group_pow_t* pow) {
    group_elem_array_free(pow->group, pow->cubes, pow->cubes_size);
    pow->cubes = 0;
    pow->cubes_size = 0;
    
    group_elem_free(pow->group, pow->R);
    group_elem_free(pow->group, pow->T);
    group_elem_free(pow->group, pow->I);
    group_elem_free(pow->group, pow->E);
    mpz_clear(pow->ex);
    mpz_clear(pow->t);

    pow->R = 0;
    pow->T = 0;
    pow->I = 0;
    pow->group = 0;
}

void group_pow_naf_r2l(group_pow_t* pow, group_elem_t* R, const group_elem_t* A, const mpz_t n) {
    group_t* group = pow->group;
    int m = 0; // two bit window into (n+(c*2^i))
    int c = 0; // carry flag
    int i = 0; // bit index into n
    size_t l = mpz_sizeinbase(n, 2);

    group->set_id(group, pow->R);
    group->set(group, pow->T, A);

    while (i < l || c) {

        // take floor(n/(2^i))+c (mod 4)
        m = (mpz_get_bit_window(n, i, 2) + c) & 3;
        if (m == 1) {
            // subtract 1 from n, compose T with R
            group->compose(group, pow->R, pow->R, pow->T);
            c = 0;
        }
        else if (m == 3) {
            // add 1 to n, compose T^-1 with R
            group->inverse(group, pow->T);
            group->compose(group, pow->R, pow->R, pow->T);
            group->inverse(group, pow->T);
            c = 1;
        }

        // n is now even, divide by 2 and square T
        group->square(group, pow->T, pow->T);
        i ++;
    }

    // copy to output
    group->set(group, R, pow->R);
}

void group_pow_naf_r2l_u32(group_pow_t* pow, group_elem_t* R, const group_elem_t* A, uint32_t n) {
    group_t* group = pow->group;
    int m = 0;

    group->set_id(group, pow->R);
    group->set(group, pow->T, A);

    while (n > 0) {

        // this is okay because we're guaranteed that n > 0
        m = n & 3; // take n (mod 4)
        if (m == 1) {
            // subtract 1 from n, compose T with R
            group->compose(group, pow->R, pow->R, pow->T);
            n --;
        }
        else if (m == 3) {
            // add 1 to n, compose T^-1 with R
            group->inverse(group, pow->T);
            group->compose(group, pow->R, pow->R, pow->T);
            group->inverse(group, pow->T);
            n ++;
        }

        // n is now even, divide by 2 and square T
        group->square(group, pow->T, pow->T);
        n >>= 1;
    }

    // copy to output
    group->set(group, R, pow->R);
}



static void ensure_cubes_size(group_pow_t* pow, uint32_t size) {
    if (pow->cubes_size >= size) {
        return;
    }
    
    printf("%s %d: Resizing to %"PRIu32"\n", __FILE__, __LINE__, size);
    
    // Round 'size' up to the nearest 2^k for some int k>=0
    if (msb_u32(size) != lsb_u32(size)) {
        size = 1 << (msb_u32(size) + 1);
    }

    group_elem_array_free(pow->group, pow->cubes, pow->cubes_size);
    pow->cubes = group_elem_array_alloc(pow->group, size);
    pow->cubes_size = size;
}


/**
 * Exponentiate A^n where n is given as a 16bit factored 2,3 representation.
 */
void group_pow_factored23(group_pow_t* pow,
                          group_elem_t* R,
                          const group_elem_t* A,
                          const factored_two_three_term16_t* terms,
                          const int term_count) {
    group_t* group = pow->group;
    int i;
    int max_b;

    // Find the largest cubed size.
    max_b = 0;
    for (i = 0; i < term_count; i ++) {
        uint16_t t = terms[i].b & ~(1<<15);
        if (t > max_b) {
            max_b = t;
        }
    }

    // ensure there is enough room for the precomputed cubes (0 through max_b)
    ensure_cubes_size(pow, max_b+1);

    // pre compute all the cubes from 0 to max_b
    group->set(group, pow->cubes, A);
    intptr_t p = (intptr_t)pow->cubes;
    for (i = 1; i <= max_b; i ++) {
        group->cube(group, (group_elem_t*)(p + group->elem_size), (group_elem_t*)p);
        p += group->elem_size;
    }

    // Iterate over the factored terms computing the power
    group->set_id(group, pow->R);
    for (i = 0; i < term_count; i ++) {
        // Compose with 3^b term.
        if (terms[i].b & (1<<15)) {
            // Term is negative. Compose with the inverse.
            uint16_t b = terms[i].b & ~(1<<15);
            group_elem_t* p = group_elem_array_index(group, pow->cubes, b);
            group->inverse(group, p);
            group->compose(group, pow->R, pow->R, p);
            group->inverse(group, p);
        }
        else {
            // Term is positive
            uint16_t b = terms[i].b;
            group_elem_t* p = group_elem_array_index(group, pow->cubes, b);
            group->compose(group, pow->R, pow->R, p);
        }

        // compose with 2^a term.
        uint16_t a;
        for (a = 0; a < terms[i].a; a ++) {
            group->square(group, pow->R, pow->R);
        }
    }

    // Copy result to output
    group->set(group, R, pow->R);
}

