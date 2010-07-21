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

#include <ODYNARRB.h>

class SnowGround
{
public:
	int	snow_map_id;	// 0 for non-existing
	int	minor_hp;		// 0 to 99, promote if >=100
	short	abs_x;
	short	abs_y;
	short abs_x1;
	short abs_y1;
	short abs_x2;
	short abs_y2;

public:
	void	init(short absX, short absY, int snowMapId=0);
	void	grow(int rate);
	int	dying(int rate);			// 1 when die
	int	is_valid()				{ return snow_map_id; }
};

/*
#define SNOW_ZONE_SIZE 20
#if (SNOW_ZONE_SIZE < ZOOM_WIDTH / ZOOM_LOC_WIDTH || SNOW_ZONE_SIZE < ZOOM_HEIGHT / ZOOM_LOC_HEIGHT)
#error
#endif

#define WIDTH_IN_ZONE ((MAX_MAP_WIDTH+SNOW_ZONE_SIZE-1)/SNOW_ZONE_SIZE)
#define HEIGHT_IN_ZONE ((MAX_MAP_HEIGHT+SNOW_ZONE_SIZE-1)/SNOW_ZONE_SIZE)
*/

#pragma pack(1)
class SnowGroundArray
{
private:
	unsigned	seed;

public:
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
	unsigned rand_seed();
};
#pragma pack()

extern SnowGroundArray snow_ground_array;

#endif
