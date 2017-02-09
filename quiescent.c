/****************************************************************************
 * quiescent.c - captures-only search to avoid evaluating unstable positions
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
#include <stdint.h>
#include "quiescent.h"
#include "eval.h"
#include "movelist.h"

extern volatile unsigned char timeup;
extern int lazy, nonlazy;

#ifndef QUIESCENT_MAX_DEPTH
/* Warning: NEVER put this at 4 or below - it causes the bot to be too weak */
#define QUIESCENT_MAX_DEPTH 8
#endif

static int16_t qalphabeta(board_t *, int16_t, int16_t, uint8_t);

/**
 * Play out capture chains in a position until it's quiet (or until a certain
 * depth is reached, for speed) so we can accurately use the evaluator
 */
int16_t quiesce(board_t *board, int16_t alpha, int16_t beta)
{
	return qalphabeta(board, alpha, beta, QUIESCENT_MAX_DEPTH);
}

/**
 * Alpha beta searching over captures. Don't need to return a move, only the
 * value. No repetition/50-move checking (obvious reasons), no transposition
 * table (simplicity) - we get pretty good move ordering for captures already
 * from the board library.
 */
static int16_t qalphabeta(board_t *board, int16_t alpha, int16_t beta,
                         uint8_t depth)
{
	movelist_t moves;
	move_t curmove;
	/* who has the move */
	int color;
	//XXX: due to we're only generating captures, we can't check if we get
	//XXX: mated here... this seems bad, but should turn out ok (i.e., a
	//XXX: quiescence that doesn't see mates is better than no quiescence
	//XXX: at all.
	
	/* used in place of lastval */
	int16_t a;

	/* what happens if the player to move declines to make any captures */
	int16_t stand_pat;

	if (timeup)
	{
		return 0;
	}
	
	/* we'll be using stand_pat in a lot of places*/
	/* first do a lazy evaluation; if it's too far from the window we will
	 * not need the precision of eval() */
	stand_pat = eval_lazy(board);
	/* check if we're not allowed to be lazy */
	if ((stand_pat > (alpha - EVAL_LAZY_THRESHHOLD)) &&
	    (stand_pat < (beta + EVAL_LAZY_THRESHHOLD)))
	{
		stand_pat = eval(board);
		nonlazy++;
	}
	else { lazy++; }

	/********************************************************************
	 * terminal condition - search depth ran out
	 ********************************************************************/
	if (depth == 0)
	{
		return stand_pat;
	}
	
	/********************************************************************
	 * Main iteration over all the moves
	 ********************************************************************/
	/* set some preliminary values */
	if (stand_pat > alpha)
	{
		if (stand_pat >= beta)
		{
			return stand_pat;
		}
		alpha = stand_pat;
	}
	color = board->tomove;
	
	/* and we're ready to go */
	board_generatecaptures(board, &moves);
	while (!movelist_isempty(&moves))
	{
		if (timeup)
		{
			break;
		}
		curmove = movelist_remove_max(&moves);
		
		board_applymove(board, curmove);
		/* see if this move puts us in check */
		if (board_colorincheck(board, color))
		{
			board_undomove(board, curmove);
			continue;
		}
		a = -qalphabeta(board, -beta, -alpha, depth-1);
		board_undomove(board, curmove);

		if (a > alpha)
		{
			alpha = a;
		}
		if (beta <= alpha)
		{
			break;
		}
	}
	movelist_destroy(&moves);
	
	return alpha; /* will work even if no captures were available */
}
