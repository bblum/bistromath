/****************************************************************************
 * movelist.c - data structure for move sorting in move generation/iteration
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
#include "assert.h"
#include "movelist.h"
#include "board.h"

extern int16_t eval_squarevalue[2][6][64];

/* Material values, add/subtract to find indices for moves which change the
 * material count */
static uint8_t movelist_material[] = { 1, 3, 3, 5, 9, 12 };

/* arg 1 is the attacked boards, arg 2 is the move */
static unsigned long movelist_findindex(uint64_t[2], uint32_t);

/**
 * Add a move. Determines where to add it using findindex
 */
void movelist_add(movelist_t *ml, uint64_t attackedby[2], uint32_t m)
{
	movelist_addtohead(ml, m, movelist_findindex(attackedby, m));
}

/**
 * Determine at which index in the movelist a move should be added
 */
static unsigned long movelist_findindex(uint64_t attackedby[2], uint32_t m)
{
	int16_t materialgain;
	/* which side has the move? this is a substitute for board->tomove */
	unsigned char tomove = MOV_COLOR(m);

	if (MOV_PROM(m))
	{
		if (MOV_PROMPC(m) == QUEEN)
		{
			/* MOV_CAPT is 1 or 0 */
			return MOVELIST_INDEX_PROM_QUEEN + MOV_CAPT(m);
		}
		return MOVELIST_INDEX_PROM_MINOR;
	}
	if (MOV_CASTLE(m))
	{
		/* The second term will be 1 if O-O, 0 if O-O-O */
		return MOVELIST_INDEX_CASTLE + (COL(MOV_DEST(m)) == COL_G);
	}
	if (MOV_CAPT(m))
	{
		materialgain = movelist_material[MOV_CAPTPC(m)];
		/* see if the capturing piece will be recaptured - assume it
		 * will be if the opponent attacks that square at all */
		if (attackedby[OTHERCOLOR(tomove)] & BB_SQUARE(MOV_DEST(m)))
		{
			/* this will index past the end of the array if the
			 * moving piece is a king - we rely on the board
			 * library to filter out suicide king captures */
			materialgain -= movelist_material[MOV_PIECE(m)];
			/* neutral capture */
			if (materialgain == 0)
			{
				return MOVELIST_INDEX_NEUTRAL + MOV_PIECE(m);
			}
			//real SEE could be done here
		}
		/* losing material */
		else if (materialgain < 0)
		{
			return MOVELIST_INDEX_MAT_LOSS + materialgain;
		}
		/* winning capture */
		else
		{
			return MOVELIST_INDEX_MAT_GAIN + materialgain;
		}
	}
	/* here it's a regular move */
	/* moving the piece where it can be captured */
	if (attackedby[OTHERCOLOR(tomove)] & BB_SQUARE(MOV_DEST(m)))
	{
		/* Moves that put the king in check are only generated if the
		 * attackedby bitmasks don't see it, so we won't fall apart on
		 * one either - they just get poor ordering, which is fine. */
		assert(MOV_PIECE(m) != KING);
		/* if we can recapture after being captured */
		if (attackedby[tomove] & BB_SQUARE(MOV_DEST(m)))
		{
			/* if the moving piece is better than a pawn, call it
			 * a loss. otherwise, assume the pawn won't be taken
			 * (and fall through to the next phase). */
			if (MOV_PIECE(m) != PAWN)
			{
				/* give an "approximate" SEE - guessing, when
				 * we recapture we'll get between 1 and 3 */
				return MOVELIST_INDEX_MAT_LOSS + 2 -
				       movelist_material[MOV_PIECE(m)];
			}
			//real SEE could be done here
		}
		/* if we move to a hanging square, we consider it complete
		 * loss of that piece */
		else
		{
			return MOVELIST_INDEX_MAT_LOSS -
			       movelist_material[MOV_PIECE(m)];
		}
	}
	/* here our piece is safe on its destination - let's see if we were
	 * saving it from being captured otherwise ("unhang"ing the piece); if
	 * so, it goes in the material gain category */
	if ((attackedby[OTHERCOLOR(tomove)] & ~(attackedby[tomove])) &
	    BB_SQUARE(MOV_SRC(m)))
	{
		return MOVELIST_INDEX_MAT_GAIN +
		       movelist_material[MOV_PIECE(m)];
	}
	/* regular move */
	return MOVELIST_INDEX_REGULAR +
	       eval_squarevalue[tomove][MOV_PIECE(m)][MOV_DEST(m)] -
	       eval_squarevalue[tomove][MOV_PIECE(m)][MOV_SRC(m)];
}

void movelist_addtohead(movelist_t *ml, uint32_t m, unsigned long index)
{
	/* sanity check */
	assert(index < MOVELIST_NUM_BUCKETS);
	/* add to list */
	assert(ml->sublist_count[index] < MOVELIST_SUBLIST_LENGTH);
	ml->array[index][ml->sublist_count[index]] = m;
	ml->sublist_count[index]++;

	/* adjust max pointer */
	if (index > ml->max)
	{
		ml->max = index;
	}
	return;
}

int movelist_isempty(movelist_t *ml)
{
	return (ml->sublist_count[ml->max] == 0);
}

/* Don't call this if movelist_isempty! */
uint32_t movelist_remove_max(movelist_t *ml)
{
	uint32_t returnval = ml->array[ml->max][--ml->sublist_count[ml->max]];
	/* check if we need to adjust the max pointer */
	while (movelist_isempty(ml))
	{
		/* we hit the bottom of the array; doesn't matter if empty */
		if (ml->max == 0)
		{
			break;
		}
		ml->max--;
	}
	return returnval;
}
