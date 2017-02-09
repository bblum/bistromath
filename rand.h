/****************************************************************************
 * rand.h - wrappers for GSL random number generation utilities
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
/**
 * Uses the Mersenne Twister algorithm provided by GSL, the GNU Scientific
 * Library. Compile with "-lgsl -lgslcblas". We seed the Twister using the
 * current UNIX timestamp, so the values will vary for each run of the
 * program. This allows for nondeterministic play if desired.
 */
#ifndef _RAND_H
#define _RAND_H

#include <stdint.h>

/* Set up the random number generator */
void rand_init();
/* A "random" 32-bit integer as provided by GSL's mersenne twister */
uint32_t rand32();
/* A "random" 64-bit integer as provided by GSL's mersenne twister */
uint64_t rand64();
/* Free all memory associated with the rand generator */
void rand_teardown();

#endif
