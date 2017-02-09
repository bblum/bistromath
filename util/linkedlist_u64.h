#ifndef LINKEDLIST_U64_H
#define LINKEDLIST_U64_H

#include <stdint.h>

/* Function pointer used for generic operations (i.e. printf) on all nodes */
typedef void (*u64_operate_fn)(uint64_t);

typedef struct node_u64_t {
        struct node_u64_t *next;
        uint64_t data;
} node_u64_t;

/**
 * This list is specially tuned to hold uint64_t's. Memory semantics are not
 * considered; uint64_t's are always passed by value (no dealing with any
 * void *'s ); therefore, the data is considered immutable. Uses standard
 * integer comparison in place of data_compare_fn from linkedlist.{c,h}. See
 * linkedlist.h for an in-depth description of how the data structure works.
 */
typedef struct {
	node_u64_t *head;
	node_u64_t *tail;
	unsigned long count;
} linkedlist_u64_t;

linkedlist_u64_t *ll_u64_init();
int ll_u64_setup(linkedlist_u64_t *);
int ll_u64_destroy(linkedlist_u64_t *);
int ll_u64_clear(linkedlist_u64_t *);
int ll_u64_add(linkedlist_u64_t *, uint64_t);
int ll_u64_add_index(linkedlist_u64_t *, unsigned long, uint64_t);
int ll_u64_remove(linkedlist_u64_t *, uint64_t);
uint64_t ll_u64_remove_index(linkedlist_u64_t *, unsigned long);
uint64_t ll_u64_get(linkedlist_u64_t *, unsigned long);
unsigned long ll_u64_search(linkedlist_u64_t *, uint64_t);
int ll_u64_operate(linkedlist_u64_t *, u64_operate_fn);
void ll_u64_append(linkedlist_u64_t *, linkedlist_u64_t *);

#endif
