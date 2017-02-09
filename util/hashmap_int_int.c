#include <stdlib.h>
#include "hashmap_int_int.h"

typedef struct hashmap_int_int_entry_t {
	int key;
	int value;
} hashmap_int_int_entry_t;

static hashmap_int_int_entry_t *hm_int_int_entry_init(int key, int value)
{
	hashmap_int_int_entry_t *entry = malloc(sizeof(hashmap_int_int_entry_t));
	entry->key = key;
	entry->value = value;
	return entry;
}

static int hm_int_int_entry_compare(void *a, void *b)
{
	hashmap_int_int_entry_t *foo = (hashmap_int_int_entry_t *)a;
	hashmap_int_int_entry_t *bar = (hashmap_int_int_entry_t *)b;
	return (bar->key) - (foo->key);
}

static void hm_int_int_entry_destroy(void *entry)
{
	free(entry);
}

static unsigned long int_hash(int data, unsigned long size)
{
	return data % size; 
}


/**
 * @brief Initialize a hashmap.
 * @param size how many buckets the hashmap will contain
 * @return a pointer to the new hashmap
 */
hashmap_int_int_t *hm_int_int_init(unsigned long size)
{
	hashmap_int_int_t *hm = malloc(sizeof(hashmap_int_int_t));
	hm_int_int_setup(hm, size);
	return hm;
}

/**
 * @brief Set up a hashmap in an already alloc'd piece of memory
 * @param hm pointer to the memory in which the hashmap will be set up
 * @param size how many buckets the hashmap will contain
 * @return 0 on success, 1 if the pointer is null
 */
int hm_int_int_setup(hashmap_int_int_t *hm, unsigned long size)
{
	unsigned long index;
	if (hm == NULL)
	{
		return 1;
	}
	hm->array = malloc(size * sizeof(linkedlist_t));
	for (index = 0; index < size; index++)
	{
		ll_setup(&(hm->array[index]),
		         hm_int_int_entry_compare, hm_int_int_entry_destroy);
	}

	hm->size = size;
	hm->count = 0;
	return 0;
}

/**
 * @brief Destroy a hashmap
 * @param hm pointer to the memory in which the hashmap exists
 * @return 0 on success, -1 if the pointer is null.
 */
int hm_int_int_destroy(hashmap_int_int_t *hm)
{
	if (hm == NULL)
	{
		return -1;
	}
	hm_int_int_teardown(hm);
	free(hm);
	
	return 0;
}

/**
 * @brief Release all memory used by the hashmap, but not the struct itself
 * @param hm pointer to the memory in which the hashmap exists
 * @return 0 on success, -1 if the pointer is null.
 */
int hm_int_int_teardown(hashmap_int_int_t *hm)
{
	if (hm == NULL)
	{
		return -1;
	}
	hm_int_int_clear(hm);
	free(hm->array);
	return 0;
}

/**
 * @brief Remove all entries from the hashmap
 * @param hm pointer to the memory in which the hashmap exists
 * @return 0 on success, -1 if the pointer is null.
 */
int hm_int_int_clear(hashmap_int_int_t *hm)
{
	unsigned long index;

	if (hm == NULL)
	{
		return -1;
	}

	for (index = 0; index < hm->size; index++)
	{
		ll_clear(&(hm->array[index]));
	}
	hm->count = 0;
	return 0;
}

/**
 * @brief Add an pairing to the hashmap
 * @param hm pointer to the memory in which the hashmap exists
 * @param key the key which will map to the value
 * @param value the value which is mapped to by the key
 * @return 0 on success, -1 if the pointer is null, 1 if an old mapping was
 *         replaced.
 */
int hm_int_int_add(hashmap_int_int_t *hm, int key, int value)
{
	hashmap_int_int_entry_t *entry;
	int retval = 0;
	if (hm == NULL)
	{
		return -1;
	}
	
	if (hm_int_int_containskey(hm, key))
	{
		hm_int_int_removekey(hm, key);
		retval = 1;
	}
	hm->count++; /* works in either case - hm_remove decrements count */
	entry = hm_int_int_entry_init(key, value);
	ll_add(&(hm->array[int_hash(key, hm->size)]), entry);
	return retval;
}

