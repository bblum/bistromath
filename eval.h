/****************************************************************************
 * eval.h - evaluation function to tell how good a position is
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
#ifndef EVAL_H
#define EVAL_H

#include <stdint.h>
#include "board.h"

#ifndef EVAL_LAZY_THRESHHOLD
#define EVAL_LAZY_THRESHHOLD 250
#endif

/**
 * Is this board an endgame position
 */
int eval_isendgame(board_t *);

/**
 * Evaluation function. Given a board, returns an int value representing how
 * good this position is for whoever has the move. A positive score means the
 * player to move has the advantage; a negative score means the player to move
 * has the disadvantage; a zero score means both sides have equal chances.
 */
int16_t eval(board_t *);

/**
 * Lazy eval - just the material difference
 */
int16_t eval_lazy(board_t *);

#endif
