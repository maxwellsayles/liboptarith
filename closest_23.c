#include <float.h>
#include <gmp.h>
#include <stdlib.h>

#include "liboptarith/closest_23.h"
#include "liboptarith/heap.h"
#include "liboptarith/math32.h"
#include "liboptarith/math_mpz.h"
#include "liboptarith/vector.h"

// two-three linked list node
struct ttll_struct;
typedef struct ttll_struct ttll_t;
struct ttll_struct {
  // term = sign * 2^a * 3^b
  int sign;
  int a;
  int b;
  
  // cost of chain so far
  // this value is cached. if it is FLT_MAX, then the cost should be computed
  double cost;
  const group_cost_t* costs;
  
  // remaining target
  mpz_t n;
  
  // pointer to the next term
  ttll_t* next;
  
  // number of pointers to this term
  int ref_count;
};

// (2,3) number engine
// holds free nodes
typedef struct {
  ttll_t* free_list;
  mpz_t v;
  mpz_t tmp;
  const group_cost_t* costs;
  heap_t heap;
  int k;
  vector_t chains;
} ttengine_t;

// return the cost of a chain
static double ttll_cost(const ttll_t* in_p) {
  // check if cost is cached
  if (in_p->cost != FLT_MAX) return in_p->cost;
  
  // nope, compute the cost
  const ttll_t* p = in_p;
  const group_cost_t* costs = in_p->costs;
  
  int a = 0;
  int b = 0;
  int n = 0;
  while (p) {
    if (p->a > a) a = p->a;
    if (p->b > b) b = p->b;
    n ++;
    p = p->next;
  }
  
  // store the cost, break const in this case
  if (n == 0) ((ttll_t*)in_p)->cost = 0;
  else ((ttll_t*)in_p)->cost =
	 costs->compose*(n-1) + costs->square*a + costs->cube*b;
  
  return in_p->cost;
}

// compare the remaining number of both chains
// if they are equal, then compare the costs
static int ttll_cmpabs(ttll_t* x, ttll_t* y) {
  int res = mpz_cmpabs(x->n, y->n);
  if (res == 0) {
    double costx = ttll_cost(x);
    double costy = ttll_cost(y);
    if (costx < costy) return -1;
    if (costx > costy) return 1;
  }
  return res;
}

// create empty free node list
static void ttengine_init(ttengine_t* engine, const group_cost_t* costs, int k) {    
  engine->free_list = 0;
  mpz_init(engine->v);
  mpz_init(engine->tmp);
  engine->costs = costs;
  heap_init(&engine->heap, (heap_cmp_f*)&ttll_cmpabs);
  engine->k = k;
  vector_init(&engine->chains, k);
}

/**
 * free the memory for all nodes in the free list
 */
static void ttengine_clear(ttengine_t* engine) {
  vector_clear(&engine->chains);
  heap_clear(&engine->heap);
  mpz_clear(engine->v);
  mpz_clear(engine->tmp);
  
  ttll_t* current = engine->free_list;
  ttll_t* next;
  while (current) {
    next = current->next;
    mpz_clear(current->n);
    free(current);
    current = next;
  }
  engine->free_list = 0;
}

/**
 * return the length of a linked list.
 */
static int ttll_length(ttll_t* p) {
  int n = 0;
  while (p) {
    p = p->next;
    n ++;
  }
  return n;
}

/**
 * Create a node in the linked list.
 */
static inline ttll_t* ttll_alloc(ttengine_t* engine, 
                                 const int sign,
                                 const int a,
                                 const int b,
                                 const mpz_t n,
                                 ttll_t* next) {
  ttll_t* res = 0;
  
  if (engine->free_list) {
    // there is a node on the free list, use it
    res = engine->free_list;
    engine->free_list = res->next;
  } else {
    // no node on the free list, allocate a new one
    res = (ttll_t*)malloc(sizeof(ttll_t));
    mpz_init(res->n);
  }
  
  res->sign = sign;
  res->a = a;
  res->b = b;
  res->cost = FLT_MAX; // initially no cost is computed
  res->costs = engine->costs;
  mpz_set(res->n, n);
  res->next = next;
  if (next) next->ref_count ++;
  res->ref_count = 1;
  
  return res;
}

