/****************************************************************************
 * popcnt.h - various population-count implementations
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
#ifndef POPCNT_H
#define POPCNT_H

#include <stdint.h>

/* Can be used to change which implementation we use */
//#define POPCOUNT(x) popcnt3(x)
#define POPCOUNT(x) __builtin_popcountll(x)

/**
 * Returns a count of how many bits are set in the argument.
 */
int popcnt(uint64_t);

/**
 * Alternative implementations
 */
int popcnt2(uint64_t);
int popcnt3(uint64_t);

#endif
