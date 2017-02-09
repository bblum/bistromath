#ifndef HASHMAP_INT_INT_H
#define HASHMAP_INT_INT_H

#include <stdint.h>
#include "linkedlist.h"

typedef void (*int_int_operate_fn)(int, int);

#ifndef HASHMAP_DATA_NOT_PRESENT
#define HASHMAP_DATA_NOT_PRESENT (-1)
#endif

/**
 * A hashmap that maps unsigned 64 bit types as keys to integer values
 */
typedef struct {
	unsigned long size;
	unsigned long count;
	linkedlist_t *array;
} hashmap_int_int_t;

hashmap_int_int_t *hm_int_int_init(unsigned long);
int hm_int_int_setup(hashmap_int_int_t *, unsigned long);
int hm_int_int_destroy(hashmap_int_int_t *);
int hm_int_int_teardown(hashmap_int_int_t *);
int hm_int_int_clear(hashmap_int_int_t *);
/* add a (key,value) pair to the map. remaps the key if already present. */
int hm_int_int_add(hashmap_int_int_t *, int, int);
/* provided the key, return the value */
int hm_int_int_get(hashmap_int_int_t *, int);
/* given the key, return 1 if present, 0 if not */
int hm_int_int_containskey(hashmap_int_int_t *, int);
/* remove the key and its value from the map */
int hm_int_int_removekey(hashmap_int_int_t *, int);
/* perform an operation on each (key,value) pair */
int hm_int_int_operate(hashmap_int_int_t *, int_int_operate_fn);

#endif
