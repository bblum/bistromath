/****************************************************************************
 * rand.c - wrappers for GSL random number generation utilities
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
#include <time.h>
#include <gsl/gsl_rng.h>
#include "rand.h"

static gsl_rng *randgen;
static unsigned char rand_initted = 0;

void rand_init()
{
	if (rand_initted)
	{
		return;
	}
	gsl_rng_env_setup();
	randgen = gsl_rng_alloc(gsl_rng_mt19937);
	gsl_rng_set(randgen, (unsigned long)time(NULL));
	rand_initted = 1;
}

uint32_t rand32()
{
	rand_init();
	return (uint32_t)gsl_rng_get(randgen);
}

uint64_t rand64()
{
	rand_init();
	return ((uint64_t)rand32()) | (((uint64_t)rand32()) << 32);
}

void rand_teardown()
{
	gsl_rng_free(randgen);
}
