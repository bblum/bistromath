#ifndef LINKEDLIST_U32_H
#define LINKEDLIST_U32_H

#include <stdint.h>

/* Function pointer used for generic operations (i.e. printf) on all nodes */
typedef void (*u32_operate_fn)(uint32_t);

typedef struct node_u32_t {
        struct node_u32_t *next;
        uint32_t data;
} node_u32_t;

/**
 * This list is specially tuned to hold uint32_t's. Memory semantics are not
 * considered; uint32_t's are always passed by value (no dealing with any
 * void *'s ); therefore, the data is considered immutable. Uses standard
 * integer comparison in place of data_compare_fn from linkedlist.{c,h}. See
 * linkedlist.h for an in-depth description of how the data structure works.
 */
typedef struct {
	node_u32_t *head;
	node_u32_t *tail;
	unsigned long count;
} linkedlist_u32_t;

linkedlist_u32_t *ll_u32_init();
int ll_u32_setup(linkedlist_u32_t *);
int ll_u32_destroy(linkedlist_u32_t *);
int ll_u32_clear(linkedlist_u32_t *);
int ll_u32_add(linkedlist_u32_t *, uint32_t);
int ll_u32_add_index(linkedlist_u32_t *, unsigned long, uint32_t);
int ll_u32_remove(linkedlist_u32_t *, uint32_t);
uint32_t ll_u32_remove_index(linkedlist_u32_t *, unsigned long);
uint32_t ll_u32_get(linkedlist_u32_t *, unsigned long);
unsigned long ll_u32_search(linkedlist_u32_t *, uint32_t);
int ll_u32_operate(linkedlist_u32_t *, u32_operate_fn);
void ll_u32_append(linkedlist_u32_t *, linkedlist_u32_t *);

#endif
