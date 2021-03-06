#ifndef HASHMAP_U64_INT_H
#define HASHMAP_U64_INT_H

#include <stdint.h>
#include "linkedlist.h"

typedef void (*u64_int_operate_fn)(uint64_t, int);

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
} hashmap_u64_int_t;

hashmap_u64_int_t *hm_u64_int_init(unsigned long);
int hm_u64_int_setup(hashmap_u64_int_t *, unsigned long);
int hm_u64_int_destroy(hashmap_u64_int_t *);
int hm_u64_int_teardown(hashmap_u64_int_t *);
int hm_u64_int_clear(hashmap_u64_int_t *);
/* add a (key,value) pair to the map. remaps the key if already present. */
int hm_u64_int_add(hashmap_u64_int_t *, uint64_t, int);
/* provided the key, return the value */
int hm_u64_int_get(hashmap_u64_int_t *, uint64_t);
/* given the key, return 1 if present, 0 if not */
int hm_u64_int_containskey(hashmap_u64_int_t *, uint64_t);
/* remove the key and its value from the map */
int hm_u64_int_removekey(hashmap_u64_int_t *, uint64_t);
/* perform an operation on each (key,value) pair */
int hm_u64_int_operate(hashmap_u64_int_t *, u64_int_operate_fn);

#endif