/**
 * @brief See if an element is contained in the hashtable.
 * @param hm pointer to the memory in which the hashmap exists
 * @param key the key to look for
 * @return 1 if the item is present, 0 if not, -1 if the pointer is null.
 */
int hm_int_int_containskey(hashmap_int_int_t *hm, int key)
{
	hashmap_int_int_entry_t dummy; /* on the stack not heap for speed */
	if (hm == NULL)
	{
		return -1;
	}
	
	/* Comparing only cares about the key, not the value */
	dummy.key = key;
	/* ll_search returns the index, -1 if not present */
	return (ll_search(&(hm->array[int_hash(key, hm->size)]), &dummy) != (unsigned long)-1);
}

/**
 * @brief Given the key, find the value that it maps to.
 * @param hm pointer to the memory in which the hashmap exists
 * @param key the key to search for
 * @return Whatever value the key maps to.
 *         If not present, returns HASHMAP_DATA_NOT_PRESENT. This return value
 *         is not to be used as a status indicator unless you really know what
 *         you're doing, as a value could legitimately map to that value. Use
 *         containskey() if you're not sure that a key is present. (Also
 *         returns that constant on a null hashmap pointer)
 */
int hm_int_int_get(hashmap_int_int_t *hm, int key)
{
	node_t *currentnode;
	linkedlist_t *bucket;

	if (hm == NULL)
	{
		return HASHMAP_DATA_NOT_PRESENT;
	}
	
	/* the bucket this key would be in */
	bucket = &(hm->array[int_hash(key, hm->size)]);
	/* search for our item */
	currentnode = bucket->head;
	while (currentnode)
	{
		if (key == ((hashmap_int_int_entry_t *)(currentnode->data))->key)
		{
			return ((hashmap_int_int_entry_t *)(currentnode->data))->value;
		}
		currentnode = currentnode->next;
	}
	/* we hit the end of the list, not found */
	return HASHMAP_DATA_NOT_PRESENT;
}

/**
 * @brief Removes an mapping from the table.
 * @param hm pointer to the memory in which the hashmap exists
 * @param key the key whose mapping will be removed
 * @return 0 on success, 1 if the item is not present in the table, -1 if the
 *         pointer is null.
 */
int hm_int_int_removekey(hashmap_int_int_t *hm, int key)
{
	/* same principle as contains - the ll needs to search for the pair */
	hashmap_int_int_entry_t dummy;
	int retval;
	if (hm == NULL)
	{
		return -1;
	}

	dummy.key = key;
	retval = ll_remove(&(hm->array[int_hash(key, hm->size)]), &dummy);
	if (retval == 0) /* the item was present */
	{
		hm->count--;
	}
	return retval;
}

/* XXX: This is really ugly, but the data_operate_fn passed to the linkedlist
 * needs some way of knowing what function to perform, so we keep the fnptr
 * in a global... yeah. At least it's static. You know what, if you don't
 * like it, don't use hm_int_int_operate */
static int_int_operate_fn pair_operate;
/**
 * For passing to the linkedlist. This function should never be passed to
 * ll_operate unless by the publicly accessible hm_int_int_operate, or else
 * the data pointer may be invalid.
 */
static void hm_int_int_entry_operate(void *ptr)
{
	hashmap_int_int_entry_t *entry = (hashmap_int_int_entry_t *)ptr;
	pair_operate(entry->key, entry->value);
}
/**
 * @brief Given a function pointer to a int_int_operate_fn, perform it on every
 *        (key,value) pairing in the map.
 */
int hm_int_int_operate(hashmap_int_int_t *hm, int_int_operate_fn pair_fn)
{
	unsigned long i;

	if (hm == NULL || pair_fn == NULL)
	{
		return -1;
	}

	pair_operate = pair_fn; /* augh */
	/* now do the standard 'iterate through every slot in the array' */
	for (i = 0; i < hm->size; i++)
	{
		ll_operate(&(hm->array[i]), hm_int_int_entry_operate);
	}

	return 0;
}
