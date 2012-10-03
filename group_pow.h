/**
 * @file group_pow.h
 * 
 * The structure and methods for powering a group member.
 */
#pragma once
#ifndef GROUP_POW__INCLUDED
#define GROUP_POW__INCLUDED

#include <inttypes.h>
#include <gmp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "closest_23.h"
#include "group.h"

/**
 * Structure required for group_pow methods.
 */
typedef struct {
    group_t* group;

    // used by innermost methods
    group_elem_t* R;  // temporary result
    group_elem_t* T;  // temporary value
    group_elem_t* I;  // inverse
    group_elem_t* E;  // extra. used by external routines.

    // An array of A^{3^i}.
    // Indexing this should be done carefully,
    // using group->elem_size and NOT sizeof(group_elem_t).
    group_elem_t* cubes;
    int cubes_size;  // number of elements in cubes. byte size is

    mpz_t ex;
    mpz_t t;
} group_pow_t;

/**
 * Initialize a group_pow_t given a group_t.
 */
void group_pow_init(group_pow_t* pow, group_t* group);

/**
 * Release resources associated with a group_pow_t.
 */
void group_pow_clear(group_pow_t* pow);


/**
 * Perform a right-to-left non-adjacent format exponentation.
 * This requires logn squares and on average logn/3 compose operations.
 */
void group_pow_naf_r2l(group_pow_t* pow,
                       group_elem_t* R,
                       const group_elem_t* A,
                       const mpz_t n);

/**
 * Perform a right-to-left non-adjacent format exponentation.
 * This requires logn squares and on average logn/3 compose operations.
 */
void group_pow_naf_r2l_u32(group_pow_t* pow,
                           group_elem_t* R,
                           const group_elem_t* A,
                           uint32_t n);

/**
 * Exponentiate A^n where n is given as a 16bit factored 2,3 representation.
 */
void group_pow_factored23(group_pow_t* pow,
                          group_elem_t* R,
                          const group_elem_t* A,
                          const factored_two_three_term16_t* terms,
                          const int term_count);

#endif

