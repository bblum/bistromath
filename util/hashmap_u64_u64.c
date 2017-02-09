#include <stdlib.h>
#include "hashmap_u64_u64.h"

typedef struct hashmap_u64_u64_entry_t {
	uint64_t key;
	uint64_t value;
} hashmap_u64_u64_entry_t;

static hashmap_u64_u64_entry_t *hm_u64_u64_entry_init(uint64_t key, uint64_t value)
{
	hashmap_u64_u64_entry_t *entry = malloc(sizeof(hashmap_u64_u64_entry_t));
	entry->key = key;
	entry->value = value;
	return entry;
}

static int hm_u64_u64_entry_compare(void *a, void *b)
{
	hashmap_u64_u64_entry_t *foo = (hashmap_u64_u64_entry_t *)a;
	hashmap_u64_u64_entry_t *bar = (hashmap_u64_u64_entry_t *)b;
	return (bar->key) - (foo->key);
}

static void hm_u64_u64_entry_destroy(void *entry)
{
	free(entry);
}

static unsigned long u64_hash(uint64_t data, unsigned long size)
{
	return data % size; 
}


/**
 * Initialize a hashmap. Returns 0 on success, -1 if the fnpointer is null.
 */
hashmap_u64_u64_t *hm_u64_u64_init(unsigned long size)
{
	hashmap_u64_u64_t *hm = malloc(sizeof(hashmap_u64_u64_t));
	hm_u64_u64_setup(hm, size);
	return hm;
}

/**
 * Set up a hashmap in an already alloc'd piece of memory
 */
int hm_u64_u64_setup(hashmap_u64_u64_t *hm, unsigned long size)
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
		         hm_u64_u64_entry_compare, hm_u64_u64_entry_destroy);
	}

	hm->size = size;
	hm->count = 0;
	return 0;
}

/**
 * Destroy a hashmap. Returns 0 on success, -1 if the pointer is null.
 */
int hm_u64_u64_destroy(hashmap_u64_u64_t *hm)
{
	if (hm == NULL)
	{
		return -1;
	}
	hm_u64_u64_teardown(hm);
	free(hm);
	
	return 0;
}

int hm_u64_u64_teardown(hashmap_u64_u64_t *hm)
{
	if (hm == NULL)
	{
		return -1;
	}
	hm_u64_u64_clear(hm);
	free(hm->array);
	return 0;
}

int hm_u64_u64_clear(hashmap_u64_u64_t *hm)
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
 * Add an pairing to the hashmap. Return 0 on success, -1 if the pointer is
 * null, 1 if an old mapping was replaced.
 */
int hm_u64_u64_add(hashmap_u64_u64_t *hm, uint64_t key, uint64_t value)
{
	hashmap_u64_u64_entry_t *entry;
	int retval = 0;
	if (hm == NULL)
	{
		return -1;
	}
	
	if (hm_u64_u64_containskey(hm, key))
	{
		hm_u64_u64_removekey(hm, key);
		retval = 1;
	}
	hm->count++; /* works in either case - hm_remove decrements count */
	entry = hm_u64_u64_entry_init(key, value);
	ll_add(&(hm->array[u64_hash(key, hm->size)]), entry);
	return retval;
}

/**
 * See if an element is contained in the hashtable. Returns 1 if the item is
 * present, 0 if not, -1 if the pointer is null.
 */
int hm_u64_u64_containskey(hashmap_u64_u64_t *hm, uint64_t key)
{
	hashmap_u64_u64_entry_t dummy; /* on the stack not heap for speed */
	if (hm == NULL)
	{
		return -1;
	}
	
	/* Comparing only cares about the key, not the value */
	dummy.key = key;
	/* ll_search returns the index, -1 if not present */
	return (ll_search(&(hm->array[u64_hash(key, hm->size)]), &dummy) != (unsigned long)-1);
}

/**
 * Given the key, return the value that it maps to. If not present. returns
 * HASHMAP_DATA_NOT_PRESENT. This return value is not to be used as a status
 * indicator unless you really know what you're doing, as a value could
 * legitimately map to that value. Use containskey() if you're not sure that a
 * key is present. (Also returns that constant on a null hashmap pointer)
 */
uint64_t hm_u64_u64_get(hashmap_u64_u64_t *hm, uint64_t key)
{
	node_t *currentnode;
	linkedlist_t *bucket;
	
	if (hm == NULL)
	{
		return HASHMAP_DATA_NOT_PRESENT;
	}

	/* the bucket this key would be in */
	bucket = &(hm->array[u64_hash(key, hm->size)]);
	/* search for our item */
	currentnode = bucket->head;
	while(currentnode)
	{
		if (key == ((hashmap_u64_u64_entry_t *)(currentnode->data))->key)
		{
			return ((hashmap_u64_u64_entry_t *)(currentnode->data))->value;
		}
		currentnode = currentnode->next;
	}
	/* we hit the end of the list, not found */
	return HASHMAP_DATA_NOT_PRESENT;
}

/**
 * Removes an item from the table. Returns 0 on success, 1 if the item is not
 * present in the table, -1 if the pointer is null.
 */
int hm_u64_u64_removekey(hashmap_u64_u64_t *hm, uint64_t key)
{
	/* same principle as contains - the ll needs to search for the pair */
	hashmap_u64_u64_entry_t dummy;
	int retval;
	if (hm == NULL)
	{
		return -1;
	}

	dummy.key = key;
	retval = ll_remove(&(hm->array[u64_hash(key, hm->size)]), &dummy);
	if (retval == 0) /* the item was present */
	{
		hm->count--;
	}
	return retval;
}

/* XXX: This is really ugly, but the data_operate_fn passed to the linkedlist
 * needs some way of knowing what function to perform, so we keep the fnptr
 * in a global... yeah. At least it's static. You know what, if you don't
 * like it, don't use hm_u64_u64_operate */
static u64_u64_operate_fn pair_operate;
/**
 * For passing to the linkedlist. This function should never be passed to
 * ll_operate unless by the publicly accessible hm_u64_u64_operate, or else
 * the data pointer may be invalid.
 */
static void hm_u64_u64_entry_operate(void *ptr)
{
	hashmap_u64_u64_entry_t *entry = (hashmap_u64_u64_entry_t *)ptr;
	pair_operate(entry->key, entry->value);
}
/**
 * Given a function pointer to a u64_u64_operate_fn, perform it on every
 * (key,value) pairing in the map.
 */
int hm_u64_u64_operate(hashmap_u64_u64_t *hm, u64_u64_operate_fn pair_fn)
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
		ll_operate(&(hm->array[i]), hm_u64_u64_entry_operate);
	}

	return 0;
}
