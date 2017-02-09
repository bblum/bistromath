#ifndef LINKEDLIST_H
#define LINKEDLIST_H

/* Function pointer used to compare two elements */ 
typedef int (*data_compare_fn)(void *, void *);
/* Function pointer used to destroy an element */ 
typedef void (*data_destroy_fn)(void *);
/* Function pointer used for generic operations (i.e. printf) on all nodes */
typedef void (*data_operate_fn)(void *);

typedef struct node_t {
        struct node_t *next;
        void *data;
} node_t;

/**
 * Linkedlist data structure, singly linked with a tail pointer. Keeps track
 * of the number of elements in an unsigned long - therefore if more than
 * UNSIGNED_LONG_MAX items are stored in the list, things will break (note
 * also that this means that the *index* of the last element is greater than
 * UL_MAX - 1 things will break - for example, search() will return -1 which
 * is also UL_MAX as an *index* if an item is not present).
 * For stack functionality, use add_index(0) and remove_index(0); for queue
 * use add() and remove_index(0). All four of these guarantee O(1) (note,
 * remove from end is O(n)).
 */
typedef struct {
	node_t *head;
	node_t *tail;
	data_compare_fn data_compare;
	data_destroy_fn data_destroy;
	unsigned long count;
} linkedlist_t;

/* Initialize a linkedlist; returns a pointer to the new linkedlist */
linkedlist_t *ll_init(data_compare_fn, data_destroy_fn);
/* Set up a linkedlist given already initialized space */
int ll_setup(linkedlist_t *, data_compare_fn, data_destroy_fn);
/* destroys the linkedlist, freeing all nodes inside */
int ll_destroy(linkedlist_t *);
/* removes all nodes from the list */
int ll_clear(linkedlist_t *);
/* Add an item to the end of the list. O(1) */
int ll_add(linkedlist_t *, void *);
/* Insert at a specified index. O(index)  */
int ll_add_index(linkedlist_t *, unsigned long, void *);
/* Remove given the data pointer. O(n) */
int ll_remove(linkedlist_t *, void *);
/* Remove given the index. O(index) */
void *ll_remove_index(linkedlist_t *, unsigned long);
/* Get the data given an index. O(index) */
void *ll_get(linkedlist_t *, unsigned long);
/* Get the index given the data. O(n) */
unsigned long ll_search(linkedlist_t *, void *);
/* Perform a function on every item in the list */
int ll_operate(linkedlist_t *, data_operate_fn);

#endif
