/****************************************************************************
 * pawnstructure.c - evaluation helpers to judge pawn structure quality
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
#include <stdlib.h>
#include "board.h"
#include "attacks.h"
#include "pawnstructure.h"
#include "popcnt.h"
#include "bitscan.h"

/* We do pawn square scoring here rather than in eval.c to save time */
extern int16_t eval_squarevalue[2][6][64];

typedef struct ps_entry_t {
	bitboard_t key;
	bitboard_t holes;
	int16_t value;
} ps_entry_t;

/* this is a lot like the transposition table in case you haven't noticed... */
#ifndef PS_NUM_BUCKETS
#define PS_NUM_BUCKETS 4194301
#endif
static ps_entry_t array[PS_NUM_BUCKETS];

static void ps_add(bitboard_t key, bitboard_t holes, int16_t value)
{
	unsigned long bucket;
	
	/* find the bucket */
	bucket = key % PS_NUM_BUCKETS;
	/* always evict */
	array[bucket].key = key;
	array[bucket].holes = holes;
	array[bucket].value = value;
	
	return;
}

/**
 * Fetch data from the table.
 */
#define PAWNSTRUCTURE_LOOKUP_FAIL INT16_MIN
static int16_t ps_get(bitboard_t key, bitboard_t *holes)
{
	unsigned long bucket = key % PS_NUM_BUCKETS;
	if (array[bucket].key == key)
	{
		*holes = array[bucket].holes;
		return array[bucket].value;
	}
	return PAWNSTRUCTURE_LOOKUP_FAIL;
}

/* Bonus for pawn chains - for each pawn protected by another, add bonus */
#ifndef PS_CHAIN_BONUS
#define PS_CHAIN_BONUS      2
#endif
/* Penalty for doubling pawns - for each pawn with another pawn on its file,
 * subtract the penalty (note, tripled pawns net loss 6 times this!) */
#ifndef PS_DOUBLED_PENALTY
#define PS_DOUBLED_PENALTY  8
#endif
/* Isolated penalty - for each pawn with no brothers on adjacent files
 * note, if a pawn gets this penalty it will also get the backwards penalty */
#ifndef PS_ISOLATED_PENALTY
#define PS_ISOLATED_PENALTY 16
#endif
/* Backward pawns - nothing on same rank or behind on adjacent columns */
#ifndef PS_BACKWARD_PENALTY
#define PS_BACKWARD_PENALTY 8
#endif

/* Bonuses for having a passed pawn of <color> on the given <rank> */
int16_t pawnstructure_passed_bonus[2][8] = {
	{ 0,  3,  6, 12, 24, 48, 96, 0 },
	{ 0, 96, 48, 24, 12,  6,  3, 0 }
};

/**
 * Pawnstructure evaluator - color being whose pawns these are, and holes a
 * pass-by-reference second return value - a chart of all "holes" in the pawn
 * structure; that is, all squares which can never be attacked by these pawns.
 */
int16_t eval_pawnstructure(board_t *board, unsigned char color, bitboard_t *h)
{
	/* the value we calculate and store in the hashtable */
	int16_t value;
	/* value dependent on factors outside of just the pawn pos, so we
	 * cannot hash this and must calculate even if the cache hits */
	int16_t unhashable_bonus = 0;
	
	square_t square;
	bitboard_t pawns, pos, friends, everybodyelse, behindmask, holes;

	pawns = board->pos[color][PAWN];
	
	/* attempt to lookup - if we succeed, only need to calculate the
	 * unhashable bonus. pass in the h ptr as well; if the lookup
	 * fails it'll be untouched */
	if ((value = ps_get(pawns, h)) != PAWNSTRUCTURE_LOOKUP_FAIL)
	{
		while (pawns)
		{
			square = BITSCAN(pawns);
			pawns ^= BB_SQUARE(square);
			if (board_pawnpassed(board, square, color))
			{
				unhashable_bonus += pawnstructure_passed_bonus[color][ROW(square)];
			}
		}
		return value + unhashable_bonus;
	}
	
	/* holes starts with everything set, and we take off bits that our
	 * pawns could attack (using passedpawn masks for this) */
	holes = ~BB_SQUARE(0x0);
	/* for each pawn, consider its value with relation to the structure */
	pos = pawns;
	while (pos)
	{
		/* find and clear a bit */
		square = BITSCAN(pos);
		pos ^= BB_SQUARE(square);
		/* piece/square table */
		value += eval_squarevalue[color][PAWN][square];

		/* check for passed pawn */
		if (board_pawnpassed(board, square, color))
		{
			unhashable_bonus += pawnstructure_passed_bonus[color][ROW(square)];
		}
		
		everybodyelse = pawns ^ BB_SQUARE(square);
		
		/* first evaluate chaining */
		friends = everybodyelse & pawnattacks[color][square];
		value += PS_CHAIN_BONUS * POPCOUNT(friends);

		/* next, doubled pawns */
		friends = everybodyelse & BB_FILE(COL(square));
		value -= PS_DOUBLED_PENALTY * POPCOUNT(friends);
		
		/* backwards */
		friends = everybodyelse & bb_adjacentcols[COL(square)];
		/* get a mask of all squares behind/same rank as this square */
		if (color == WHITE)
		{
			behindmask = (BB_SQUARE(square) ^ (BB_SQUARE(square) - 1)) |
			             BB_RANK(ROW(square));
		}
		else
		{
			behindmask = (BB_SQUARE(square) ^ (-BB_SQUARE(square))) |
			             BB_RANK(ROW(square));
		}
		if (!(friends & behindmask))
		{
			value -= PS_BACKWARD_PENALTY;
			/* see if isolated entirely */
			if (!friends) /* SO RONERY */
			{
				value -= PS_ISOLATED_PENALTY;
			}
		}
		/* calculate holes in pawnstructure - any squares that this
		 * pawn may be able to attack, take out of the bitmap */
		holes ^= (bb_passedpawnmask[color][square] & ~BB_FILE(COL(square)));
	}
	ps_add(pawns, holes, value);
	*h = holes;
	return value + unhashable_bonus;
}