/**
 * increment the reference count of a node
 */
static inline void ttll_incref(ttengine_t* engine, ttll_t* p) {
  p->ref_count ++;
}

/**
 * decrement the reference count of a node,
 * and possible release memory, and recurse.
 */
static inline void ttll_decref(ttengine_t* engine, ttll_t* p) {
  ttll_t* next;
  while (p) {
    p->ref_count --;
    if (p->ref_count > 0) return;
    
    // reference count is zero, release this node to the free list
    next = p->next;
    p->next = engine->free_list;
    engine->free_list = p;
    
    // point to the next node in the list
    p = next;
  }
}

/**
 * convert a list to an array
 */
static two_three_term_t* ttll_to_array(int* term_count, ttll_t* p) {
  *term_count = ttll_length(p);
  two_three_term_t* res =
    (two_three_term_t*)malloc(sizeof(two_three_term_t) * (*term_count));
  int i = 0;
  
  while (p) {
    res[i].a = p->a;
    res[i].b = p->b;
    res[i].sign = p->sign;
    
    i ++;
    p = p->next;
  }
  
  return res;
}

// print a chain
static void ttll_print(const ttll_t*) __attribute__ ((unused));
static void ttll_print(const ttll_t* p) {
  if (!p) {
    printf("0");
    return;
  }
  
  double cost = ttll_cost(p);
  
  gmp_printf("%Zd ", p->n);
  while (p) {
    if (p->sign == -1) {
      gmp_printf("-");
    } else if (p->sign == 1) {
      gmp_printf("+");
    } else {
      gmp_printf("unknown sign");
      exit(-1);
    }
    gmp_printf("2^%d*3^%d ", p->a, p->b);
    p = p->next;
  }
  
  printf(" cost=%f", cost);
}

/**
 * Iterates a from min(max_a, floor(log_2(n))) to 0
 * Iterates b from 0 to min(max_b, ceil(log_3(n)))
 * Adds 2^a*3^b and 2^{a+1}*3^b to the heap.
 */
static void update_closest(ttengine_t* engine,
                           ttll_t* current,
                           const mpz_t in_n,
                           const int max_a,
                           const int max_b) {
  int sign;
  int a;
  int b;
  int B;
  ttll_t* node;
  
  if (mpz_cmp_ui(in_n, 0) == 0) {
    // already zero. just put this chain back into the heap
    if (current) {
      ttll_incref(engine, current);
      node = heap_add_bounded(&engine->heap, current, engine->k);
      ttll_decref(engine, node);
    }
    return;
  }
  
  // take the sign of the number
  sign = mpz_sgn(in_n);
  
  // iterate over each b from 0 to min(ceil(log_3(n)), max_b)
  
  // A = min(ceil(log_2(n))-1, max_a)
  a = mpz_sizeinbase(in_n, 2)-1;
  if (a > max_a-1) a = max_a-1;
  if (a < 0) a = 0;
  // iterate 'a' from A > a >= 0
  
  // B = min(ceil(log_3(n)), max_b)
  B = mpz_sizeinbase(in_n, 3);
  if (B > max_b) B = max_b;
  // iterate 'b' from 0 <= b <= B
  
  // set v = 2^a
  mpz_set_ui(engine->v, 1);
  mpz_mul_2exp(engine->v, engine->v, a);
  
  // iterate b from 0 to B
  b = 0;
  while (b <= B) {
    // case 2^a*3^b < n
    if (in_n->_mp_size < 0) {
      // negative
      mpz_add(engine->tmp, in_n, engine->v);
    } else {
      // positive
      mpz_sub(engine->tmp, in_n, engine->v);
    }
    node = ttll_alloc(engine, sign, a, b, engine->tmp, current);
    node = heap_add_bounded(&engine->heap, node, engine->k);
    ttll_decref(engine, node);
    
    // case 2^{a+1}*3^b > n
    if (in_n->_mp_size < 0) {
      // negative
      mpz_add(engine->tmp, engine->tmp, engine->v);
    } else {
      // positive
      mpz_sub(engine->tmp, engine->tmp, engine->v);
    }
    node = ttll_alloc(engine, sign, a+1, b, engine->tmp, current);
    node = heap_add_bounded(&engine->heap, node, engine->k);
    ttll_decref(engine, node);
    
    // increment b
    b ++;
    mpz_mul_ui(engine->v, engine->v, 3);
    
    // while v >= n, divide by 2 and reduce a
    while (mpz_cmpabs(engine->v, in_n) >= 0 && a > 0) {
      mpz_tdiv_q_2exp(engine->v, engine->v, 1);
      a --;
    }
  }
}

