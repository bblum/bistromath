/****************************************************************************
 * pawnstructure.h - evaluation helpers to judge pawn structure quality
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
#ifndef PAWNSTRUCTURE_H
#define PAWNSTRUCTURE_H

#include <stdint.h>
#include "board.h"

/**
 * Evaluate the pawn structure given the position bitboard for the pawns. We
 * don't want to consider anything besides the position of the pawns relative
 * to each other - this just assigns rewards and penalties for pawn-related
 * issues such as doubled pawns, isolated pawns, etcetera.
 * Pass in the pawn position bitboard and the color to whom these pwans belong
 */
int16_t eval_pawnstructure(board_t *, unsigned char, bitboard_t *);

#endif
