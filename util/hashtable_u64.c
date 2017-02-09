#include <stdlib.h>
#ifdef HT_CHECK_MALLOC_FAIL
#include <stdio.h>
#endif
#include "hashtable_u64.h"

static unsigned long u64_hash(uint64_t data, unsigned long size)
{
	return data % size;
}

/**
 * Initialize a hashtable. Returns a pointer to the new ht on success, NULL if
 * the fnpointer is null. Prints a message to stderr and returns NULL if the
 * malloc fails.
 */
hashtable_u64_t *ht_u64_init(unsigned long size)
{
	hashtable_u64_t *ht;
	
	ht = malloc(sizeof(hashtable_u64_t));
	#ifdef HT_CHECK_MALLOC_FAIL
	if (ht == NULL)
	{
		perror("HT_U64_INIT: Malloc failed");
		return NULL;
	}
	#endif
	
	ht_u64_setup(ht, size);
	return ht;
}

/**
 * Set up a hashtable in a piece of already alloc'd memory for a ht_u64_t
 * Note: this does allocate more memory for the array
 */
int ht_u64_setup(hashtable_u64_t *ht, unsigned long size)
{
	unsigned long index;
	if (ht == NULL)
	{
		return 1;
	}

	ht->array = malloc(size * sizeof(linkedlist_u64_t));
	#ifdef HT_CHECK_MALLOC_FAIL
	if (ht->array == NULL)
	{
		perror("HT_U64_INIT: Malloc failed");
		return NULL;
	}
	#endif
	
	for (index = 0; index < size; index++)
	{
		ll_u64_setup(&(ht->array[index]));
	}

	ht->size = size;
	ht->count = 0;
	return 0;
}

/**
 * Destroy a hashtable. Returns 0 on success, -1 if the pointer is null.
 */
int ht_u64_destroy(hashtable_u64_t *ht)
{
	if (ht == NULL)
	{
		return -1;
	}
	
	ht_u64_teardown(ht);
	free(ht);
	
	return 0;
}

int ht_u64_teardown(hashtable_u64_t *ht)
{
	if (ht == NULL)
	{
		return 1;
	}
	ht_u64_clear(ht);
	free(ht->array);
	return 0;
}

int ht_u64_clear(hashtable_u64_t *ht)
{
	unsigned long index;
	if (ht == NULL)
	{
		return 1;
	}

	for (index = 0; index < ht->size; index++)
	{
		ll_u64_clear(&(ht->array[index]));
	}
	ht->count = 0;
	return 0;
}

/**
 * Add an element to the hashtable. Return 0 on success, -1 if the pointer is
 * null, 1 if already present.
 */
int ht_u64_add(hashtable_u64_t *ht, uint64_t data)
{
	unsigned long hash;

	if (ht == NULL)
	{
		return -1;
	}
	hash = u64_hash(data, ht->size);
	
	/* if item already present */
	if (ll_u64_search(&(ht->array[hash]), data) + 1)
	{
		return 1;
	}
	else /* perform the add */
	{
		ht->count++;
		return ll_u64_add(&(ht->array[hash]), data);
	}
}

/**
 * See if an element is contained in the hashtable. Returns 1 if the item is
 * present, 0 if not, -1 if the pointer is null.
 */
int ht_u64_contains(hashtable_u64_t *ht, uint64_t data)
{
	if (ht == NULL)
	{
		return -1;
	}
	return !!(ll_u64_search(&(ht->array[u64_hash(data, ht->size)]), data) + 1);
}

/**
 * Removes an item from the table. Returns 0 on success, 1 if the item is not
 * present in the table, -1 if the pointer is null.
 */
int ht_u64_remove(hashtable_u64_t *ht, uint64_t data)
{
	int retval;
	if (ht == NULL)
	{
		return -1;
	}
	
	retval = ll_u64_remove(&(ht->array[u64_hash(data, ht->size)]), data);
	if (retval == 0)
	{
		/* the item was in fact present */
		ht->count--;
	}
	return retval;
}

/**
 * Perform a function on every element in the table. Returns 0 on success, -1
 * if either pointer is null.
 */
int ht_u64_operate(hashtable_u64_t *ht, u64_operate_fn data_function)
{
	unsigned long index;
	
	if (ht == NULL || data_function == NULL)
	{
		return -1;
	}

	for (index = 0; index < ht->size; index++)
	{
		ll_u64_operate(&(ht->array[index]), data_function);
	}

	return 0;
}
