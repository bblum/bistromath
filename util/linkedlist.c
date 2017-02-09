#include <stdlib.h>
#ifdef LL_CHECK_MALLOC_FAIL
#include <stdio.h>
#endif
#include "linkedlist.h"

static node_t *node_init();
static int node_destroy(node_t *, data_destroy_fn);
static void do_nothing(void *);

/**
 * @brief Initialize a linked list.
 * @param data_compare a function pointer that takes two void * pointers and
 *        compares them, returning <0, 0, or >0 accordingly.
 * @param data_destroy a function pointer that, given a void *, destroys the
 *        element
 * @return a pointer to the new linkedlist, or returns NULL if either pointer
 *         is null. Returns NULL and prints an error message if malloc fails.
 */
linkedlist_t *ll_init(data_compare_fn data_compare, data_destroy_fn data_destroy)
{
	linkedlist_t *ll;

	if (data_compare == NULL || data_destroy == NULL)
	{
		return NULL;
	}
	
	ll = malloc(sizeof(linkedlist_t));
	#ifdef LL_CHECK_MALLOC_FAIL
	if (ll == NULL)
	{
		perror("LL_INIT: Malloc failed");
		return NULL;
	}
	#endif

	ll_setup(ll, data_compare, data_destroy);

	return ll;
}

/**
 * @brief Setup a linkedlist given already alloc'd space. 
 * @param ll a pointer to memory in which the linkedlist will be set up
 * @param data_compare see ll_init
 * @param data_destroy see ll_init
 * @return 1 if any of the pointers are null, 0 on success.
 */
int ll_setup(linkedlist_t *ll, data_compare_fn data_compare,
             data_destroy_fn data_destroy)
{
	if (ll == NULL || data_compare == NULL || data_destroy == NULL)
	{
		return 1;
	}

	ll->head = NULL;
	ll->data_compare = data_compare;
	ll->data_destroy = data_destroy;
	ll->count = 0;
	
	return 0;
	
}


/**
 * @brief Destroy a linkedlist. Runs in O(n) time (if data_destroy is O(1))
 * @param list a pointer to memory in which the linkedlist exists
 * @return 0 on success, -1 if the pointer is null.
 */
int ll_destroy(linkedlist_t *list)
{
	if (!list)
	{
		return -1;
	}

	ll_clear(list);
	free(list);

	return 0;
}

/**
 * @brief remove all elements from the list. O(n) time.
 * @param list a pointer to memory in which the linkedlist exists
 * @return 0 on success, -1 if the pointer is null
 */
int ll_clear(linkedlist_t *list)
{
	node_t *nextnode;
	if (!list)
	{
		return -1;
	}

	while (list->head)
	{
		nextnode = list->head->next;
		node_destroy(list->head, list->data_destroy);
		list->head = nextnode;
	}
	list->count = 0;
	/* list->head is already null */
	list->tail = NULL;

	return 0;
}

/**
 * @brief Add an element to the end of the list. Runs in O(1) time. Does not
 *        check for duplicate entries.
 * @param list a pointer to memory in which the linkedlist exists
 * @param data the data to add
 * @return 0 on success, -1 if the list pointer is null.
 */
int ll_add(linkedlist_t *list, void *data)
{
	node_t *newnode;

	if (!list)
	{
		return -1;
	}
	
	newnode = node_init();
	newnode->data = data;

	if (!list->head)
	{
		list->head = newnode;
	}
	else
	{
		list->tail->next = newnode;
	}
	
	list->tail = newnode;
	list->count++;

	return 0;
}

/**
 * @brief Insert an element into the list at the specified index. If the index
 *        is beyond the end of the list, this will simply append the item to
 *        the end of the list. Runs in O(index) time, unless index is equal to
 *        (or greater than) the number of elements in the list, then O(1).
 * @param list a pointer to memory in which the linkedlist exists
 * @param index where to add the data
 * @param data the data to be added
 * @return -1 if the pointer is null, 0 on success
 */
int ll_add_index(linkedlist_t *list, unsigned long index, void *data)
{
	node_t *newnode;

	if (!list)
	{
		return -1;
	}
	
	newnode = node_init();
	newnode->data = data;

	/* special case - empty list; just add the node onto the list */
	if (!list->head)
	{
		list->head = newnode;
		list->tail = newnode;
	}
	/* special case - add to the front of a nonempty list */
	else if (index == 0)
	{
		newnode->next = list->head;
		list->head = newnode;
	}
	/* special case - adding to the end of the list */
	else if (index >= list->count)
	{
		list->tail->next = newnode;
		list->tail = newnode;
	}
	/* here we iterate through the list 
	 * we are guaranteed at this point 0 < index < count */
	else
	{
		node_t *prev = list->head;
		unsigned long i;
		/* move prev to the node before where we want to add */
		for (i = 0; i < index - 1; i++)
		{
			/* prev won't be null due to our guarantee */
			prev = prev->next;
		}
		newnode->next = prev->next;
		prev->next = newnode;
	}
	list->count++;
	return 0;
}

/**
 * @brief Remove an item from the list. Runs in O(n) time.
 * @param list a pointer to memory in which the linkedlist exists
 * @param data the data to be removed
 * @return 0 on success, 1 if the item is not present in the list, -1 if the
 *         list pointer is null.
 */
