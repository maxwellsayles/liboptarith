/**
 * @file group.h
 * Describes a group by the size of its elements
 * and the operations available on its elements
 * (mainly compose, square, cube, inverse).
 */

#pragma once
#ifndef GROUP__INCLUDED
#define GROUP__INCLUDED

#include <stdint.h>
#include <stdlib.h>

/// The abstract type of a group element.
typedef void group_elem_t;

/// The abstract type of a group.
typedef struct group_struct group_t;

/// Initialize an element.
typedef void group_elem_init_f(group_t* group, group_elem_t* R);

/// Release any resources associated with an element.
typedef void group_elem_clear_f(group_t* group, group_elem_t* R);

/// Compute the 32-bit unsigned hash of a group element.
typedef uint32_t group_hash32_f(group_t* group, const group_elem_t* A);

/// Set the element to the identity element.
typedef void group_set_id_f(group_t* group, group_elem_t* R);

/// True if the element is the identity element.
typedef int group_is_id_f(group_t* group, const group_elem_t* A);

/// Set R=A.
typedef void group_set_f(group_t* group, group_elem_t* R, const group_elem_t* A);

/// True if A==B.
typedef int group_equal_f(group_t* group, const group_elem_t* A, const group_elem_t* B);

/// Compute R=R^(-1).
typedef void group_inverse_f(group_t* group, group_elem_t* R);

/// Compute R=A*B.
typedef void group_compose_f(group_t* group, group_elem_t* R, const group_elem_t* A, const group_elem_t* B);

/// Compute R=A^2.
typedef void group_square_f(group_t* group, group_elem_t* R, const group_elem_t* A);

/// Compute R=A^3.
typedef void group_cube_f(group_t* group, group_elem_t* R, const group_elem_t* A);

/// Print A to stdout using printf.
typedef void group_print_f(group_t* group, const group_elem_t* A);

/**
 * Define the operations available on a group and the size
 * of its elements.
 */
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


/// Allocate and initialize the memory for a single element.
static inline group_elem_t* group_elem_alloc(group_t* group) {
  void* res = malloc(group->elem_size);
  group->elem_init(group, res);
  return res;
}

/// Clear and free the memory for a single element.
static inline void group_elem_free(group_t* group, group_elem_t* elem) {
  group->elem_clear(group, elem);
  free(elem);
}

/// Returns a pointer to the specified index in the array.
static inline group_elem_t* group_elem_array_index(group_t* group,
                                                   group_elem_t* A,
                                                   const int index) {
  intptr_t p = (intptr_t)A;
  return (group_elem_t*)(p + index * group->elem_size);
}

/// Allocate a contiguous block of memory and initialize each group element.
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

/// Clear the memory for an array of elements and then free the block.
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

/**
 * Describe the relative costs of compose, square, and cube for a group.
 * Useful for exponentation and other algorithms that can choose
 * between these operations.
 */
typedef struct {
    double compose;
    double square;
    double cube;
} group_cost_t;

/// Each operation has the unit cost, 1.
extern const group_cost_t unit_costs;

/// Compose has a cost of 1. Square and Cube have a cost of 0.
extern const group_cost_t compose_only_costs;

#endif

