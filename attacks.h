/****************************************************************************
 * attacks.h - precomputed attack-/move-mask tables
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
#ifndef ATTACKS_H
#define ATTACKS_H

#include <stdint.h>
#define U64(x) ((uint64_t)(x))
#define U32(x) ((uint32_t)(x))

extern uint64_t knightattacks[64];
extern uint64_t kingattacks[64];
extern uint64_t pawnattacks[2][64];
extern uint64_t pawnmoves[2][64];

extern uint64_t rowattacks[256][8];
extern uint64_t colattacks[256][8];

extern int rot45diagindex[64];
extern int rot315diagindex[64];

extern int rot45index_shiftamountright[15];
extern int rot315index_shiftamountright[15];
extern int rot315index_shiftamountleft[15];

extern int rotresult_shiftamountleft[15];
extern int rotresult_shiftamountright[15];

uint64_t rot45attacks[256][8];
uint64_t rot315attacks[256][8];

#endif
