/*
 * Seven Kingdoms: Ancient Adversaries
 *
 * Copyright 1997,1998 Enlight Software Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// Filename   : SnowGround
// Description: Header file of Snow Ground
// Owner      : Gilbert

#ifndef __SNOWG_H
#define __SNOWG_H

#include <stdint.h>

class File;


class SnowGroundArray
{
public:
	unsigned int seed;
	int32_t snow_thick;
	int32_t snow_pattern;

public:
	void	init(long initSnowScale, long anyNumber);
	void	inc_snow(int snowScale);			// pass weather.snow_scale()
	void	dec_snow(int decRate);				// pass weather.temp_c()
	void	process();
	int	has_snow(short x, short y);		// return 0 or snow map id

	int 	write_file(File* filePtr);
	int	read_file(File* filePtr);

private:
	unsigned int rand_seed();
};

extern SnowGroundArray snow_ground_array;

#endif
