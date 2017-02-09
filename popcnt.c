/****************************************************************************
 * popcnt.c - various population-count implementations
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
#include "popcnt.h"
#include "bitscan.h"

/**
 * Kernighan's LSB-resetting popcount implementation
 */
int popcnt(uint64_t x)
{
	int count = 0;
	while (x)
	{
		count++;
		x &= x - 1; // reset LS1B
	}
	return count;
}

/**
 * Good on procs with slow multiplication
 */
int popcnt2(uint64_t x)
{
	x -= (x >> 1) & 0x5555555555555555;
	x = (x & 0x3333333333333333) + ((x >> 2) & 0x3333333333333333);
	x = (x + (x >> 4)) & 0x0f0f0f0f0f0f0f0f;
	x += x >>  8;
	x += x >> 16;
	x += x >> 32;
	return x & 0x7f;
}

/**
 * Good on procs with fast multiplication
 */
int popcnt3(uint64_t x) {
	x -= (x >> 1) & 0x5555555555555555;
	x = (x & 0x3333333333333333) + ((x >> 2) & 0x3333333333333333);
	x = (x + (x >> 4)) & 0x0f0f0f0f0f0f0f0f;
	return (x * 0x0101010101010101)>>56;
}
