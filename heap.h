/**
 * @file heap.h
 * Max-heap (Priority Queue) for generic objects.
 */

#pragma once
#ifndef HEAP__INCLUDED
#define HEAP__INCLUDED

#include <stdlib.h>

/**
 * The heap comparison function.
 * Returns -1 if a < b, 0 if a == b, and 1 otherwise
 */
typedef int heap_cmp_f(void* a, void* b);

/**
 * Heap structure.
 */
typedef struct {
  heap_cmp_f* cmp;
  void** elems;
  int size;
  int capacity;
} heap_t;

/// @private Left child of i.
static inline int heap_left_child(int i) {
  return (i << 1) + 1;
}

/// @private Right child of i.
static inline int heap_right_child(int i) {
  return (i << 1) + 2;
}

/// @private Parent of i.
static inline int heap_parent(int i) {
  return (i - 1) >> 1;
}

/// @private Swap elements i and j.
static inline void heap_swap(heap_t* h, int i, int j) {
  void* tmp = h->elems[i];
  h->elems[i] = h->elems[j];
  h->elems[j] = tmp;
}

/// @private Restore the heap property by moving an element up.
static inline void heapify_up(heap_t* h, int i) {
  int p;
  if (i == 0) return;
  p = heap_parent(i);
  if (h->cmp(h->elems[i], h->elems[p]) <= 0) return;

  // swap i and p and then heapify p
  heap_swap(h, i, p);
  heapify_up(h, p);
}

/// @private Restore the heap property by moving an element down.
static inline void heapify_down(heap_t* h, int i) {
  int lcmp;
  int rcmp;
  int l = heap_left_child(i);
  int r = l+1;
  
  // no left child, guaranteed no right child, just return
  if (l >= h->size) return;
  
  // no right child, test if element is less than left child
  if (r >= h->size) {
    if (h->cmp(h->elems[i], h->elems[l]) < 0) {
      // swap i with l, recurse on l
      heap_swap(h, i, l);
      return heapify_down(h, l);
    }
    return;
  }
  
  // both left and right children exist
  lcmp = h->cmp(h->elems[i], h->elems[l]);
  rcmp = h->cmp(h->elems[i], h->elems[r]);
  
  // if i < l and i < r then swap with larger of l and r
  if (lcmp < 0 && rcmp < 0) {
    if (h->cmp(h->elems[l], h->elems[r]) > 0) {
      // swap with left
      heap_swap(h, i, l);
      return heapify_down(h, l);
    } else {
      // swap with right
      heap_swap(h, i, r);
      return heapify_down(h, r);
    }
  }
  
  if (lcmp < 0) {
    // swap with left
    heap_swap(h, i, l);
    return heapify_down(h, l);
  }
  
  if (rcmp < 0) {
    // swap with right
    heap_swap(h, i, r);
    return heapify_down(h, r);
  }
}

/**
 * Initialize a heap.
 */
static inline void heap_init(heap_t* h, heap_cmp_f* cmp) {
  h->cmp = cmp;
  h->size = 0;
  h->capacity = 15;
  h->elems = (void**)malloc(sizeof(void*)*h->capacity);
}

/**
 * Destroy a heap.
 */
static inline void heap_clear(heap_t* h) {
  h->cmp = 0;
  h->size = 0;
  h->capacity = 0;
  free(h->elems);
  h->elems = 0;
}

/**
 * Empty the heap.
 */
static inline void heap_empty(heap_t* h) {
  h->size = 0;
}

/// @private Double the capacity of the heap.
static inline void heap_grow(heap_t* h) {
  int new_capacity = (h->capacity << 1) + 1;
  void** new_elems = (void**)malloc(sizeof(void*)*new_capacity);
  int i;
  for (i = 0;  i < h->size;  i ++) {
    new_elems[i] = h->elems[i];
  }
  free(h->elems);
  h->elems = new_elems;
  h->capacity = new_capacity;
}

/**
 * Return the maximum element of the heap.
 */
static inline void* heap_get_max(heap_t* h) {
  if (h->size == 0) return 0;
  return h->elems[0];
}

/**
 * Remove the maximum element from the heap.
 * @return The maximum element.
 */
static inline void* heap_remove_max(heap_t* h) {
  void* res = h->elems[0];
  
  // base cases
  if (h->size == 0) return 0;
  if (h->size == 1) {
    h->size = 0;
    return res;
  }
  
  // swap the last element with the top and heapify down
  h->size --;
  h->elems[0] = h->elems[h->size];
  heapify_down(h, 0);
  return res;
}

/**
 * Add an element to the heap.
 */
static inline void heap_add(heap_t* h, void* elem) {
  if (h->size == h->capacity) {
    heap_grow(h);
  }
  h->elems[h->size] = elem;
  h->size ++;
  heapify_up(h, h->size-1);
}

/**
 * Add an element to heap, removing the largest element once the heap
 * reaches a maximum size.
 */
static inline void* heap_add_bounded(heap_t* h, void* elem, int max_size) {
  void* res = 0;

  // is heap maximum size?
  if (h->size == max_size) {
    // is the element we want to add any smaller?
    if (h->cmp(elem, h->elems[0]) >= 0) return elem; // no, it's bigger
    
    // yes it's smaller
    res = h->elems[0];
    h->elems[0] = elem;
    heapify_down(h, 0);
    return res;
  }
  
  // heap is not maximum size, add the element the normal way
  heap_add(h, elem);
  return 0;
}

#endif