/**
 * True if any elements in the heap have non-zero cost.
 * Since this is a max heap based on the remainder of the chain, if any
 * elements have non-zero remainder, then the maximum element will have
 * non-zero cost. Therefore, we only need check the maximum element.
 */
static int any_not_zero(const heap_t* h) {
  if (h->size == 0) return 0;
  ttll_t* node = (ttll_t*)h->elems[0];
  return mpz_cmp_ui(node->n, 0) != 0;
}

/**
 * Return the cheapest representation given bounds on a and b
 */
static ttll_t* rep_prune_closest_bound_ab(ttengine_t* engine,
                                          const mpz_t x,
                                          const int max_a,
                                          const int max_b) {
  int i = 0;
  double cost = 0;
  double best_cost = FLT_MAX;
  int best_rep_i = 0;
  ttll_t* node = 0;
  
  // Stuff some initial chains into the heap
  update_closest(engine, 0, x, max_a, max_b);
  
  while (any_not_zero(&engine->heap)) {
    // Find the best cost complete chain, if one exists.
    best_cost = FLT_MAX;
    for (i = 0;  i < engine->heap.size;  i ++) {
      node = engine->heap.elems[i];
      if (mpz_cmp_ui(node->n, 0) == 0) {
	cost = ttll_cost(node);
	if (cost < best_cost) best_cost = cost;
      }
    }
    
    // Move heap chains into vector
    // if their cost is less than the best complete chain, if one exists.
    // We do this because if there is a complete chain,
    // than any incomplete chain with higher cost cannot possibly be
    // cheaper once it is complete, and any complete chain is finished
    // and we're only interested in the cheapest cost one.
    for (i = 0;  i < engine->heap.size;  i ++) {
      node = engine->heap.elems[i];
      cost = ttll_cost(node);
      if (cost <= best_cost) {
	vector_push_back(&engine->chains, node);
      } else {
	ttll_decref(engine, node);
      }
    }
    heap_empty(&engine->heap);
    
    // For each element in the vector, find all the (2,3) numbers near it
    // decrement its reference count.
    while (engine->chains.size > 0) {
      node = vector_pop_back(&engine->chains);
      update_closest(engine, node, node->n, max_a, max_b);
      ttll_decref(engine, node);
    }
  }
  
  // Iterate over each list in the heap and keep the one with lowest cost.
  best_cost = FLT_MAX;
  for (i = 0;  i < engine->heap.size;  i ++) {
    cost = ttll_cost(engine->heap.elems[i]);
    if (cost < best_cost) {
      best_cost = cost;
      best_rep_i = i;
    }
  }
  
  // Increment the reference associated with the best list.
  node = engine->heap.elems[best_rep_i];
  ttll_incref(engine, node);
  
  // Decrement the reference count on each heap entry
  // this essentially frees all lists, except the cheapest list.
  for (i = 0;  i < engine->heap.size;  i ++) {
    ttll_decref(engine, engine->heap.elems[i]);
  }
  heap_empty(&engine->heap);
  
  return node;
}

