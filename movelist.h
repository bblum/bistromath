/****************************************************************************
 * movelist.h - data structure for move sorting in move generation/iteration
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
 * ****************************************************************************/
#ifndef MOVELIST_H
#define MOVELIST_H

#include <stdint.h>
#include <string.h>

#define MOVELIST_NUM_BUCKETS 64
#define MOVELIST_SUBLIST_LENGTH 128

/****************************************************************************
 * Our move ordering scheme will look something like this:
 *
 * 00-08: Moves that lose material
 * 	00: -9 (Q hang)
 * 	01: -8 (Qx protected P)
 * 	02: -7 (None)
 * 	03: -6 (Qx protected B/N)
 * 	04: -5 (R hang)
 * 	05: -4 (Qx protected R)
 * 	06: -3 (B/N hang)
 * 	07: -2 (Bx/Nx protected P, Rx protected B/N)
 * 	08: -1 (P hang)
 * 9-41: Regular moves, indexed by piecetable[dest] - piecetable[src] + 25
 * 42: Minor promotions
 * 43: O-O-O
 * 44: O-O
 * 45-49: Neutral captures
 * 	45: PxP
 * 	46: NxN
 * 	47: BxB
 * 	48: RxR
 * 	49: QxQ
 * 50-58: Moves that win material
 * 	50: +1 (P unhang, *x hung P)
 * 	51: +2 (Px protected B/N, Bx/Nx protected R)
 * 	52: +3 (B/N unhang, *x hung B/N)
 * 	53: +4 (Px protected R)
 * 	54: +5 (R unhang, *x hung B/N)
 * 	55: +6 (Bx/Nx protected Q)
 * 	56: +7 (None)
 * 	57: +8 (Px protected Q)
 * 	58: +9 (Q unhang, *x hung Q)
 * 59: Promotions to queen
 * 60: Promotions to queen with capture
 * 61: K unhang
 ****************************************************************************/

/* starting index for moves that lose material - subtract how much is lost */
#define MOVELIST_INDEX_MAT_LOSS    9
/* regular move - add the piece/square table difference */
#define MOVELIST_INDEX_REGULAR    25
/* minor promotion - the thought behind this is if the Q promotion is wrong,
 * probably this will be too, so try other moves first */
#define MOVELIST_INDEX_PROM_MINOR 42
/* castle - add 1 if kingside */
#define MOVELIST_INDEX_CASTLE     43
/* neutral capture - add the piece number (P=0 ... Q=4) */
#define MOVELIST_INDEX_NEUTRAL    45
/* moves that win material - add how much is gained */
#define MOVELIST_INDEX_MAT_GAIN   49
/* promotion to queen - add one if it's also a capture */
#define MOVELIST_INDEX_PROM_QUEEN 59

/**
 * The movelist_t data structure is a generalization of maintaining multiple
 * linkedlists and appending them at the end for O(n) pseudo-sorting - it
 * allows an arbitrary number of lists for sorting into that many buckets,
 * with the only constraints being 1) space and 2) time to move the "max"
 * index to point to the next bucket when one bucket is empty. The result is
 * O(n) sorting during creation (beating O(n^2 logn)), but an additional O(m)
 * coefficient for both space and time complexity, so choose a small m!
 */
typedef struct {
	uint32_t array[MOVELIST_NUM_BUCKETS][MOVELIST_SUBLIST_LENGTH];
	int sublist_count[MOVELIST_NUM_BUCKETS];
	/* highest index of a bucket which contains elements; note, if
	 * array[max] is empty, that means the whole movelist is empty */
	unsigned long max;
} movelist_t;

/* Movelist's max should be zero, each array should be zero, and each
* array should have a count of zero. */
static inline void movelist_init(movelist_t *ml)
{
	memset(ml->sublist_count, 0, MOVELIST_NUM_BUCKETS*sizeof(int));
	ml->max = 0;
}

#define movelist_destroy(ml) do { } while (0)

void movelist_add(movelist_t *, uint64_t[2], uint32_t);
void movelist_addtohead(movelist_t *, uint32_t, unsigned long);
int movelist_isempty(movelist_t *);
uint32_t movelist_remove_max(movelist_t *);

#endif
