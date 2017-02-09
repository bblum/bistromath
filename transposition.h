/****************************************************************************
 * transposition.h - avoid re-searching already searched positions
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
#ifndef TRANSPOSITION_H
#define TRANSPOSITION_H

#include <stdint.h>
#include "board.h"

/**
 * Transposition table - keyed by zobrist hash, stores info about a node. We
 * keep track of the following data:
 * 1) The best move at this position (possibly 0, if a fail-low occurred)
 * 2) The evaluated (or approximate) value of the position
 * 3) The depth this node was searched to, or -1 if this was {check,stale}mate
 *    Note we use -1 (as an unsigned depth value) so that we will always use
 *    transposition information for terminal nodes rather than wasting time
 *    figuring out how big the movelist is.
 * 4) How far into the game (not search tree) this position is. This data lets
 *    us always evict nodes beyond which the state of the game has gone. Note,
 * 5) What type of values these are:
 *         Exact value
 *         Beta cutoff - failed high (storedval is lower bound)
 *         Alpha cutoff - failed low (storedval is upper bound)
 *
 * Note: The "depth" should not be simply the depth to which this node was
 * searched. It should be instead that depth plus how far into the game we are
 * (i.e., the game clock). This will help us replace nodes that are behind our
 * search tree entirely - otherwise, the table will fill with high-depth nodes
 * that we'll never use because they were back in the beginning of the game.
 * When comparing for replacement, use the gamedepth of the root node (the
 * position currently on the board), but when adding, use the gamedepth of
 * the current node (the position a few moves in the future). Because of
 * this the searcher needs to pass both values to trans_add().
 * Definitions: "search depth" refers to how deep this node will be searched;
 * "game depth" refer's to THIS NODE's depth in the whole game tree; "root
 * depth" refers to how deep the position the searcher started at is.
 */
typedef struct trans_data_t {
	/* gcc -O3 optimizes all this into a register;
	 * note, the element ordering is critical */
	uint8_t  flags;
	uint8_t  gamedepth;
	int16_t value;
	uint32_t move;
} trans_data_t;
/* The 'flags' field stores the alpha/beta/exact flags in two bits and the
 * searched-depth of the node
 * BBBBBB AA
 *      2  0
 * A: Alpha/beta/exact
 * B: searchdepth
 * If we search deeper than 63 plies then we're in trouble.
 * */
#define TRANS_FLAG_EXACT 0x2
#define TRANS_FLAG_BETA  0x1
#define TRANS_FLAG_ALPHA 0x0
/* Conversion macros - see trans_data() in trans.c for the other direction */
#define TRANS_INDEX_FLAG        0
#define TRANS_INDEX_SEARCHDEPTH 2
#define TRANS_FLAG(t)        (((t).flags >> TRANS_INDEX_FLAG) & 0x3)
#define TRANS_SEARCHDEPTH(t) (((t).flags >> TRANS_INDEX_SEARCHDEPTH) & 0x3f)
#define TRANS_GAMEDEPTH(t)   ((t).gamedepth)
#define TRANS_VALUE(t)       ((t).value)
#define TRANS_MOVE(t)        ((t).move & ((1 << MOV_INDEX_UNUSED) - 1))
#define TRANS_REPS(t)        ((t).move >> MOV_INDEX_UNUSED)

/* Add an entry. On collision, will succeed only if it's better information */
void trans_add(zobrist_t, move_t, uint8_t, int16_t, uint8_t, uint8_t, uint8_t,
               unsigned char);
/* Find a value from the table. Returns -1 if none exists - you can trust this
 * value because a trans_data_t will never be -1 due to the blank bits */
trans_data_t trans_get(zobrist_t);
/* Check if the return value from get() was valid */
int trans_data_valid(trans_data_t);

#endif