// For fixed b, computes the max_a, and returns the cheapest rep.
static ttll_t* rep_prune_closest_bound_b(ttengine_t* engine, const mpz_t x, const int max_b) {
  // compute ceil(log_2(n/(3^b)))
  mpz_ui_pow_ui(engine->tmp, 3, max_b);
  mpz_tdiv_q(engine->tmp, x, engine->tmp);
  int max_a = mpz_sizeinbase(engine->tmp, 2);
  
  // compute cheapest chain given max_a and max_b
  return rep_prune_closest_bound_ab(engine, x, max_a, max_b);
}

/**
 * Repeatedly find the 2^a*3^b closest to the remaining amount.
 * Return an array of (2,3) terms equal to x.
 * 
 * The representation is generated by repeatedly finding the k closest 2^a*3^b
 * when we have k reps for x, we return the least costly.
 */
two_three_term_t* rep_prune_closest(int* term_count, const mpz_t x, const group_cost_t* costs, const int keep_count) {
  ttengine_t engine;
  two_three_term_t* res = 0;
  
  ttengine_init(&engine, costs, 1);
  
  int B = mpz_sizeinbase(x, 3);
  
  ttll_t* best_chain = 0;
  double best_cost = FLT_MAX;
  
  int blo = 0;
  int bhi = B;
  
  while (blo < bhi) {
    int bdif = (bhi-blo)/3;
    int bmid1 = blo + bdif;
    int bmid2 = bhi - bdif;
    
    // Sample the cost at each third.
    ttll_t* chain = rep_prune_closest_bound_b(&engine, x, bmid1);
    double cost1 = ttll_cost(chain);
    ttll_decref(&engine, chain);
    
    chain = rep_prune_closest_bound_b(&engine, x, bmid2);
    double cost2 = ttll_cost(chain);
    ttll_decref(&engine, chain);
    
    // throw away the most expensive third
    if (cost1 > cost2) {
      if (blo == bmid1) break;
      blo = bmid1;
    } else {
      if (bhi == bmid2) break;
      bhi = bmid2;
    }
  }

  // recreate the two-three engine using a larger number of leaf nodes
  ttengine_clear(&engine);
  ttengine_init(&engine, costs, keep_count);
  
  // this is the bmax where we search from
  int bdist = msb_u32(B);
  if (bdist < 8) bdist = 8; // a minimum search around the noisiness
  
  // bound the low and high search points
  blo -= bdist;
  if (blo < 0) blo = 0;
  bhi += bdist;
  if (bhi > B) bhi = B;
  
  int b;
  for (b = blo;  b <= bhi;  b ++) {
    ttll_t* chain = rep_prune_closest_bound_b(&engine, x, b);
    double cost = ttll_cost(chain);
    if (cost < best_cost) {
      ttll_decref(&engine, best_chain);
      best_cost = cost;
      best_chain = chain;
    } else {
      ttll_decref(&engine, chain);
    }
  }
  
  // here we should have the cheapest chain  
  // convert the chain to an array
  res = ttll_to_array(term_count, best_chain);
  ttll_decref(&engine, best_chain);
  
  ttengine_clear(&engine);
  
  return res;
}

/**
 * Print a two three representation.
 */
void print_two_three_terms(const two_three_term_t* terms, const int term_count) {
  int i;
  for (i = 0; i < term_count; i ++) {
    printf("%c", terms[i].sign == -1 ? '-' : '+');
    printf("2^%d*3^%d", terms[i].a, terms[i].b);
    if (i + 1 < term_count) {
      printf(" ");
    }
  }
}

/**
 * Compare two 2,3 terms based on the '2' component.
 */
