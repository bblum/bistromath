/****************************************************************************
 * search.h - traverse the game tree looking for the best position
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
#ifndef SEARCH_H
#define SEARCH_H

#include <stdint.h>
#include "board.h"
#include "util/hashmap_u64_int.h"

/**
 * A search function will require the following arguments:
 * 1) Board state
 * 2) How many seconds we should think before moving
 * 3) int * - if nonnull, lets you know how many nodes were searched
 * 4) int * - if nonnull, lets you know the alpha value of the position
 */
typedef move_t (*search_fn)(board_t *, unsigned int, int *, int16_t *);

move_t getbestmove(board_t *, unsigned int, int *, int16_t *);

#endif
