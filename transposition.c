/****************************************************************************
 * transposition.c - avoid re-searching already searched positions
 * copyright (C) 2008 Ben Blum
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 ****************************************************************************/
#include <stdlib.h>
#include "transposition.h"
#include "assert.h"

typedef struct trans_entry_t {
	uint64_t key;
	trans_data_t value;
} trans_entry_t;

/**
 * Statically initting the array is good because it avoids committing memory
 * we don't use, but depends, for efficiency not correctness, on the kernel
 * harbl-filling our memory with zeros. When we go to kick out an entry, if
 * we haven't put one there yet, we'll see that the depth is 0 and the flag
 * is ALPHA - the two worst possible values, so this type will always be
 * replaced.
 */
#ifndef TRANS_NUM_BUCKETS
#define TRANS_NUM_BUCKETS 67108864
#endif
static trans_entry_t array[TRANS_NUM_BUCKETS];

/**
 * Convert separate data values -> condensed struct
 */
static trans_data_t trans_data(move_t move, uint8_t reps, int16_t value,
			       uint8_t gamedepth, uint8_t searchdepth,
			       uint8_t flag)
{
	trans_data_t foo;
	assert(searchdepth < 64);
	foo.move = move | (reps << MOV_INDEX_UNUSED);
	foo.value = value;
	foo.gamedepth = gamedepth;
	foo.flags = ((searchdepth & 0x3f) << TRANS_INDEX_SEARCHDEPTH) |
	           ((flag & 0x3) << TRANS_INDEX_FLAG);
	return foo;
}

/**
 * Add transposition information to the table. Note, it will only kick the old
 * entry out if:
 * 	1) The flag is higher (exact kicks out beta, beta kicks out alpha)
 * 	2) The flag is equal and the depth is higher
 * 	3) The old node was from ancient history
 * Note the different depth arguments:
 * 	rootdepth   - how far down the game tree the real board state is.
 * 	              used in comparison for evicting the old node.
 * 	gamedepth   - how far down this current node is in the game tree. we
 * 	              store this for future nodes to use to evict us.
 * 	searchdepth - how much deeper still we've searched this node. this
 * 	              tells how trustworthy the alpha value is.
 * At present we don't discriminate based on repetition depth.
 */
void trans_add(zobrist_t key, move_t move, uint8_t reps, int16_t value,
               uint8_t rootdepth, uint8_t gamedepth,
               uint8_t searchdepth, uint8_t flag)
{
	unsigned long bucket;
	trans_data_t old;
	
	/* find the bucket */
	bucket = key % TRANS_NUM_BUCKETS;
	
	/* find what used to be there */
	old = array[bucket].value;

	/* see if we should evict */
	if ((rootdepth >= TRANS_GAMEDEPTH(old)) || /* old node too old */
	    (flag > TRANS_FLAG(old)) || /* more accurate data */
	    ((flag == TRANS_FLAG(old)) && /* same type of data but... */
	     (searchdepth > TRANS_SEARCHDEPTH(old)))) /* ...deeper search */
	{
		/* so we set the stuff to our new node */
		array[bucket].key = key;
		array[bucket].value = trans_data(move, reps, value, gamedepth,
		                                 searchdepth, flag);
	}
	return;
}

/**
 * Fetch data from the table. If there is no entry for this key the flag in
 * the return value will be set to -1.
 */
static trans_data_t foo = { (uint8_t)(-1), (uint8_t)(-1),
                            (int16_t)(-1), (uint32_t)(-1) };
trans_data_t trans_get(zobrist_t key)
{
	unsigned long bucket = key % TRANS_NUM_BUCKETS;
	if (array[bucket].key == key)
	{
		return array[bucket].value;
	}
	return foo;
}

/**
 * Check if the transposition data is valid, according to whatever spec may be
 * in use for saying "invalid entry". Used for checking the returnvalue of get
 */
int trans_data_valid(trans_data_t foo)
{
	return (foo.flags != (uint8_t)(-1));
}
