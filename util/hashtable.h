#ifndef HASHTABLE_H
#define HASHTABLE_H

#ifdef HT_CHECK_MALLOC_FAIL
#define LL_CHECK_MALLOC_FAIL
#endif
#include "linkedlist.h"

typedef unsigned long (*data_hash_fn)(void *, unsigned long);

typedef struct {
	unsigned long size; /* number of buckets */
	unsigned long count; /* current number of elements */
	data_hash_fn data_hash;
	linkedlist_t *array;
} hashtable_t;

/* Initialize a hashtable; returns a pointer to the new hashtable */
hashtable_t *ht_init(unsigned long, data_compare_fn, data_destroy_fn,
            data_hash_fn);
/* Setup a hashtable given already alloc'd space */
int ht_setup(hashtable_t *, unsigned long, data_compare_fn,
             data_destroy_fn, data_hash_fn);
/* Destroy a hashtable, destroying all the underlying linkedlists */
int ht_destroy(hashtable_t *);
/* Tear down a hashtable made with ht_setup */
int ht_teardown(hashtable_t *);
/* Remove all elements from a hashtable */
int ht_clear(hashtable_t *);
/* Add an item to the hashtable. O(1) */
int ht_add(hashtable_t *, void *);
/* Check if the hashtable contains the given element O(1) */
int ht_contains(hashtable_t *, void *);
/* Remove an item from the hashtable. O(1) */
int ht_remove(hashtable_t *, void *);
/* Perform a function on every item in the hashtable. O(n) */
int ht_operate(hashtable_t *, data_operate_fn);

#endif
