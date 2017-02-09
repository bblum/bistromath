/****************************************************************************
 * engine.c - wrapper functions for managing computer vs human play
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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "engine.h"

#define ENGINE_REPEATED_BUCKETS 127

/* from xboard.c, for printing */
extern FILE *ttyout;
void output(char *);

/**
 * Initialize a game engine given the searching function, with a new board
 */
engine_t *engine_init(search_fn search)
{
	engine_t *e = malloc(sizeof(engine_t));
	e->board = board_init();
	e->search = search;
	e->time_remaining = 0;
	e->time_increment = 0;
	e->line[0] = '\0';
	e->inbook = 1;
	return e;
}

/**
 * Determine how much time to think based on how much time is left
 */
static unsigned int engine_alloctime(engine_t *e)
{
	/* allocate the time */
	unsigned int secs;
	if (e->board->moves > 20)
		secs = e->time_increment + (e->time_remaining / 30) - 1;
	else
		secs = e->time_increment + (e->time_remaining / 60) - 1;
	/* we shouldn't limit the search to less than one second, also in some
	 * cases we get -1 (i.e. lightning) */
	return (((signed int)secs) >= 1) ? secs : 1;
}

/**
 * Have the engine generate a move for the side to play. String must be freed.
 */
char *engine_generatemove(engine_t *e)
{
	if (e->inbook)
	{
		move_t move = book_move(e->line, e->board);
		if (move)
		{
			/* book lookup was successful */
			output("ENGINE: Book lookup successful");
			return move_tostring(move);
		}
		else
		{
			/* we've left the book lines */
			output("ENGINE: Leaving opening book lines");
			e->inbook = 0;
		}
	}
	/* we get here if already out of book or if we just left book */
	return move_tostring(e->search(e->board, engine_alloctime(e), NULL, NULL));
}

/**
 * Change the board to reflect a move being made. Returns 1 if the move is
 * illegal, 0 if it's good
 */
int engine_applymove(engine_t *e, char *str)
{
	/* keep track of the game's line, but only if not too long */
	if (strlen(e->line) < BOOK_LINE_MAX_LENGTH-6)
	{
		strcat(e->line, str);
		strcat(e->line, " ");
	}

	/* we need to make the move before adding, else we won't detect draws
	 * before they actually are called */
	move_t move = move_islegal(e->board, str);
	if (move == 0)
	{
		return 0;
	}
	board_applymove(e->board, move);
	
	return 1;
}

char *engine_result_checkmated[2] = { "0-1 {White checkmated}",
                                      "1-0 {Black checkmated}" };
char *engine_result_stalemated[2] = { "1/2-1/2 {White stalemated}",
                                      "1/2-1/2 {Black stalemated}" };
char *engine_result_repetition = "1/2-1/2 {Draw by threefold repetition}";
char *engine_result_fiftymoves = "1/2-1/2 {Draw by the fifty move rule}";

/**
 * Check if the game is over. Returns nonzero if so, 0 if not. If so, gives
 * back a result string in the second argument. If it's the sort of draw that
 * needs to be claimed, returns 2.
 */
int engine_checkgameover(engine_t *e, char **result)
{
	unsigned char color = e->board->tomove;
	int matetype = board_mated(e->board);
	/* check if a player is mated */
	if (matetype == BOARD_CHECKMATED)
	{
		*result = engine_result_checkmated[color];
		return 1;
	}
	else if (matetype == BOARD_STALEMATED)
	{
		*result = engine_result_stalemated[color];
		return 1;
	}
	/* threefold repetition */
	if (board_threefold_draw(e->board))
	{
		*result = engine_result_repetition;
		return 2;
	}
	/* fifty moves */
	if (e->board->halfmoves >= 100)
	{
		*result = engine_result_fiftymoves;
		return 2;
	}
	return 0;
}

void engine_destroy(engine_t *e)
{
	if (e == NULL)
	{
		return;
	}
	board_destroy(e->board);
	free(e);
	return;
}
