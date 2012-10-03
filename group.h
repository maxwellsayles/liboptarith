/**
 * @file group.h
 * Describes a group by the size of its elements
 * and the operations available on its elements
 * (mainly compose, square, cube, inverse).
 */

#pragma once
#ifndef GROUP__INCLUDED
#define GROUP__INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdlib.h>

typedef void group_elem_t;
typedef struct group_struct group_t;

typedef void group_elem_init_f(group_t* group, group_elem_t* R);
typedef void group_elem_clear_f(group_t* group, group_elem_t* R);
typedef uint32_t group_hash32_f(group_t* group, const group_elem_t* A);
typedef void group_set_id_f(group_t* group, group_elem_t* R);
typedef int group_is_id_f(group_t* group, const group_elem_t* A);
typedef void group_set_f(group_t* group, group_elem_t* R, const group_elem_t* A);
typedef int group_equal_f(group_t* group, const group_elem_t* A, const group_elem_t* B);
typedef void group_inverse_f(group_t* group, group_elem_t* R);
typedef void group_compose_f(group_t* group, group_elem_t* R, const group_elem_t* A, const group_elem_t* B);
typedef void group_square_f(group_t* group, group_elem_t* R, const group_elem_t* A);
typedef void group_cube_f(group_t* group, group_elem_t* R, const group_elem_t* A);
typedef void group_print_f(group_t* group, const group_elem_t* A);

struct group_struct {
    group_elem_init_f* elem_init;
    group_elem_clear_f* elem_clear;
    int elem_size;

    group_hash32_f* hash32;
    group_set_id_f* set_id;
    group_is_id_f* is_id;
    group_set_f* set;
    group_equal_f* equal;
    group_inverse_f* inverse;
    group_compose_f* compose;
    group_square_f* square;
    group_cube_f* cube;
    group_print_f* print;
};


// allocate and initialize the memory for a single element
static inline group_elem_t* group_elem_alloc(group_t* group) {
    void* res = malloc(group->elem_size);
    group->elem_init(group, res);
    return res;
}

// clear and free the memory for a single element
static inline void group_elem_free(group_t* group, group_elem_t* elem) {
    group->elem_clear(group, elem);
    free(elem);
}

// returns a pointer to the specified index in the array
static inline group_elem_t* group_elem_array_index(group_t* group,
                                                   group_elem_t* A,
                                                   const int index) {
    intptr_t p = (intptr_t)A;
    return (group_elem_t*)(p + index * group->elem_size);
}

// allocate a contiguous block of memory and initialize each group element
static inline group_elem_t* group_elem_array_alloc(group_t* group,
                                                   const int count) {
    group_elem_t* A = (group_elem_t*)malloc(count * group->elem_size);
    if (!A) {
        return 0;
    }
    int i;
    intptr_t p = (intptr_t)A;
    for (i = 0; i < count; i ++) {
        group->elem_init(group, (group_elem_t*)p);
        p += group->elem_size;
    }
    return A;
}


// clear the memory for an array of elements and then free the block
static inline void group_elem_array_free(group_t* group,
                                         group_elem_t* A, const int size) {
    if (A && size) {
        int i;
        intptr_t p = (intptr_t)A;
        for (i = 0; i < size; i ++) {
            group->elem_clear(group, (group_elem_t*)p);
            p += group->elem_size;
        }
        free(A);
    }
}



typedef struct {
    double compose;
    double square;
    double cube;
} group_cost_t;

extern const group_cost_t unit_costs;
extern const group_cost_t compose_only_costs;



// extern "C" {
#ifdef __cplusplus
}
#endif

#endif