int compare_two_three_terms(const void* A, const void* B) {
  const two_three_term_t* S = (const two_three_term_t*)A;
  const two_three_term_t* T = (const two_three_term_t*)B;
  if (S->a < T->a) return -1;
  if (S->a > T->a) return 1;
  if (S->b < T->b) return -1;
  if (S->b > T->b) return 1;
  return 0;
}

/**
 * Computes the factored representation of a 2,3 number.
 * Caller must 'free()' the returned array.
 * The two-three representation passed in is also sorted by the 'a' component.
 */
factored_two_three_term16_t* factored_rep(int* term_count,
                                          two_three_term_t* rep,
                                          int rep_count) {
  factored_two_three_term16_t* res;
  int i;
  int j;
  
  // Sort the representation by the 2 term.
  qsort(rep, rep_count, sizeof(two_three_term_t), compare_two_three_terms);
  
  // Allocate the result array.
  res = (factored_two_three_term16_t*)malloc(
            sizeof(factored_two_three_term16_t) * rep_count);
  if (!res) return 0;
  
  // Compute the first n-1 terms of the factored representation.
  j = 0;
  for (i = rep_count - 1; i > 0; i --) {
    res[j].a = rep[i].a - rep[i-1].a;
    res[j].b = rep[i].sign < 0 ? rep[i].b | (1<<15) : rep[i].b;
    j ++;
  }
  
  // Compute the last term.
  res[j].a = rep[0].a;
  res[j].b = rep[0].sign < 0 ? rep[0].b | (1<<15) : rep[0].b;
  
  *term_count = rep_count;
  
  return res;
}

/**
 * Print a factored two three 16bit representation.
 */
void print_factored_two_three_term16(const factored_two_three_term16_t* terms, const int term_count) {
  int i;
  for (i = 0; i < term_count; i ++) {
    printf("(");
  }
  for (i = 0; i < term_count; i ++) {
    if (terms[i].b & (1<<15)) {
      printf("-");
    } else {
      printf("+");
    }
    printf("3^%d)*2^%d", terms[i].b & ((1<<15)-1), terms[i].a);
  }
}

/**
 * Returns a 2,3 16 bit factored representation of the number x
 * using a gready k-closest algorithm.
 * 
 * The caller is responsible for the lifetime of the returned object.
 * 
 * This function essentially calls rep_prune_closest()
 * and then factored_rep().
 */
factored_two_three_term16_t* factored_rep_prune_closest(
    int* term_count,
    const mpz_t x,
    const group_cost_t* costs,
    const int keep_count) {
  int count = 0;
  two_three_term_t* rep = rep_prune_closest(&count, x, costs, keep_count);
  factored_two_three_term16_t* terms =
      factored_rep(term_count, rep, count);
  free(rep);
  return terms;
}

/*
// sums up a chain
static void ttll_sum(const ttll_t* p, mpz_t res) {
  mpz_t tmp;
  mpz_init(tmp);
  mpz_set(res, p->n);
  while (p) {
    // add this term to res
    mpz_ui_pow_ui(tmp, 3, p->b);
    mpz_mul_2exp(tmp, tmp, p->a);
    if (p->sign == -1) {
      mpz_sub(res, res, tmp);
    } else {
      mpz_add(res, res, tmp);
    }

    // next node
    p = p->next;
  }
  mpz_clear(tmp);
}

// Verify that the sum of the chain is equal to x
// Print the chain and terminate if it is not.
static void ttll_sanity(const ttll_t* p, const mpz_t x) {
  mpz_t sum;
  mpz_init(sum);
  ttll_sum(p, sum);
  if (mpz_cmp(sum, x) != 0) {
    // chain is not sane
    gmp_printf("Insane chain!\n");
    gmp_printf("x=%Zd\n", x);
    if (!p) {
      gmp_printf("p is null\n");
      exit(-1);
    }
    gmp_printf("p=");
    ttll_print(p);
    gmp_printf("\n");
    exit(-1);
  }
  mpz_clear(sum);
}
*/