int ll_remove(linkedlist_t *list, void *data)
{
	node_t *prev;
	node_t *next;

	/* null list */
	if (!list)
	{
		return -1;
	}
	/* empty list */
	if (list->count == 0)
	{
		return 1;
	}
	
	/* Edge case: removing the first item in the list - we won't be able
	 * to move prev to the one behind it, because there's no one behind */
	if (0 == (*(list->data_compare))(list->head->data, data))
	{
		next = list->head->next;
		node_destroy(list->head, list->data_destroy);
		list->head = next;
	}
	/* Typical case */
	else
	{
		prev = list->head;
		/* while prevnode's next isn't what we're looking for - also,
		 * watch out for hitting the end of the list */
		while (prev->next &&
		       0 != (*(list->data_compare))(prev->next->data, data))
		{
			prev = prev->next;
		}
		/* end of list, item not found */
		if (prev->next == NULL)
		{
			return 1;
		}
		next = prev->next->next;
		/* if result was the last node, need to adjust the tail pointer */
		if (next == NULL)
		{
			list->tail = prev;
		}
		node_destroy(prev->next, list->data_destroy);
		prev->next = next;
	}	
	
	list->count--;
	return 0;
}

/**
 * @brief Remove the item at the given index from the list. Note: This DOES
 *        NOT free the data pointer given, as it has to return it.
 * @param list a pointer to memory in which the linkedlist exists
 * @param index the index to remove from
 * @return the data pointer stored there, or NULL if the list pointer is null
 *         or the index is out of bounds.
 */
void *ll_remove_index(linkedlist_t *list, unsigned long index)
{
	node_t *previousnode;
	node_t *nextnode;
	void *data;

	if (!list || index >= list->count)
	{
		return NULL;
	}
	
	if (index == 0)
	{
		nextnode = list->head->next;
		data = list->head->data;
		node_destroy(list->head, do_nothing); /* free the node without freeing the data pointer */
		list->head = nextnode;
	}
	else
	{
		previousnode = list->head;
		while (index > 1)
		{
			previousnode = previousnode->next;
			index--;
		}
		nextnode = previousnode->next->next;
		data = previousnode->next->data;
		node_destroy(previousnode->next, do_nothing); /* again, don't want to free the return val */
		previousnode->next = nextnode;

	}
	list->count--;
	return data;
}

/**
 * @brief Get an item (by index) from the list. Runs in O(index) time.
 * @param list a pointer to memory in which the linkedlist exists
 * @param index the index to get from
 * @return the data pointer on success, NULL if the index is out of bounds or
 *         the list pointer is null or if the list is empty.
 */
void *ll_get(linkedlist_t *list, unsigned long index)
{	
	unsigned long i;
	node_t *currentnode;

	if (!list || index >= list->count)
	{
		return NULL;
	}
	
	/* Move currentnode to the index we want. We can't hit the end before
	 * then because of our previous check. */
	currentnode = list->head;
	for (i = 0; i < index; i++)
	{
		currentnode = currentnode->next;
	}
	
	return currentnode->data;
}

/**
 * @brief search the list for the specified data
 * @param list a pointer to memory in which the linkedlist exists
 * @param data the data to look for
 * @return the index of the first occurrance of the data in the list, or -1 if
 *         not present. Basically the inverse of ll_get. Runs in O(n) time,
 *         depending where the data is.
 */
unsigned long ll_search(linkedlist_t *list, void *data)
{
	node_t *currentnode;
	unsigned long index;
	
	if (!list || list->count == 0)
	{
		return -1;
	}
	
	index = 0;
	currentnode = list->head;
	
	while (currentnode)
	{
		if ( 0 == (*(list->data_compare))(currentnode->data, data) )
		{
			return index;
		}
		index++;
		currentnode = currentnode->next;
	}
	/* if we get to here then item not found */
	return -1;
}

/**
 * @brief Perform a function (given a function pointer to it) on every node in
 *        the list.
 * @param list a pointer to memory in which the linkedlist exists
 * @param data_function the function to call on each data element
 * @return 0 on success, or -1 if either pointer is null.
 */
int ll_operate(linkedlist_t *list, data_operate_fn data_function)
{
	node_t *currentnode;
	
	if (!list || !data_function)
	{
		return -1;
	}
	
	currentnode = list->head;
	while (currentnode)
	{
		data_function(currentnode->data);
		currentnode = currentnode->next;
	}
	
	return 0;
}

/**
 * Initializes a node.
 */
static node_t *node_init()
{
	node_t *n = malloc(sizeof(node_t));
	#ifdef LL_CHECK_MALLOC_FAIL
	if (n == NULL)
	{
		fprintf(stderr, "LL_NODE_INIT: Malloc failed - %s\n", strerror(errno));
		return NULL;
	}
	#endif
	n->next = NULL;
	return n;
}

/**
 * Destroy a node.
 */
static int node_destroy(node_t *n, data_destroy_fn data_destroy)
{
	if (!n)
	{
		return -1;
	}
	
	data_destroy(n->data);
	free(n);

	return 0;
}

/**
 * Does nothing. Used by remove_index, where you have to free the node but not
 * the data pointer in it.
 */
static void do_nothing(void *data __attribute__((unused)))
{
	return;
}
