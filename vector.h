/**
 * @file vector.h
 * 
 * Doubling stack with amortized O(1) push/pop and O(1) random access.
 * Essentially this is std::vector but usuable from standard C.
 */

#pragma once
#ifndef VECTOR__INCLUDED
#define VECTOR__INCLUDED

#include <stdlib.h>

typedef struct {
  void** elems;
  int size;
  int capacity;
} vector_t;

/**
 * Vector constructor.
 */
static inline void vector_init(vector_t* v, int capacity) {
  v->capacity = capacity;
  v->size = 0;
  v->elems = (void**)malloc(sizeof(void*)*capacity);
}

/**
 * Vector destructor.
 */
static inline void vector_clear(vector_t* v) {
  v->capacity = 0;
  v->size = 0;
  free(v->elems);
  v->elems = 0;
}

/**
 * Empty a vector without changing its capacity.
 */
static inline void vector_empty(vector_t* v) {
  v->size = 0;
}

/**
 * Guarantee that the vector's capacity is at least 'capacity'.
 */
static inline void vector_reserve(vector_t* v, int capacity) {
  void** new_elems;
  int i;
  if (v->capacity >= capacity) {
    return;
  }
  new_elems = (void**)malloc(sizeof(void*)*capacity);
  for (i = 0;  i < v->size;  i ++) {
    new_elems[i] = v->elems[i];
  }
  free(v->elems);
  v->elems = new_elems;
  v->capacity = capacity;
}

/**
 * Add an element to the back of the vector in O(1) amortized time.
 */
static inline void vector_push_back(vector_t* v, void* elem) {
  if (v->size == v->capacity) {
    vector_reserve(v, v->capacity << 1);
  }
  v->elems[v->size] = elem;
  v->size ++;
}

/**
 * Remove an element from the back of a vector.
 * Does not resize the capacity of a vector.
 */
static inline void* vector_pop_back(vector_t* v) {
  if (v->size == 0) return 0;
  v->size --;
  return v->elems[v->size];
}

/**
 * Return the element at the back of the vector.
 */
static inline void* vector_back(vector_t* v) {
  if (v->size == 0) return 0;
  return v->elems[v->size - 1];
}

#endif

