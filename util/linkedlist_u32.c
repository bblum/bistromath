#include <stdlib.h>
#ifdef LL_CHECK_MALLOC_FAIL
#include <stdio.h>
#endif
#include "linkedlist_u32.h"

static node_u32_t *node_u32_init(uint32_t);

#define LLU32_RETURN_FAILURE -1

/**
 * Initialize a linkedlist for uint32_t's. Only fails if malloc fails.
 */
linkedlist_u32_t *ll_u32_init()
{
	linkedlist_u32_t *ll;

	ll = malloc(sizeof(linkedlist_u32_t));
	#ifdef LL_CHECK_MALLOC_FAIL
	if (ll == NULL)
	{
		perror("LL_U32_INIT: Malloc failed");
		return NULL;
	}
	#endif

	ll_u32_setup(ll);

	return ll;
}

/**
 * Set up a linkedlist in already alloc'd memory. returns -1 if the pointer
 * is null, or 0 on success.
 */
int ll_u32_setup(linkedlist_u32_t *list)
{
	if (list == NULL)
	{
		return 0;
	}
	list->head = NULL;
	list->count = 0;
	return 0;
}

/**
 * Destroy a linkedlist.
 */
int ll_u32_destroy(linkedlist_u32_t *list)
{
	if (!list)
	{
		return -1;
	}
	
	ll_u32_clear(list);
	free(list);

	return 0;
}

int ll_u32_clear(linkedlist_u32_t *list)
{
	node_u32_t *nextnode;
	if (!list)
	{
		return -1;
	}
	
	while (list->head)
	{
		nextnode = list->head->next;
		free(list->head);
		list->head = nextnode;
	}
	list->count = 0;
	list->tail = 0;
	return 0;
}

/**
 * Add an element to the end of the list. Returns 0 on success, -1 if the list
 * pointer is null.
 * Runs in O(1) time. Does not check for duplicate entries.
 */
int ll_u32_add(linkedlist_u32_t *list, uint32_t data)
{
	node_u32_t *newnode;

	if (!list)
	{
		return -1;
	}
	
	newnode = node_u32_init(data);

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
 * Insert an element into the list at the specified index. If the index is
 * beyond the end of the list, this will simply append the item to the end of
 * the list. Runs in O(index) time, unless index is equal to (or greater
 * than) the number of elements in the list, then O(1)
 */
int ll_u32_add_index(linkedlist_u32_t *list, unsigned long index, uint32_t data)
{
	node_u32_t *newnode;

	if (!list)
	{
		return -1;
	}
	
	newnode = node_u32_init(data);
	
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
		node_u32_t *prev = list->head;
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
 * Remove an item from the list. Returns 0 on success, 1 if the item is not
 * present in the list, -1 if the list pointer is null.
 * Runs in O(n) time.
 */
int ll_u32_remove(linkedlist_u32_t *list, uint32_t data)
{
	node_u32_t *prev;
	node_u32_t *next;

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
	
	/* Edge case: removing the first item in the list - we won't be able to move
	 * previousnode to the one behind it, because there is no one behind it. */
	if (list->head->data == data)
	{
		next = list->head->next;
		free(list->head);
		list->head = next;
	}
	/* Typical case */
	else
	{
		prev = list->head;
		/* while prevnode's next isn't what we're looking for - also,
		 * watch out for hitting the end of the list */
		while (prev->next && prev->next->data != data)
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
		free(prev->next);
		prev->next = next;
	}	
	
	list->count--;
	return 0;
}

/**
 * Remove the item at the given index from the list. Returns the data
 * stored there, or -1 if the index is out of bounds. Note: This return value
 * is not reliable, due to it could be a valid return value if the function
 * succeeded. The user, not this function, must check for invalid arguments;
 * this function is not appropriate for doing so.
 */
uint32_t ll_u32_remove_index(linkedlist_u32_t *list, unsigned long index)
{
	node_u32_t *previousnode;
	node_u32_t *nextnode;
	uint32_t data;

	if (!list || index >= list->count)
	{
		return LLU32_RETURN_FAILURE;
	}
	
	if (index == 0)
	{
		nextnode = list->head->next;
		data = list->head->data;
		free(list->head);
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
		free(previousnode->next);
		previousnode->next = nextnode;

	}
	list->count--;
	return data;
}

/**
 * Get an item (by index) from the list. Returns a pointer to the data on
 * success, NULL if the index is out of bounds or the list pointer is null
 * or if the list is empty. Runs in O(index) time. The return value is not
 * a reliable status indicator - see the comment for remove_index
 */
uint32_t ll_u32_get(linkedlist_u32_t *list, unsigned long index)
{	
	unsigned long i;
	node_u32_t *currentnode;

	if (!list || index >= list->count)
	{
		return LLU32_RETURN_FAILURE;
	}
	
	/* Move currentnode to the index we want. We're guaranteed not to hit
	 * the end before then because of the previous check */
	currentnode = list->head;
	for (i = 0; i < index; i++)
	{
		currentnode = currentnode->next;
	}
	
	return currentnode->data;
}

/**
 * Search method: Returns the index of the first occurrance of the data in the
 * list, or -1 if not present. Basically the inverse of ll_u32_get. Runs in O(n) 
 * time, depending where the data is.
 */
unsigned long ll_u32_search(linkedlist_u32_t *list, uint32_t data)
{
	node_u32_t *currentnode;
	unsigned long index;
	
	if (!list || list->count == 0)
	{
		return -1;
	}
	
	index = 0;
	currentnode = list->head;
	
	while (currentnode)
	{
		if (currentnode->data == data)
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
 * Perform a function (given a function pointer to it) on every node in the
 * list. Returns 0 on success, or -1 if either pointer is null.
 */
int ll_u32_operate(linkedlist_u32_t *list, u32_operate_fn data_function)
{
	node_u32_t *currentnode;
	
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
 * Appends a list to the end of this one. The count of this is updated to
 * reflect the new number of elements. Any memory used by the second list
 * argument no longer used is freed (currently, the struct itself).
 */
void ll_u32_append(linkedlist_u32_t *list, linkedlist_u32_t *second)
{
	if (list == NULL || second == NULL)
	{
		return;
	}
	
	if (second->count == 0)
	{
		free(second);
		return;
	}
	if (list->count == 0)
	{
		list->head = second->head;
		list->tail = second->tail;
		list->count = second->count;
		free(second);
		return;
	}
	/* here we know that both lists have stuff in them */
	list->tail->next = second->head;
	list->tail = second->tail;
	list->count += second->count;
	free(second);
	return;
}

/**
 * Initializes a node.
 */
static node_u32_t *node_u32_init(uint32_t data)
{
	node_u32_t *n = malloc(sizeof(node_u32_t));
	#ifdef LL_CHECK_MALLOC_FAIL
	if (n == NULL)
	{
		perror("LL_NODE_INIT: Malloc failed");
		return NULL;
	}
	#endif
	n->next = NULL;
	n->data = data;
	return n;
}
