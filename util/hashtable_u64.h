#ifndef HASHTABLE_H
#define HASHTABLE_H

#ifdef HT_CHECK_MALLOC_FAIL
#define LL_CHECK_MALLOC_FAIL
#endif
#include "linkedlist_u64.h"

typedef struct {
	unsigned long size;
	unsigned long count;
	linkedlist_u64_t *array;
} hashtable_u64_t;

/* Initialize a hashtable; returns a pointer to the new hashtable */
hashtable_u64_t *ht_u64_init(unsigned long);
/* set up a hashtable in a piece of already alloc'd space */
int ht_u64_setup(hashtable_u64_t *, unsigned long);
/* Destroy a hashtable, destroying all the underlying linkedlists */
int ht_u64_destroy(hashtable_u64_t *);
/* Free all memory besides the struct itself */
int ht_u64_teardown(hashtable_u64_t *);
/* Remove all elements from a hashtable */
int ht_u64_clear(hashtable_u64_t *);
/* Add an item to the hashtable. O(1) */
int ht_u64_add(hashtable_u64_t *, uint64_t);
/* Check if the hashtable contains the given element O(1) */
int ht_u64_contains(hashtable_u64_t *, uint64_t);
/* Remove an item from the hashtable. O(1) */
int ht_u64_remove(hashtable_u64_t *, uint64_t);
/* Perform a function on every item in the hashtable. O(n) */
int ht_u64_operate(hashtable_u64_t *, u64_operate_fn);

#endif
