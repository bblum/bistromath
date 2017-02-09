#include <stdlib.h>
#ifdef HT_CHECK_MALLOC_FAIL
#include <stdio.h>
#endif
#include "hashtable.h"

/**
 * @brief Initialize a hashtable.
 * @param size how many buckets the hashtable will have
 * @param data_compare a function pointer that takes two void * pointers and
 *        compares them, returning <0, 0, or >0 accordingly.
 * @param data_destroy a function pointer that, given a void *, destroys the
 *        element
 * @param data_hash a function pointer that computes a hash of the data
 * @return a pointer to the new ht on success, NULL if the fnpointer is null.
 */
hashtable_t *ht_init(unsigned long size, data_compare_fn data_compare,
                     data_destroy_fn data_destroy, data_hash_fn data_hash)
{
	hashtable_t *ht;

	if (data_compare == NULL || data_destroy == NULL || data_hash == NULL)
	{
		return NULL;
	}
	
	ht = malloc(sizeof(hashtable_t));
	#ifdef HT_CHECK_MALLOC_FAIL
	if (ht == NULL)
	{
		perror("HT_INIT: Malloc failed");
		return NULL;
	}
	#endif
	
	ht_setup(ht, size, data_compare, data_destroy, data_hash);
	return ht;
}

/**
 * @brief Setup a hashtable in already allocated space
 * @param ht a pointer to the memory in which the hashtable will be set up
 * @param size how many buckets the hashtable will have
 * @param data_compare see ht_init
 * @param data_destroy see ht_init
 * @param data_hash see ht_init
 * @return 0 on success, 1 if any pointer is null
 */
int ht_setup(hashtable_t *ht, unsigned long size, data_compare_fn data_compare,
             data_destroy_fn data_destroy, data_hash_fn data_hash)
{
	unsigned long index;

	if (ht == NULL || data_compare == NULL ||
	    data_destroy == NULL || data_hash == NULL)
	{
		return 1;
	}

	ht->array = malloc(size * sizeof(linkedlist_t));
	#ifdef HT_CHECK_MALLOC_FAIL
	if (ht->array == NULL)
	{
		perror("HT_INIT: Malloc failed");
		return -1;
	}
	#endif

	for (index = 0; index < size; index++)
	{
		ll_setup(&(ht->array[index]), data_compare, data_destroy);
	}
	
	ht->data_hash = data_hash;
	ht->size = size;
	ht->count = 0;

	return 0;
}

/**
 * @brief Destroy a hashtable, freeing all memory associated with it
 * @param ht a pointer to the memory in which the hashtable exists
 * @return 0 on success, -1 if the pointer is null.
 */
int ht_destroy(hashtable_t *ht)
{
	if (ht == NULL)
	{
		return -1;
	}
	ht_teardown(ht);
	free(ht);
	
	return 0;
}

/**
 * @brief Return a hashtable initialized with ht_setup not ht_init to original
 *        state
 * @param ht a pointer to the memory in which the hashtable exists
 * @return 0 on success, 1 if the pointer is null
 */
int ht_teardown(hashtable_t *ht)
{
	if (ht == NULL)
	{
		return 1;
	}
	ht_clear(ht);
	free(ht->array);
	return 0;
}

/**
 * @brief Remove all the items from a hashtable
 * @param ht a pointer to the memory in which the hashtable exists
 * @return 0 on success, 1 if the pointer is null
 */
int ht_clear(hashtable_t *ht)
{
	unsigned long index;
	if (ht == NULL)
	{
		return 1;
	}

	for (index = 0; index < ht->size; index++)
	{
		ll_clear(&(ht->array[index]));
	}
	ht->count = 0;
	return 0;
}

/**
 * @brief Add an element to the hashtable.
 * @param ht a pointer to the memory in which the hashtable exists
 * @param data the data to add
 * @return 0 on success, -1 if the pointer is null.
 */
int ht_add(hashtable_t *ht, void *data)
{
	unsigned long hash;
	
	if (ht == NULL)
	{
		return -1;
	}
	hash = ht->data_hash(data, ht->size);
	/* element already present */
	if (ll_search(&(ht->array[hash]), data) + 1)
	{
		return 1;
	}
	else /* perform the add */
	{
		ht->count++;
		return ll_add(&(ht->array[hash]), data);
	}
}

/**
 * @brief See if an element is contained in the hashtable.
 * @param ht a pointer to the memory in which the hashtable exists
 * @param data the data to look for
 * @return 1 if the item is present, 0 if not, -1 if the pointer is null.
 */
int ht_contains(hashtable_t *ht, void *data)
{
	if (ht == NULL)
	{
		return -1;
	}
	return (ll_search(&(ht->array[ht->data_hash(data, ht->size)]), data) != (unsigned long)-1);
}

/**
 * @brief Removes an item from the table.
 * @param ht a pointer to the memory in which the hashtable exists
 * @param data the data to remove
 * @return 0 on success, 1 if the item is not present in the table, -1 if the
 *         pointer is null.
 */
int ht_remove(hashtable_t *ht, void *data)
{
	int retval;
	if (ht == NULL)
	{
		return -1;
	}
	retval = ll_remove(&(ht->array[ht->data_hash(data, ht->size)]), data);
	if (retval == 0)
	{
		/* the remove was successful, adjust the counter */
		ht->count--;
	}
	return retval;
}

/**
 * @brief Perform a function on every element in the table.
 * @param ht a pointer to the memory in which the hashtable exists
 * @param data_function the function to be called on each element
 * @return 0 on success, -1 if either pointer is null.
 */
int ht_operate(hashtable_t *ht, data_operate_fn data_function)
{
	unsigned long index;
	
	if (ht == NULL || data_function == NULL)
	{
		return -1;
	}

	for (index = 0; index < ht->size; index++)
	{
		ll_operate(&(ht->array[index]), data_function);
	}

	return 0;

}
